// SPI driver, supports master mode only

#if !defined(SPI_ORDER) || SPI_ORDER < 0 || SPI_ORDER > 1
#error Must define SPI_ORDER 0 or 1
#endif
#if !defined(SPI_MODE) || SPI_MODE < 0 || SPI_MODE > 3
#error Must define SPI_MODE 0 to 3
#endif
#if !defined(SPI_CLOCK) || SPI_CLOCK < 0 || SPI_CLOCK > 3
#error Must define SPI_CLOCK 0 to 3
#endif

#ifdef THREAD
static semaphore complete, mutex=available(1);
#endif

static volatile uint8_t *txd, *rxd, txc, rxc, rxs;
ISR(SPI_STC_vect)
{
    if (rxs)                            // receive skip?
    {
        rxs--;                          // count down
    }
    else if (rxc)                       // else bytes to receive?
    {
        *rxd++=SPDR;                    // do so
        rxc--;
    }

    if (txc)                            // bytes to send?
    {
        SPDR=*++txd;                    // do so, note pre-increment
        txc--;
    }
    else if (rxc)                       // still bytes to receive?
    {
        SPDR=txd?*txd:0;                // Just repeat the last byte that was sent
    }
    else                                // transfer complete
    {
        SPCR &= 0x7f;                   // clear SPIE
#ifdef THREAD        
        release(&complete);             // unblock waiting thread
#endif        
    }
}

// Perform a single SPI transfer, sending txcount bytes from *txdata while
// simultaneously receiving rxcount bytes to *rxdata. Note txdata and rxdata
// can safely point to the same memory buffer.
//
// rxskip defines how many received bytes to ignore before receive data is
// valid, often this is set to 1.
//
// If txcount is less than rxskip+rxcount, the last byte of txdata will be
// repeated until all bytes are received. If txdata is NULL then zeros are
// sent.
//
// Returns true when done, or false if arguments are invalid.
bool xfer_spi(uint8_t *txdata, uint8_t txcount, uint8_t rxskip, uint8_t *rxdata, uint8_t rxcount)
{
    if ((!txcount && !rxcount) || (txcount && !txdata) ||
        (rxcount && !rxdata) || (rxskip && !rxcount)) return 0;
#ifdef THREAD        
    suspend(&mutex);
#endif    
    txd=txdata; txc=txcount; rxs=rxskip; rxd=rxdata; rxc=rxcount;
    CLR_GPIO(SPI_SS);                   // take SS low
    SPSR; SPDR;                         // reset latent interrupt
    if (txc)                            // maybe send first byte
    {
        SPDR=*txd;                      // note ISR pre-increments
        txc--;
    } else SPDR=0;                      // else send zero
    SPCR |= 0x80;                       // let ISR do the rest
    sei();                              // make sure interrupts are enabled
#ifdef THREAD    
    suspend(&complete);                 // wait for transfer complete
#else
    while (SPCR & 0x80);
#endif    
    SET_GPIO(SPI_SS);                   // take SS high
#ifdef THREAD    
    release(&mutex);
#endif    
    return 1;
}

// Initialize SPI
void init_spi(void)
{
#ifdef THREAD
    suspend(&mutex);
#endif    
    SET_GPIO(SPI_SS);                   // SS high
    OUT_GPIO(SPI_SS);                   // SS is output
    IN_GPIO(SPI_MISO);                  // MISO is input
    OUT_GPIO(SPI_MOSI);                 // MOSI is output
    OUT_GPIO(SPI_SCK);                  // SCK is output
    SPCR = 0x40|(SPI_ORDER<<5)|0x10|(SPI_MODE<<2)|SPI_CLOCK;
#ifdef THREAD    
    release(&mutex);
#endif    
}
