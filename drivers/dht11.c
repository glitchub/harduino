// DHT11 temp/humidity sensor driver

// Requires one pin which must have 5K to 10K pull-up resistor.
#ifndef DHT11_IO
#error Must define pin DHT11_IO
#endif

// Initialize
void init_dht11(void)
{
    DDR(DHT11_IO) &= NOBIT(DHT11_IO);                                           // Make sure our pin in an input
    PORT(DHT11_IO) &= NOBIT(DHT11_IO);                                          // and will drive low when an output
}

// Read DHT11 and set dc = degrees C and rh = relative humidity percent and
// return 0, or non-0 on error.
int8_t get_dht11(uint8_t *dc, uint8_t *rh)
{
    uint8_t l1, l2, d[5];

    DDR(DHT11_IO) |= BIT(DHT11_IO);                                             // First drive the I/O low
    sleep_ticks(18);                                                            // For 18 mS
    DDR(DHT11_IO) &= NOBIT(DHT11_IO);                                           // Then back to input
    __asm__ __volatile__("nop; nop;");                                          // allow some time for pin to pull high

    // Note these are 6 cycles/loop, so max pulse at 16Mhz = 6*255/16 = 95 uS
    l1=0; while (PIN(DHT11_IO) & BIT(DHT11_IO)) if (!++l1) return 1;            // DHT may output high for a bit
    l1=0; while (!(PIN(DHT11_IO) & BIT(DHT11_IO))) if (!++l1) return 2;         // then will go low for 80uS
    l1=0; while (PIN(DHT11_IO) & BIT(DHT11_IO)) if (!++l1) return 3;            // then high for 80uS

    for (uint8_t b=0; b<40; b++)                                                // now for 5 bytes
    {
        l1=0; while (!(PIN(DHT11_IO) & BIT(DHT11_IO))) if (!++l1) return 4;     // DHT will go low for 50uS
        l2=0; while (PIN(DHT11_IO) & BIT(DHT11_IO)) if (!++l2) return 5;        // then high for 28uS (zero) or 70uS (one).
        d[b/8] = (d[b/8] << 1) | (l2 > l1);                                     // Compare bit time against the 50uS high
    }
    if (((d[0]+d[1]+d[2]+d[3]) & 255) != d[4]) return 6;                        // if checksum is valid
    *dc=d[2];                                                                   // then set degrees C
    *rh=d[0];                                                                   // and relative humidity
    return 0;                                                                   // and return success
}
