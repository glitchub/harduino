// Definitions for Arduino Uno R3

// Each gpio is defined as GPIOXX, where XX is the nominal Uno pin number 1-13
// or A1-A5.

// BITN(GPIOXX) gives the gpio's bit as a value 0-7
#define _BITN_(bit, pin, ddr, port) (bit)
#define BITN(...) _BITN_(__VA_ARGS__)

// BIT(GPIOXX) gives the gpio's bit mask as a power of 2, 0x01 to 0x80 (aka bit
// vector).
#define BIT(...) (1<<BITN(__VA_ARGS__))

// NOBIT(GPIOXX) gits the gpio's inverted bitmask, cast to avoid promotion to
// int.
#define NOBIT(...)((uint8_t)~(BIT(__VA_ARGS__)))

// PIN(GPIOXX) gives the gpio's PIN register address
#define _PIN_(bit, pin, ddr, port) (pin)
#define PIN(...) _PIN_(__VA_ARGS__)

// DDR(GPIOXX) gives the gpio's DDR register address
#define _DDR_(bit, pin, ddr, port) (ddr)
#define DDR(...) _DDR_(__VA_ARGS__)

// PORT(GPIOXX) gives the gpio's PORT register address
#define _PORT_(bit, pin, ddr, port) (port)
#define PORT(...) _PORT_(__VA_ARGS__)

#define GPIO00 PD0, PIND, DDRD, PORTD
#define GPIO01 PD1, PIND, DDRD, PORTD
#define GPIO02 PD2, PIND, DDRD, PORTD
#define GPIO03 PD3, PIND, DDRD, PORTD
#define GPIO04 PD4, PIND, DDRD, PORTD
#define GPIO05 PD5, PIND, DDRD, PORTD
#define GPIO06 PD6, PIND, DDRD, PORTD
#define GPIO07 PD7, PIND, DDRD, PORTD
#define GPIO08 PB0, PINB, DDRB, PORTB
#define GPIO09 PB1, PINB, DDRB, PORTB
#define GPIO10 PB2, PINB, DDRB, PORTB
#define GPIO11 PB3, PINB, DDRB, PORTB
#define GPIO12 PB4, PINB, DDRB, PORTB
#define GPIO13 PB5, PINB, DDRB, PORTB
#define GPIOA0 PC0, PINC, DDRC, PORTC
#define GPIOA1 PC1, PINC, DDRC, PORTC
#define GPIOA2 PC2, PINC, DDRC, PORTC
#define GPIOA3 PC3, PINC, DDRC, PORTC
#define GPIOA4 PC4, PINC, DDRC, PORTC
#define GPIOA5 PC5, PINC, DDRC, PORTC
