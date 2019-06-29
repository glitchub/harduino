// AVR threads

semaphore __runnable;

// Release a thread suspended on the semaphore
void release(semaphore *s)
{
    char sreg=SREG;
    cli();
    if (s->list)
    {
        void **l = s->list;
        s->list=*l;
        *l=NULL;
        void **r=(void **)&__runnable.list;
        while (*r) r=*r;
        *r=l;
    } else
        s->count++; // nobody is suspended, just count
    SREG=sreg;
}

// Suspend the current thread and start next runnable thread. This is the core
// of the thread dispatcher.
//
// We need to exactly control the stack frame and the "naked" attribute doesn't
// work right when there's a function argument, so we can't use C at all here.
// But it we did it would start with:
void suspend(semaphore *sem);
asm (
    ".global suspend            \n" // tell linker
    "suspend:                   \n"
    "    in r18, __SREG__       \n" // save global interrupt flag
    "    cli                    \n" // disable interrupts
    "    movw r30, r24          \n" // sem is passed in r24:r25, move to Z
    "    tst r30                \n" // null pointer?
    "    breq kamikaze          \n" // yes, thread commits suicide

    // If sem->count is non-zero, just decrement and return.
    "    ldd r0, Z+2            \n" // get sem->count (one byte at offset 2)
    "    tst r0                 \n" // zero?
    "    breq suspense          \n" // yes, go suspend
    "    dec r0                 \n" // no, decrement it
    "    std Z+2, r0            \n" //
    "    out __SREG__, r18      \n" // restore SREG
    "    ret                    \n" // and return

    // Semaphore count is zero, we will suspend. First push all registers of
    // interest, per https://www.nongnu.org/avr-libc/user-manual/FAQ.html#faq_reg_usage
    "suspense:                  \n"
    "    push r2                \n"
    "    push r3                \n"
    "    push r4                \n"
    "    push r5                \n"
    "    push r6                \n"
    "    push r7                \n"
    "    push r8                \n"
    "    push r9                \n"
    "    push r10               \n"
    "    push r11               \n"
    "    push r12               \n"
    "    push r13               \n"
    "    push r14               \n"
    "    push r15               \n"
    "    push r16               \n"
    "    push r17               \n"
    "    push r28               \n"
    "    push r29               \n"
    // The Top Of Stack forms a pointer to another TOS, push NULL
    "    push r1                \n"
    "    push r1                \n"

    // Z = sem->list, this heads the linked list of TOS's, follow the list to last
    // suspended TOS (note there might not be any!)
    "seekend:                   \n"
    "    ld r28, Z              \n" // Y=*Z
    "    ldd r29, Z+1           \n"
    "    adiw r28,0             \n" // is it zero?
    "    breq atend             \n"
    "    movw r30, r28          \n" // no, Z=Y
    "    rjmp seekend           \n"

    // Z = last TOS in the list (or &sem->list), add our TOS
    "atend:                     \n"
    "    in r28, __SP_L__       \n" // Y=SP
    "    in r29, __SP_H__       \n"
    "    adiw r28, 1            \n" // but undo push's post-decrement
    "    st Z, r28              \n" // *Z=Y
    "    std Z+1, r29           \n"

    // The thread is now suspended, check if __runnable.list is pointing to a
    // thread.
    "kamikaze:\n"
    "    ldi r30,lo8(__runnable)\n" // Z=__runnable.list
    "    ldi r31,hi8(__runnable)\n"
    "getnext:                   \n"
    "    ld r28, Z              \n" // Y=*Z
    "    ldd r29, Z+1           \n"
    "    adiw r28,0             \n" // zero?
    "    brne gotnext           \n" // no, go unsuspend

    // Ohh, nothing to do, we must wait for an ISR to release(something). So we
    // must turn interrupts on. If sleep mode was previously enabled, we'll
    // also sleep, else just spin madly. Note we're still using the suspended
    // thread's stack.
    "    sei                    \n"
    "    sleep                  \n" // sleep or nop
    "    cli                    \n"
    "    rjmp getnext           \n" // go check again

    // Here, Y=runnable thread's TOS. Delink it from runnable.list.
    "gotnext:                   \n"
    "    ld r26, Y              \n" // X=*Y,
    "    ldd r27, Y+1           \n"
    "    st Z, r26              \n" // *Z=X
    "    std Z+1, r27           \n"

    // Activate the new stack frame
    "    out __SP_H__, r29      \n" // SP=Y
    "    out __SP_L__, r28      \n"
    "    pop r0                 \n" // discard the LSB of the TOS link

    // Restore thread registers
    "    pop r29                \n"
    "    pop r28                \n"
    "    pop r17                \n"
    "    pop r16                \n"
    "    pop r15                \n"
    "    pop r14                \n"
    "    pop r13                \n"
    "    pop r12                \n"
    "    pop r11                \n"
    "    pop r10                \n"
    "    pop r9                 \n"
    "    pop r8                 \n"
    "    pop r7                 \n"
    "    pop r6                 \n"
    "    pop r5                 \n"
    "    pop r4                 \n"
    "    pop r3                 \n"
    "    pop r2                 \n"

    // Th-th-th-th-that's all folks!
    "    out __SREG__, r18      \n" // restore SREG
    "    ret                    \n"
);

// Given a thread entry point and address/size of an allocated stack, init the
// thread's stack frame and suspend on the runnable list.
static void init_thread (void (*thread)(void), uint8_t *stack, int size)
{
    memset(stack, 0xA5, size);

    // We replicate the stack frame created by suspend
    stack=stack+size-1;                 // start at the last byte of the allocated stack
    *stack-- = (uint16_t)thread & 255;  // push return address low
    *stack-- = (uint16_t)thread >> 8;   // push return address high
    *stack-- = 2;                       // push r2
    *stack-- = 3;                       // push r3
    *stack-- = 4;                       // push r4
    *stack-- = 5;                       // push r5
    *stack-- = 6;                       // push r6
    *stack-- = 7;                       // push r7
    *stack-- = 8;                       // push r8
    *stack-- = 9;                       // push r9
    *stack-- = 10;                      // push r10
    *stack-- = 11;                      // push r11
    *stack-- = 12;                      // push r12
    *stack-- = 13;                      // push r13
    *stack-- = 14;                      // push r14
    *stack-- = 15;                      // push r15
    *stack-- = 16;                      // push r16
    *stack-- = 17;                      // push r17
    *stack-- = 28;                      // push r28
    *stack-- = 29;                      // push r29
    *stack-- = 0;                       // push TOS pointer to NULL
    *stack-- = 0;
    void **r=(void **)&__runnable.list; // find end of runnable list
    while (*r) r=*r;
    *r=(void *)stack+1;                 // link our TOS there
}

// These point to the start and end of the .threads section, exported by the
// linker, see the .lds file generated by the build.
extern _thread *__threads_start, *__threads_end;

// This is the bottom of main's stack
extern uint8_t __heap_start;

// Start all threads in .threads section
void start_threads(void)
{
    cli();
    for (_thread **p =&__threads_start; p < &__threads_end; p++)
        init_thread((*p)->func, (*p)->stack, (*p)->size);
#ifdef DEBUG_STACKS
    // Also tag the main stack
    for (uint8_t *s = &__heap_start; (uint16_t)s < SP; s++) *s = 0xa5;
#endif
    set_sleep_mode(SLEEP_MODE_IDLE);    // Enable sleep when all threads are suspended
    sleep_enable();
    sei();                              // Enable interrupts
    yield();                            // Let all threads run once
}

#ifdef DEBUG_STACKS
// Given serial handle, report unused stack for each thread.
static void show(char *name, uint8_t *base, uint16_t size)
{
    uint16_t x=0;
    while (*base++ == 0xA5) x++;
    printf("%-11s: %d unused of %d\n", name, x, size);
}

void debug_stacks(void)
{
    for (_thread **p =&__threads_start; p < &__threads_end; p++) show((*p)->name, (*p)->stack, (*p)->size);
    show("main", &__heap_start, 0x900-(uint16_t)&__heap_start);
}
#endif
