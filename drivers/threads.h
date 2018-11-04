// Cooperative threading for AVR
//
// A thread is simply a function that has its own stack. It can call other
// functions, it can create stack variables, no special coding is required
// except that each thread must occasionally invoke yield() or suspend() in
// order to relinquish the CPU.
//
// Threads are defined as follows:
//
// char my_thread_stack[256];
// void __attribute__((noreturn)) my_thread(void)
// {
//     while(1)
//     {
//         yield() or suspend();
//         do stuff;
//     }
// }
//
// The thread must not return, the noreturn attribute ensures that.
//
// main() is responsible for initializing each thread:
//
// void main(void)
// {
//     init_thread(my_thread, my_thread_stack, sizeof my_thread_stack);
//     init_thread(another_thread, another_stack, sizeof another_stack);
//     ...
//     sei();
//
//     while(1)
//     {
//         yield() or suspend();
//         do stuff;
//     }
// }
//
// Note main() also becomes a thread. There is no explicit thread dispatcher.
//
// It's possible to run the same code as two different threads, each with their
// own stack. Local static variables are shared between threads.
//
// The stack size must be large enough to accommodate the thread's deepest stack
// variable allocation (including function calls, library usage, etc), plus
// whatever the worst-case interrupt handler requires, plus 20 bytes for suspended
// thread context. 64 bytes is about the minimum possible size, 128 or 256 is
// reasonable for threads of moderate complexity.
//
// The system maintains a "runnable" list of all threads which are not
// suspended on semaphores and are waiting to execute.

// Inter-thread communication, arbitration, and synchronization is accomplished
// with semaphores.
//
// A semaphore is simply a data structure:
typedef volatile struct
{
    void *list;             // head of linked list of suspended threads
    unsigned char count;    // number of unhandled releases
} semaphore;
// Warning, altering this definition will require changes to assembly language
// in threads.c.
//
// In classical CS science terminology a thread can "signal" a semaphore, or
// "wait" for a semaphore. Since these names are used by libc for other kinds
// of process control, we use the terms "release" and "suspend" instead.

// "release(&semaphore)" performs the following:
//
//     If the semaphore has no suspended threads (semaphore->list == NULL) then
//     its count is incremented.
//
//     Otherwise, the suspended thread is delinked from the semaphore's list
//     and appended to the runnable list.
//
//     Either way, control returns to the caller.
void release(semaphore *s);

// "suspend(&sempahore)" performs the following:
//
//     If the semaphore count is non-zero then it is decremented, and control
//     returns to the calling thread.
//
//     Otherwise:
//
//     1 - Pushes CPU registers of interest to the stack, followed by a "TOS"
//     pointer, which intially contains NULL. The TOS pointer is appended to
//     the linked list of pointers headed by semaphore->link.
//
//     2 - If no thread exists in the runnable list, enables interrupts and
//     spins until some ISR invokes "release(thread)".
//
//     3 - Changes the stack pointer to the stack of the runnable thread, pops
//     the CPU registers that were pushed in step 1 and returns to the new
//     thread.
void suspend(semaphore *s);

// Utility functions

// Return true if the semaphore has at least one suspended thread
static inline char is_suspended(semaphore *s)
{
    uint8_t sreg=SREG;
    cli();
    char b = s->list != NULL;
    SREG=sreg;
    return(b);
}

// Return true if the semaphore has at least one unhandled release
// Note is_suspended() and is_released() cannot both be true at once.
static inline char is_released(semaphore *s)
{
    uint8_t sreg=SREG;
    cli();
    char b = s->count != 0;
    SREG=sreg;
    return b;
}

// Suspend the current thread on the end of the runnable list only if there is
// another thread waiting to run.
static inline void yield(void)
{
    extern semaphore __runnable;
    if (__runnable.list) suspend(&__runnable);
}

// Release all suspended threads
static inline void release_all(semaphore *s)
{
    uint8_t sreg=SREG;
    cli();
    while (s->list) release(s);
    SREG=sreg;
}

// Release one suspended thread
static inline void release_one(semaphore *s)
{
    uint8_t sreg=SREG;
    cli();
    if (s->list) release(s);
    SREG=sreg;
}

// Release only if count < 1 
static inline void release_mutex(semaphore *s)
{
    uint8_t sreg=SREG;
    cli();
    if (s->count < 1) release(s);
    SREG=sreg;
}

// Initialize a thread stack and suspend the entry function on the runnable
// list. main() must call this once for each thread, with a dfferent stack
// array for each.
void init_thread(void(* thread)(void), uint8_t *stack, int size);

// Return the approximate number of bytes unused on specified stack.
int stackspace(uint8_t *stack);

// Tell interested drivers that we're using threads
#define THREADED
