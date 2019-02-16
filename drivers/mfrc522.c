// Support for MFRC522 RFID contactless card reader

// Interesting registers
#define CommandReg      0x01
#define ComIrqReg       0x04
#define ErrorReg        0x06
#define Status1Reg      0x07
#define FIFODataReg     0x09
#define FIFOLevelReg    0x0a
#define ControlReg      0x0c
#define BitFramingReg   0x0d
#define CollReg         0x0e
#define ModeReg         0x11
#define TxModeReg       0x12
#define RxModeReg       0x13
#define TxControlReg    0x14
#define TxASKReg        0x15
#define CRCResultHiReg  0x21
#define CRCResultLoReg  0x22
#define RFCfgReg        0x26
#define VersionReg      0x37

// Interesting commands (written to Command register)
#define IdleCmd         0x00
#define CalcCRCCmd      0x03
#define TranscieveCmd   0x0c

// Typing is boring
#define u8 uint8_t
#define s8 int8_t

// Read byte from register
static u8 rb(u8 reg)
{
    reg = 0x80 | (reg<<1);;
    xfer_spi(&reg, 1, 1, &reg, 1);
    return reg;
}

// Write byte to register
static void wb(u8 reg, u8 data)
{
    xfer_spi((u8[]){reg<<1, data}, 2, 0, NULL, 0);
}

// Read up to rmax bytes from fifo. Return number of bytes that were actually
// available, whether they were all read or not. Note xfer_spi repeats the last
// transmitted byte in the case where txdata is shorter than rxdata.
static u8 rfifo(u8 *data, u8 rmax)
{
    u8 avail = rb(FIFOLevelReg);
    if (data && avail && rmax)
        xfer_spi((u8[]){0x80|(FIFODataReg<<1)}, 1, 1, data, (avail < rmax) ? avail : rmax);
    return avail;
}

// Write count bytes to fifo.
static void wfifo(u8 *data, u8 count)
{
    wb(FIFOLevelReg, 0x80); // first reset it
    u8 buf[count+1];
    buf[0] = FIFODataReg << 1;
    memcpy(buf+1, data, count);
    xfer_spi(buf, count+1, 0, NULL, 0);
}

// Generate CRC of bytes at *data, write the result to *target and
// return true, or return 0 if error.
static bool crc(u8 *data, u8 bytes, u8 *target)
{
    wb(CommandReg, IdleCmd);
    wfifo(data, bytes);
    wb(CommandReg, CalcCRCCmd);
    int32_t timeout=get_ticks()+10;                             // give it 10 mS
    while (1)
    {
        yield();
        if (rb(Status1Reg) & 0x20) break;                       // until CRC complete
        if (expired(timeout)) return 0;                         // this really should not happen
    }
    *target++=rb(CRCResultLoReg);                               // write little-endian CRC to pointer
    *target=rb(CRCResultHiReg);
    return 1;                                                   // success!
}

// Send specified number of txbits from txdata and wait for response. Return 0
// if timeout, -1 if receive error, else copy up to rmax bytes from FIFO to
// *data, and return total number of bits received (will be at least 4).
// rxalign specifies the bit alignment within the first byte of rxdata, used
// for UID fragment assembly.
static s8 transceive(u8 *txdata, u8 txbits, u8 *rxdata, u8 rxmax, u8 rxalign)
{
    wb(CommandReg, IdleCmd);
    wfifo(txdata, (txbits+7)/8);                                // round up to whole bytes
    wb(ComIrqReg, 0x7F);                                        // reset interrupt status
    wb(CommandReg, TranscieveCmd);
    wb(BitFramingReg, 0x80 | ((rxalign&7)<<4) | (txbits&7));    // set StartSend and bit alignment
    int32_t timeout=get_ticks()+10;                             // give it 10 mS
    while(1)
    {
        yield();
        if (rb(ComIrqReg) & 0x20) break;                        // loop until RxIRq
        if (expired(timeout)) return 0;                         // or timeout
    }
    if (rb(ErrorReg) & 0x13) return -1;                         // BufferOvfl, ParityErr, or ProtocolError
    u8 rxbytes=rfifo(rxdata, rxmax);                            // get bytes from fifo
    if (!rxbytes) return -1;                                    // shouldn't happen
    uint16_t rxbits=((rxbytes-1)*8)+((rb(ControlReg)&7)?:8);    // calculate actual bits
    if (rxbits < 4 || rxbits > 127) return -1;                  // shouldn't happen
    return rxbits;
}

// Given two arrays, merge 'total' bits from src to the tail of dst starting at
// bit position 'first'. Note position 0 is the LSB of dst[0], position 8 is
// the LSB of dst[1], etc. Example, if dst={AA,03}, src={B8,CC,DD,FF}, first=11
// and total=22, then the result is dst={AA,BB,CC,DD,FF} (the last byte is
// copied entirely even though only one bit was significant).
static void merge(u8 *dst, u8 *src, s8 first, s8 total)
{
    for (;first>=8; first-=8) dst++;
    if (first)
    {
        u8 mask=(0xff << first); // high bit mask
        *src &= mask;
        *dst &= (u8)~mask;
        *dst++ |= *src++;
        total -= (8-first);
    }
    for (;total>0; total-=8) *dst++=*src++;
}

// Send REQA, return true if any card responded.
static inline bool REQA(void) { return transceive((u8[]){0x26}, 7, NULL, 0, 0) == 16; }

// Send HLTA, to halt the currently selected card so it no longer responds to REQA.
static inline void HLTA(void) { transceive((u8[]){0x50, 0x00, 0x57, 0xcd}, 32, NULL, 0, 0); };

// Given a pointer to 10-byte UID buffer, try to get UID from card, copy it to
// the buffer and return 4, 7, or 10 to indicate number of UID bytes. Return 0 if
// card not present, or some negative number on error...
#define err(n) -((pass*10)+n)   // ...specifically, this one
s8 get_mfrc522(u8 *uid)
{
    if (!REQA()) return 0;                                      // send REQA to wake up cards in IDLE state

    u8 pass=0;
    while(1)                                                    // extract the card's uid, this could take up to 3 passes
    {
        u8 sel[9], nvb=0;
        while (1)                                               // while we don't have 32 bits yet
        {
            sel[0] = 0x93+(pass*2);                             // send SEL with pass
            sel[1] = 0x20|nvb;                                  // and the current number of valid bits
            u8 rsp[5];                                          // The card responds with the rest, plus a BCC byte
            s8 got = transceive(sel, nvb+16, rsp, 5, nvb&7);    // Expect 40-nvb bits
            if (got <= 0) return err(1);                        // oops, card is gone
            if (got != 40-nvb) continue;                        // If not, try again
            u8 valid=rb(CollReg);                               // How many are valid?
            if (valid & 0x20) valid=got-8;                      // Maybe all of them!
            else valid = ((valid & 0x1F)?:32)-1;                // else failed bit index becomes count
            if (valid > got-8) continue;                        // shouldn't happen, try again
            merge(sel+2, rsp, nvb, valid);
            nvb += valid;
            if (nvb == 32)                                      // until we've done them all
            {
                sel[6]=rsp[(got-1)/8];                          // install the bcc
                break;                                          // and we're out
            }
        }

        // xor of sel[2] through sel[6] should be zero
        u8 bcc=0;
        for(u8 i=2; i<=6; i++) bcc ^= sel[i];
        if (bcc) continue;                                      // nope, try again

        // now send a real select
        sel[0] = 0x93+(pass*2);                                 // SEL 0x93, 0x95, or 0x97
        sel[1] = 0x70;
        if (!crc(sel,7,sel+7)) return err(2);                   // with a CRC in sel[7] and sel[8]
        while(1)
        {
            s8 got = transceive(sel, 72, NULL, 0, 0);           // send 72 bits
            if (got <= 0) return err(3);                        // oops, card is gone
            if (got == 24) break;                               // expect 3 byte SAK
        }

        // Accumulate the UID fragments, possibly across three passes
        switch(pass++)
        {
            case 0:
                if (sel[2] != 0x88)                             // cascade tag?
                {
                    memcpy(uid, sel+2, 4);                      // no, UID is 4 bytes
                    HLTA();                                     // halt the selected card and return
                    return 4;
                }
                memcpy(uid, sel+3, 3);                          // else keep three and continue
                break;

            case 1:
                if (sel[2] != 0x88)                             // cascade tag?
                {
                    memcpy(uid+3, sel+2, 4);                    // no, append the 4 bytes
                    HLTA();                                     // halt the selected card
                    return 7;                                   // 7 bytes total
                }
                memcpy(uid+3, sel+3, 3);                        // else append 3 and continue
                break;

            case 2:                                             // no further cascade is possible
                memcpy(uid+6, sel+2, 4);                        // append the 4 bytes
                HLTA();                                         // halt the card
                return 10;                                      // 10 bytes total
        }
    }
}

// Init the mfrc522 and return true, or false if error
bool init_mfrc522(void)
{
    PORT(MFRC522_RST) &= NOBIT(MFRC522_RST);                    // take reset low
    DDR(MFRC522_RST) |= BIT(MFRC522_RST);
    init_spi();                                                 // init the SPI
    PORT(MFRC522_RST) |= BIT(MFRC522_RST);                      // take reset high
    sleep_ticks(50);                                            // give it 50mS to come up
    u8 ver = rb(VersionReg);                                    // is it alive?
    if (ver != 0x91 && ver != 0x92) return 0;                   //
    wb(ModeReg, 0x3d);                                          // Use CRCPreset 0x6363 (ISO4443 CRC_A)
    wb(TxASKReg, 0x40);                                         // Force100ASK
    wb(TxControlReg,0x83);                                      // InvTx2RFOn, Tx1RFEn, Tx2RFEn
    wb(RFCfgReg, 0x70);                                         // 48dB receiver gain
    wb(TxModeReg, 0);                                           // 106 kbps transmit
    wb(RxModeReg, 8);                                           // 106 kbps receive, RxNoErr means RxIRq is set only if receive data avaiable
    return 1;
}
