// Bitbang MDIO

// Clock minimum period is 400 ns (2.5MHz max)

// MDIO requires a specific pull-up resistor of 1.5 kΩ to 10 kΩ, taking into
// account the total worst-case leakage current of 32 PHYs and one MAC.

// When the MAC drives the MDIO line, it has to guarantee a stable value 10 ns
// (setup time) before the rising edge of the clock MDC. Further, MDIO has to
// remain stable 10 ns (hold time) after the rising edge of MDC.

// When the PHY drives the MDIO line, the PHY has to provide the MDIO signal
// between 0 and 300 ns after the rising edge of the clock.[1] Hence, with a
// minimum clock period of 400 ns (2.5 MHz maximum clock rate) the MAC can
// safely sample MDIO during the second half of the low cycle of the clock.

// There are two specification clauses of interest: clause 22 provides access
// to 32 16-bit registers. Clause 45 allows an extended register to be
// addressed in one transaction and accessed in the next.

// A useful graphic can be found at
// https://en.wikipedia.org/wiki/Management_Data_Input/Output#/media/File:Miim_timing.svg.

// Clock out a bit
#define out(b) ({ if (b) set_gpio(p->MDIO); else clr_gpio(p->MDIO); set_gpio(p->MDC); clr_gpio(p->MDC); })

// Clock in a bit
#define in() ({ set_gpio(p->MDC); clr_gpio(p->MDC); get_gpio(p->MDIO); })

// mdio opcodes
#define ISC22 0x80 // set if clause22
#define ISRD  0x40 // set if read op
#define ADDR45   (0)
#define WRITE22  (1 | ISC22)
#define WRITE45  (1)
#define READ22   (2 | ISRD | ISC22)
#define READ45   (3 | ISRD)

// Send MDIO frame, possibly return read data depending on opcode
static int16_t do_mdio(phy *p, uint8_t opcode, int8_t reg, uint16_t data)
{
    // assert MDIO high, MDC low
    set_gpio(p->MDIO);
    out_gpio(p->MDIO);
    clr_gpio(p->MDC);
    out_gpio(p->MDC);

    // start with 32 ones
    for (uint8_t x=0; x<32; x++) out(1);

    // start bits
    out(0);
    out(opcode & ISC22); // 1 for clause 22, 0 for clause 45

    // opcode
    out(opcode & 2);
    out(opcode & 1);

    // 5-bit phy address
    for (uint8_t n = 0x10; n; n>>=1) out(p->addr & n);

    // 5- bit register address
    for (uint8_t n = 0x10; n; n>>=1) out(reg & n);

    if (opcode & ISRD)
    {
        // we are reading, switch to input
        in_gpio(p->MDIO); set_gpio(p->MDIO);

        // dummy clock
        in();

        // clock in 16 data bits
        for (uint8_t n=0; n < 16; n++)
        {
            data <<= 1;
            if (in()) data |= 1;
        }

        // dummy clock
        in();

        return data;
    }

    // we are writing, send turnaround
    out(1);
    out(0);

    // send 16 data bits
    for (uint16_t n = 0x8000; n; n>>=1) out(data & n);

    // leave as input
    in_gpio(p->MDIO); set_gpio(p->MDIO);

    // dummy clock
    in();

    return 0;
}

// Write 16-bit data to phy register
void inline write_mdio(phy *p, int32_t reg, uint16_t data)
{
    if (reg <= 31)
    {
        // write clause 22
        do_mdio(p, WRITE22, (uint8_t)reg, data);
    }
    else
    {
        // write clause 45
        do_mdio(p, ADDR45, (uint8_t)(reg>>16), (uint16_t)reg);
        do_mdio(p, WRITE45, (uint8_t)(reg>>16), data);
    }
}

// Read 16-bit data from phy register
uint16_t inline read_mdio(phy *p, int32_t reg)
{
    uint16_t data;
    if (reg <= 31)
    {
        // clause 22
        data=do_mdio(p, READ22, (uint8_t)reg, 0);
    } else
    {
        // clause 45
        do_mdio(p, ADDR45, (uint8_t)(reg>>16), (uint16_t)reg);
        data=do_mdio(p, READ45, (uint8_t)(reg>>16), 0);
    }
    return data;
}
