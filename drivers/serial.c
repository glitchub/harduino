// Serial port driver

// SERIAL_TX_SIZE and SERIAL_RX_SIZE define the size of ram buffer allocated
// for the corresponding function, 0 (or undefined) disables the function entirely.
#if SERIAL_TX_SIZE>255 || SERIAL_RX_SIZE>255 || SERIAL_TX_SIZE<0 || SERIAL_RX_SIZE<0 || (!SERIAL_TX_SIZE && !SERIAL_RX_SIZE)
#error "SERIAL_TX_SIZE/SERIAL_RX_SIZE config is invalid"
#endif

#ifdef THREADED
// a transmit semaphore, counts the space in the transmit buffer
static semaphore txsem=available(SERIAL_TX_SIZE);
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
#ifdef THREADED
        release(&txsem);                            // release suspended writer
#endif
    }
    else
        UCSR0B &= (uint8_t)~(1<<UDRIE0);            // else disable interrupt
}

// Block until space in the transmit queue, then send it
void write_serial(int8_t c)
{
#ifdef THREADED
    suspend(&txsem);                                // suspend while queue is full
#else
    while (txn == SERIAL_TX_SIZE);                  // spin while queue is full
#endif
    uint8_t sreg=SREG;
    cli();
    txq[(txo + txn)%SERIAL_TX_SIZE] = (uint8_t)c;   // add character to end of queue
    txn++;                                          // note another
    UCSR0B |= (1<<UDRIE0);                          // enable interrupt if not already
    SREG=sreg;
}

// Return true if chars can be written without blocking
bool writeable_serial(void)
{
#ifdef THREADED
    return is_released(&txsem);
#else
    return SERIAL_TX_SIZE-txn;
#endif
}

#ifdef SERIAL_STDIO
static int put(char c, FILE *f) { (void) f; if (!UCSR0B) return EOF; write_serial(c); return 0; }
#endif
#endif

#if SERIAL_RX_SIZE
static volatile uint8_t rxq[SERIAL_RX_SIZE];        // receive queue
static volatile uint8_t rxo=0, rxn=0;               // index of oldest char and total chars in queue

#ifdef THREADED
static semaphore rxsem;
#endif

ISR(USART_RX_vect)
{
    char c = UDR0;                                  // this clears the interrupt
    if (rxn == SERIAL_RX_SIZE) return;              // oops, receive queue overflow
    rxq[(rxo + rxn) % SERIAL_RX_SIZE] = c;          // insert into queue
    rxn++;                                          // note another
#ifdef THREADED
    release(&rxsem);                                // release suspended reader
#endif
}

// Block until character is receive queue then return it
int8_t read_serial(void)
{
#ifdef THREADED
    suspend(&rxsem);
#else
    while (!rxn);                                   // spin whle queue is empty
#endif
    uint8_t sreg=SREG;
    cli();
    uint8_t c = rxq[rxo];                           // get the oldest character
    rxo=(rxo+1)%SERIAL_RX_SIZE;                     // advance to next
    rxn--;
    SREG=sreg;;
    return c;
}

// Return true if characters can be read without blocking
bool readable_serial(void)
{
#ifdef THREADED
    return is_released(&rxsem);
#else
    return rxn;
#endif
}

#ifdef SERIAL_STDIO
static int get(FILE *f) { (void) f; return (int)((unsigned)read_serial()); }
#endif
#endif

#ifdef SERIAL_STDIO
// Static file handle for fprintf, etc. Note do NOT fclose this handle.
static FILE handle;
#endif

// Init the serial port at specified baud rate
#ifdef SERIAL_STDIO
FILE *init_serial(uint32_t baud)
#else
void init_serial(uint32_t baud)
#endif
{
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
