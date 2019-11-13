// Serial port driver

// SERIAL_TX_SIZE and SERIAL_RX_SIZE define the size of ram buffer allocated
// for the corresponding function, 0 (or undefined) disables buffering entirely
#define SERIAL_TX_SIZE 60   // transmit buffer size, must be 0 to 255
#define SERIAL_RX_SIZE 4    // receive buffer size, must be 0 to 255

// If defined, enable printf()
#define SERIAL_STDIO 1      

#ifdef THREAD
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
#ifdef THREAD        
        release(&txsem);                            // release suspended writer
#endif        
    }
    else
        UCSR0B &= (uint8_t)~(1<<UDRIE0);            // else disable interrupt
}

// Block until space in the transmit queue, then send it
void write_serial(int8_t c)
{
#ifdef THREAD    
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
#ifdef THREAD
    return is_released(&txsem);
#else
    return SERIAL_TX_SIZE-txn;
#endif
}

#ifdef SERIAL_STDIO
static int put(char c, FILE *f)
{
    (void) f;
    if (!UCSR0B) return EOF;
    if (c == '\n') write_serial('\r');              // \n -> \r\n
    write_serial(c);
    return 0;
}
#endif
#endif

#if SERIAL_RX_SIZE
static volatile uint8_t rxq[SERIAL_RX_SIZE];        // receive queue
static volatile uint8_t rxo=0, rxn=0;               // index of oldest char and total chars in queue

#ifdef THREAD
static semaphore rxsem;
#endif

ISR(USART_RX_vect)
{
    bool err = UCSR0A & 0x1c;                       // framing, overrun, parity error?
    char c = UDR0;                                  // get the char, reset the interrupt
    if (err) return;                                // but ignore error
    if (rxn == SERIAL_RX_SIZE) return;              // or receive queue overflow
    rxq[(rxo + rxn) % SERIAL_RX_SIZE] = c;          // insert into queue
    rxn++;                                          // note another
#ifdef THREAD    
    release(&rxsem);                                // release suspended reader
#endif    
}

// Block until character is receive queue then return it
int8_t read_serial(void)
{
#ifdef THREAD
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
#ifdef THREAD
    return is_released(&rxsem);
#else    
    return rxn;
#endif    
}

#ifdef SERIAL_STDIO
static int get(FILE *f)
{
    (void) f;
    int c = (int)((unsigned)read_serial());
    return (c=='\r') ? '\n' : c;                    // \r -> \n
}
#endif
#endif

#ifdef SERIAL_STDIO
// Static file handle for printf, etc. Note do NOT fclose this handle.
static FILE handle;
#endif

// Init the serial port for 115200 baud, N-8-1
void init_serial(void)
{
#if MHZ == 16
    UBRR0 = 16;                             // X2 divisor
    UCSR0A = 2;                             // set UX20
#elif MHZ == 8
    UBRR0 = 8;                              // X2 divisor
    UCSR0A = 2;                             // set UX20
#else
#error MHZ not supported
#endif
#if SERIAL_TX_SIZE
#ifdef SERIAL_STDIO
    handle.put = put;
    handle.flags = _FDEV_SETUP_WRITE;
#endif
    UCSR0C = 0x06;                          // N-8-1
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
    stdin=stdout=&handle;                   // use handle for stdin and stdout
#endif
}

// return pressed key or -1 if none
int key_press(void)
{
    if (!readable_serial()) return -1;
    return getchar();
}
