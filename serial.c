// Serial I/O for Arduino uno

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "serial.h"

#if SERIAL_TX_SIZE>256 || SERIAL_RX_SIZE>256 || SERIAL_TX_SIZE<0 || SERIAL_RX_SIZE<0 || (!SERIAL_TX_SIZE && !SERIAL_RX_SIZE)
#error "SERIAL_TX_SIZE/SERIAL_RX_SIZE config is invalid"
#endif

#if SERIAL_TX_SIZE
static volatile unsigned char txq[SERIAL_TX_SIZE];  // transmit queue 
static volatile unsigned char txo=0, txn=0;         // index of oldest char and total chars in queue

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
void write_serial(char c)
{
    while (txn == SERIAL_TX_SIZE);                  // spin while queue is full
    cli();
    txq[(txo + txn)%SERIAL_TX_SIZE] = (unsigned)c;  // add character to end of queue
    txn++;                                          // note another
    UCSR0B |= (1<<UDRIE0);                          // enable interrupt if not already
    sei();
}

// Return number of chars that can be written before blocking
int writeable_serial(void)
{
    return SERIAL_TX_SIZE-txn;
}

#if SERIAL_STDIO
static int put(char c, FILE *f) { (void) f; write_serial(c); return 0; }  
#endif
#endif

#if SERIAL_RX_SIZE
static volatile unsigned char rxq[SERIAL_RX_SIZE];  // receive queue    
static volatile unsigned char rxo=0, rxn=0;         // index of oldest char and total chars in queue

ISR(USART_RX_vect)
{
    char c = UDR0;                                  // this clears the interrupt
    if (rxn == SERIAL_RX_SIZE) return;              // oops, receive queue overflow
    rxq[(rxo + rxn) % SERIAL_RX_SIZE] = c;          // insert into queue
    rxn++;                                          // note another 
}

// Block until character is receive queue then return it
char read_serial(void)
{
    while (!rxn);                                   // spin whle queue is empty
    cli();
    unsigned char c = rxq[rxo];                     // get the oldest character
    rxo=(rxo+1)%SERIAL_RX_SIZE;                     // advance to next
    rxn--;
    sei();
    return c;
}

// Return number of characters that can be read without blocking
int readable_serial(void)
{
    return rxn;
}

#if SERIAL_STDIO
static int get(FILE *f) { (void) f; return (int)((unsigned)read_serial()); }  
#endif
#endif

// (Re)initialize serial port 
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
FILE *init_serial(unsigned long baud)
#else
void init_serial(unsigned long baud)
#endif
{
    cli();                                  
    UCSR0B = 0;                             // disable serial interrupts
    UBRR0 = (F_CPU/(8*baud))-1;             // use the X2 divisor
    UCSR0C = 0x06;                          // N-8-1
    UCSR0A = 2;                             // flush pending, set UX20
#if SERIAL_TX_SIZE && SERIAL_RX_SIZE
    txo = txn = rxo = rxn = 0;              // reset queues
    UCSR0B = 0x98;                          // receive interrupt enable, receive pin enable, transmit pin enable
#elif SERIAL_TX_SIZE
    txo = txn = 0;                          // reset queue
    UCSR0B = 0x08;                          // transmit pin enable
#else
    rxo = rxn = 0;                          // reset queue
    UCSR0B = 0x90;                          // receive interrupt enable, receive pin enable
#endif
    sei();
#if SERIAL_STDIO 
    return &handle;
#endif
}
