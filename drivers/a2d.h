// A/D conversion

// An analog input definition
struct a2d
{
    uint8_t source;     // input source 0-15
    bool low_res;       // if true, result is 8-bit, else result is 10-bit
    bool low_volt;      // if true, input max is 1.1V, else max is Vcc.
};

// read analog input
uint16_t get_a2d(struct a2d *a);
