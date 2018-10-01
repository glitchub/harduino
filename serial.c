// Serial I/O for Arduino uno

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

// Set these as desired to control how much ram will be allocated for transmit
// and receive (+2 bytes). Maximum value is 256. Set to 0 to disable the
// corresponding tx or rx write function completely. 
#define TXB 8                               // transmit buffer size
#define RXB 8                               // receive buffer size

#if TXB>256 || RXB>256 || TXB<0 || RXB<0 || (!TXB && !RXB)
#error "TXB/RXB config is invalid"
#endif

#if TXB
static volatile unsigned char txq[TXB];     // transmit queue 
static volatile unsigned char txo=0, txn=0; // index of oldest char and total chars in queue

ISR(USART_UDRE_vect)                        // holding register empty
{
    if (txn)                                // something to send?
    {
        UDR0=txq[txo];                      // do so
        txo = (txo+1)%TXB;                  // advance to next
        txn--;
    }
    else
        UCSR0B &= ~(1<<UDRIE0);             // else disable interrupt
}

// Queue character for transmit or block
static int tx(char c, FILE *f)
{
    (void) f;                               // f unused
    while (txn == TXB);                     // spin while queue full
    cli();
    txq[(txo + txn) % TXB] = (unsigned)c;   // add character to end of queue
    txn++;                                  // note another
    UCSR0B |= (1<<UDRIE0);                  // enable interrupt if not already
    sei();
    return 0;
}
#endif

#if RXB
static volatile unsigned char rxq[RXB];     // receive queue    
static volatile unsigned char rxo=0, rxn=0; // index of oldest char and total chars in queue

ISR(USART_RX_vect)
{
    char c = UDR0;                          // this clears the interrupt
    if (rxn == RXB) return;                 // oops, receive queue overflow
    rxq[(rxo + rxn) % RXB] = c;             // insert into queue
    rxn++;                                  // note another 
}

int rx(FILE *f)
{
    if (f) while (!rxn);                    // if called with file handle, spin whle queue s empty
    else if (!rxn) return -1;               // if called directly, return -1 if queue is empty
    cli();
    unsigned char c = rxq[rxo];             // get the oldest character
    rxo=(rxo+1)%RXB;                        // advance to next
    rxn--;
    sei();
    return (int)c;
}
#endif

// return number of characters that can be written before blocking 
unsigned char writable_serial(void)
{
#ifdef TXB
    return TXB-txn;
#else
    return 0;
#endif
}

// return number of character that can be read before blocking
unsigned char readable_serial(void)
{
#ifdef RXB
    return rxn;
#else
    return 0;
#endif    
}

static FILE handle = FDEV_SETUP_STREAM(
#if RXB && TXB
    tx, rx, _FDEV_SETUP_RW
#elif TXB
    tx, NULL, _FDEV_SETUP_WRITE
#else
    NULL, rx, _FDEV_SETUP_READ
#endif
);

// (Re)initialize serial port and return static file handle
FILE *init_serial(unsigned long baud)
{
    UCSR0B = 0;                             // disable serial interrupts
    baud = (F_CPU/(8*baud))-1;              // use the X2 divisor
    UBRR0H = baud / 256;                    // set baud clock
    UBRR0L = baud & 255;
    UCSR0C = 0x06;                          // N-8-1
    UCSR0A = 2;                             // flush pending, set UX20
#if TXB && RXB
    txo = txn = rxo = rxn = 0;              // reset queues
    UCSR0B = 0x98;                          // receive interrupt enable, receive pin enable, transmit pin enable
#elif TXB
    txo = txn = 0;                          // reset queue
    UCSR0B = 0x08;                          // transmit pin enable
#else
    rxo = rxn = 0;                          // reset queue
    UCSR0B = 0x90;                          // receive interrupt enable, receive pin enable
#endif
    sei();
    return &handle;
}
