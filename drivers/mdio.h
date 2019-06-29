// bitbang mdio

// A phy definition
typedef struct
{
    uint8_t addr;       // physical address 0-31
    gpio *MDC;          // clock gpio
    gpio *MDIO;         // data gpio
} phy;

// Note the register address is either 0-31 for simple 'clause22' mdio, or in
// form 0xddrrrr for 'clause45' mdio, where dd is the 5-bit device, and rrrr is
// the 16-bit register address.

// Write 16-bit data to phy register
void write_mdio(phy *p, int32_t reg, uint16_t data);

// Read 16-bit data from phy register
uint16_t read_mdio(phy *p, int32_t reg);
