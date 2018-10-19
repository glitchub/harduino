// Serial port driver

// SERIAL_TX_SIZE and SERIAL_RX_SIZE define the size of ram buffer allocated
// for the corresponding function, 0 (or undefined) disables the function entirely.
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
    while (txn == SERIAL_TX_SIZE);                  // spin while queue is full
    uint8_t sreg=SREG;
    cli();
    txq[(txo + txn)%SERIAL_TX_SIZE] = (unsigned)c;  // add character to end of queue
    txn++;                                          // note another
    UCSR0B |= (1<<UDRIE0);                          // enable interrupt if not already
    SREG=sreg;
}

// Return number of chars that can be written before blocking
int8_t writeable_serial(void)
{
    return SERIAL_TX_SIZE-txn;
}

#ifdef SERIAL_STDIO
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
    while (!rxn);                                   // spin whle queue is empty
    uint8_t sreg=SREG;
    cli();
    uint8_t c = rxq[rxo];                           // get the oldest character
    rxo=(rxo+1)%SERIAL_RX_SIZE;                     // advance to next
    rxn--;
    SREG=sreg;;
    return c;
}

// Return number of characters that can be read without blocking
int8_t readable_serial(void)
{
    return rxn;
}

#ifdef SERIAL_STDIO
static int get(FILE *f) { (void) f; if (!UCSR0B) return EOF; return (int)((unsigned)read_serial()); }
#endif
#endif

#ifdef SERIAL_STDIO
// Static file handle for fprintf, etc. Note do NOT fclose this handle.
static FILE handle;
#endif

// Disable serial port. Attempting to use serial functions will result in
// system hang.
void disable_serial(void)
{
    UCSR0B = 0;
    UCSR0A = 0;
#if SERIAL_TX_SIZE
    txo=txn=0;
#endif
#if SERIAL_RX_SIZE
    rxo=rxn=0;
#endif
#ifdef SERIAL_STDIO
    // invalidate the file handle
    memset(&handle, 0, sizeof handle);
#endif
}

// (Re)enable the serial port at specified baud rate
#ifdef SERIAL_STDIO
FILE *enable_serial(uint32_t baud)
#else
void enable_serial(uint32_t baud)
#endif
{
    disable_serial();
    UBRR0 = (F_CPU/(8*baud))-1;             // use the X2 divisor
    UCSR0C = 0x06;                          // N-8-1
    UCSR0A = 2;                             // set UX20
#if SERIAL_TX_SIZE
#ifdef SERIAL_STDIO
    handle.put = put;
    handle.flags = _FDEV_SETUP_WRITE;
#endif
    UCSR0B |= 0x08;                         // transmit pin enable
#endif
#if SERIAL_RX_SIZE
#ifdef SERIAL_STDIO
    handle.get = get;
    handle.flags |= _FDEV_SETUP_READ;
#endif
    UCSR0B |= 0x90;                         // receive interrupt enable, receive pin enable
#endif
#ifdef SERIAL_STDIO
    return &handle;
#endif
}
