// A/D conversion 
#ifdef THREAD
static semaphore mutex = available(1);
#endif

// Read the specified a2d.
uint16_t get_a2d(struct a2d * a)
{
#ifdef THREAD
    suspend(&mutex);
#endif    
    
    ADMUX = (a->low_volt ? 0x80 : 0) | (a->low_res ? 0: 0x40) | (a->source & 15);
    
#if MHZ==16    
    ADCSRA = a->low_res ? 0xC6 : 0xC7;  // divide by 64 or 128
#else  
    ADCSRA = a->low_res ? 0xC5 : 0xC6;  // divide by 32 or 64
#endif   

#ifdef THREAD    
    while (ADCSRA & 0x40) yield();
#else
    while (ADCSRA & 0x40);
#endif    
    uint16_t r = a->low_res ? ADCH : ADC;
    ADCSRA = 0;
#ifdef THREAD    
    release(&mutex);
#endif    
    return r;
}

