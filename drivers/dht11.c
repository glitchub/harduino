// DHT11 temp/humidity sensor driver

// Read DHT11 attached to specified gpio, set dc = degrees C and rh =
// relative humidity percent and return 0, or non-0 on error.
// May disable interrupts up to 3.6mS, TICKMS should be at least 2.
int8_t get_dht11(uint8_t *dc, uint8_t *rh, gpio *pin)
{
    uint8_t l1, l2, d[5], state=0;

    out_gpio(pin);
    clr_gpio(pin);                                                              // Go low
    sleep_ticks(18);                                                            // For 18 mS
    in_gpio(pin);                                                               // Then back to input
    set_gpio(pin);                                                              // Pull up

    uint8_t sreg = SREG;

    state++; l1=0; while (get_gpio(pin)) if (!++l1) goto err;                   // DHT may output high for a bit
    state++; l1=0; while (!(get_gpio(pin))) if (!++l1) goto err;                // then will go low for 80uS
    state++; l1=0; while (get_gpio(pin)) if (!++l1) goto err;                   // then high for 80uS

    cli();
    for (uint8_t b=0; b<40; b++)                                                // now for 5 bytes
    {
        state++; l1=0; while (!get_gpio(pin)) if (!++l1) goto err;              // DHT will go low for 50uS
        state++; l2=0; while (get_gpio(pin)) if (!++l2) goto err;               // then high for 28uS (zero) or 70uS (one).
        d[b/8] = (d[b/8] << 1) | (l2 > l1);                                     // Compare bit time against the 50uS high
    }
    SREG = sreg;
    state++;
    if (((d[0]+d[1]+d[2]+d[3]) & 255) != d[4]) goto err;                        // if checksum is valid
    *dc=d[2];                                                                   // then set degrees C
    *rh=d[0];                                                                   // and relative humidity
    return 0;                                                                   // and return success

    err:
    SREG=sreg;
    return state;
}
