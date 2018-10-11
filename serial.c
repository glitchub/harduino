// Serial I/O for Arduino

#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "serial.h"

#if SERIAL_TX_SIZE>255 || SERIAL_RX_SIZE>255 || SERIAL_TX_SIZE<0 || SERIAL_RX_SIZE<0 || (!SERIAL_TX_SIZE && !SERIAL_RX_SIZE)
#error "SERIAL_TX_SIZE/SERIAL_RX_SIZE config is invalid"
#endif

#if SERIAL_TX_SIZE
static volatile uint8_t txq[SERIAL_TX_SIZE];        // transmit queue 
static volatile uint8_t txo=0, txn=0;               // index of oldest char and total chars in queue

ISR(USART_UDRE_vect)                                // holding register empty
{
    if (txn)                                        // something to send?
    {
        UDR0=txq[txo];                              // do so
        txo = (txo+1)%SERIAL_TX_SIZE;               // advance to next
        txn--;
    }
    else
        UCSR0B &= ~(1<<UDRIE0);                     // else disable interrupt
}

// Block until space in the transmit queue, then send it
void write_serial(int8_t c)
{
    if (!UCSR0B) return; 
    while (txn == SERIAL_TX_SIZE);                  // spin while queue is full
    cli();
    txq[(txo + txn)%SERIAL_TX_SIZE] = (unsigned)c;  // add character to end of queue
    txn++;                                          // note another
    UCSR0B |= (1<<UDRIE0);                          // enable interrupt if not already
    sei();
}

// Return number of chars that can be written before blocking
int8_t writeable_serial(void)
{
    if (!UCSR0B) return 0;
    return SERIAL_TX_SIZE-txn;
}

#if SERIAL_STDIO
static int put(char c, FILE *f) { (void) f; if (!UCSR0B) return EOF; write_serial(c); return 0; }  
#endif
#endif

#if SERIAL_RX_SIZE
static volatile uint8_t rxq[SERIAL_RX_SIZE];        // receive queue
static volatile uint8_t rxo=0, rxn=0;               // index of oldest char and total chars in queue

ISR(USART_RX_vect)
{
    char c = UDR0;                                  // this clears the interrupt
    if (rxn == SERIAL_RX_SIZE) return;              // oops, receive queue overflow
    rxq[(rxo + rxn) % SERIAL_RX_SIZE] = c;          // insert into queue
    rxn++;                                          // note another 
}

// Block until character is receive queue then return it
int8_t read_serial(void)
{
    if (!UCSR0B) return 0;
    while (!rxn);                                   // spin whle queue is empty
    cli();
    uint8_t c = rxq[rxo];                           // get the oldest character
    rxo=(rxo+1)%SERIAL_RX_SIZE;                     // advance to next
    rxn--;
    sei();
    return c;
}

// Return number of characters that can be read without blocking
int8_t readable_serial(void)
{
    return rxn;
}

#if SERIAL_STDIO
static int get(FILE *f) { (void) f; if (!UCSR0B) return EOF; return (int)((unsigned)read_serial()); }  
#endif
#endif

// Disable serial port
void stop_serial(void)
{
    UCSR0B = 0;
    UCSR0A = 0;
#if SERIAL_TX_SIZE
    txo=txn=0;
#endif
#if SERIAL_RX_SIZE
    rxo=rxn=0;
#endif
}

// (Re)start serial port at specified baud rate, does not enable global interrupts!
#if SERIAL_STDIO
static FILE handle = FDEV_SETUP_STREAM(
#if SERIAL_RX_SIZE && SERIAL_TX_SIZE
    put, get, _FDEV_SETUP_RW
#elif SERIAL_TX_SIZE
    put, NULL, _FDEV_SETUP_WRITE
#else
    NULL, get, _FDEV_SETUP_READ
#endif
);
FILE *start_serial(uint32_t baud)
#else
void start_serial(uint32_t baud)
#endif
{
    stop_serial();                          
    UBRR0 = (F_CPU/(8*baud))-1;             // use the X2 divisor
    UCSR0C = 0x06;                          // N-8-1
    UCSR0A = 2;                             // set UX20
#if SERIAL_TX_SIZE
    UCSR0B |= 0x08;                         // transmit pin enable
#endif    
#if SERIAL_RX_SIZE
    UCSR0B |= 0x90;                         // receive interrupt enable, receive pin enable
#endif
#if SERIAL_STDIO 
    return &handle;
#endif
}
