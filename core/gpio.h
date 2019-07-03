// gpio definitions

typedef struct
{
    uint8_t bit;
    volatile uint8_t *port;
    volatile uint8_t *ddr;
    volatile uint8_t *pin;
} gpio;

// Expand port letter and bit number to gpio list
#define GPIO(_port, _num) 1<<_num, &PORT ## _port, &DDR ## _port, &PIN ## _port

// GPIOS can be accessed by reference, e.g.:
//    gpio g = {GPIO(A,1)};
//    in_gpio(&g);

// make gpio an output
#define out_gpio(pgpio)  (*(pgpio->ddr) |= pgpio->bit)

// make gpio an input
#define in_gpio(pgpio) (*(pgpio->ddr) &= (uint8_t)~pgpio->bit)

// test if gpio is an output
#define is_out_gpio(pgpio) (*(pgpio->ddr) & pgpio->bit)

// set gpio output high, or gpio input pullup
#define set_gpio(pgpio) (*(pgpio->port) |= pgpio->bit)

// set gpio output low, or input tri-state
#define clr_gpio(pgpio) (*(pgpio->port) &= (uint8_t)~pgpio->bit)

// get gpio input state
#define get_gpio(pgpio) (*(pgpio->pin) & pgpio->bit)

// toggle gpio output/pullup
#define tog_gpio(pgpio) (*(pgpio->pin) = pgpio->bit)

// GPIOs can be accessed statically, e.,g.:
//    #define g GPIO(A,1)
//    IN_GPIO(g)
// This is much smaller and faster.

// make gpio an output
#define _OUT_GPIO_(_bit, _port, _ddr, _pin) (*(_ddr) |= (_bit))
#define OUT_GPIO(...) _OUT_GPIO_(__VA_ARGS__)

// test if gpio is an output
#define _IS_OUT_GPIO_(_bit, _port, _ddr, _pin) (*(_ddr) & (_bit))
#define IS_OUT_GPIO(...) _OUT_GPIO_(__VA_ARGS__)

// make gpio an input
#define _IN_GPIO_(_bit, _port, _ddr, _pin) (*(_ddr) &= (uint8_t)~(_bit))
#define IN_GPIO(...) _IN_GPIO_(__VA_ARGS__)

// set gpio output high, or gpio input pullup
#define _SET_GPIO_(_bit, _port, _ddr, _pin) (*(_port) |= (_bit))
#define SET_GPIO(...) _SET_GPIO_(__VA_ARGS__)

// set gpio output low, or input tri-state
#define _CLR_GPIO_(_bit, _port, _ddr, _pin) (*(_port) &= (uint8_t)~(_bit))
#define CLR_GPIO(...) _CLR_GPIO_(__VA_ARGS__)

// get gpio input state
#define _GET_GPIO_(_bit, _port, _ddr, _pin) (*(_pin) & (_bit))
#define GET_GPIO(...) _GET_GPIO_(__VA_ARGS__)

// toggle gpio output/pullup
#define _TOG_GPIO_(_bit, _port, _ddr, _pin) (*(_pin) = (_bit))
#define TOG_GPIO(...) _TOG_GPIO_(__VA_ARGS__)
