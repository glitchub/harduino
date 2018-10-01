// Definitions for Arduino Uno R3

// This is always included by the Makefile, however to use these definitions
// you must explicitly include avr/io.h.

// Each gpio is defined as GPIOXX, where XX is the nominal Uno pin number 1-13
// or A1-A5.

// BIT(GPIOXX) gives the gpio's bit mask as a power of 2, 0x01 to 0x80 
// PIN(GPIOXX) gives the gpio's PIN register address
// DDR(GPIOXX) gives the gpio's DDR register address
// PORT(GPIOXX) gives the gpio's PORT register address

#define _BIT_(bit, pin, ddr, port) (bit)
#define BIT(...) _BIT_(__VA_ARGS__)

#define _PIN_(bit, pin, ddr, port) pin
#define PIN(...) _PIN_(__VA_ARGS__)

#define _DDR_(bit, pin, ddr, port) ddr
#define DDR(...) _DDR_(__VA_ARGS__)

#define _PORT_(bit, pin, ddr, port) port
#define PORT(...) _PORT_(__VA_ARGS__)

#define GPIO00   (1<<PD0), PIND, DDRD, PORTD
#define GPIO01   (1<<PD1), PIND, DDRD, PORTD
#define GPIO02   (1<<PD2), PIND, DDRD, PORTD
#define GPIO03   (1<<PD3), PIND, DDRD, PORTD
#define GPIO04   (1<<PD4), PIND, DDRD, PORTD
#define GPIO05   (1<<PD5), PIND, DDRD, PORTD
#define GPIO06   (1<<PD6), PIND, DDRD, PORTD
#define GPIO07   (1<<PD7), PIND, DDRD, PORTD
#define GPIO08   (1<<PB0), PINB, DDRB, PORTB
#define GPIO09   (1<<PB1), PINB, DDRB, PORTB
#define GPIO10   (1<<PB2), PINB, DDRB, PORTB
#define GPIO11   (1<<PB3), PINB, DDRB, PORTB
#define GPIO12   (1<<PB4), PINB, DDRB, PORTB
#define GPIO13   (1<<PB5), PINB, DDRB, PORTB
#define GPIOA0   (1<<PC0), PINC, DDRC, PORTC
#define GPIOA1   (1<<PC1), PINC, DDRC, PORTC
#define GPIOA2   (1<<PC2), PINC, DDRC, PORTC
#define GPIOA3   (1<<PC3), PINC, DDRC, PORTC
#define GPIOA4   (1<<PC4), PINC, DDRC, PORTC
#define GPIOA5   (1<<PC5), PINC, DDRC, PORTC
