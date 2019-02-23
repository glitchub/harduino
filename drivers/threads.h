// Cooperative threading for AVR
//
// A thread is simply a function that has its own stack. It can call other
// functions, it can create stack variables, no special coding is required
// except that each thread must occasionally invoke yield() or suspend() in
// order to relinquish the CPU.
//
// Threads can be defined with a special macro, as follows:
//
//      THREAD(my_thread,100)
//      {
//          while(1)
//          {
//              yield() or suspend();
//              do stuff;
//          }
//      }
// 
#define THREAD(_thread,_size) \
    static void _thread(void); \
    ADD_THREAD(_thread,_size); \
    static void __attribute__((used,noreturn)) _thread(void)                             
// The THREAD macro defines the function with the given name and allocates the
// specified number of bytes for its stack.
//
// Since the thread must not return, the function is defined with the noreturn
// attribute to ensure that.
//
// The specified stack size must be large enough to accommodate the thread's
// deepest stack variable allocation (including function calls, library usage,
// etc), plus whatever the worst-case interrupt handler requires, plus 20 bytes
// for suspended thread context. 64 bytes is about the minimum possible size,
// 128 or 256 is reasonable for threads of moderate complexity.
//
// Thread functions are never called explicitly, instead main() calls
// start_threads() to initialize all threads within their own stack frames.
// main() also becomes a thread. There is no explicit thread dispatcher.
//
// The system maintains a "runnable" list of all threads which are not
// suspended on semaphores and are waiting to execute.

// Inter-thread communication, arbitration, and synchronization is accomplished
// with semaphores.
//
// A semaphore is just a data structure:
typedef volatile struct
{
    void *list;             // head of linked list of suspended threads
    unsigned char count;    // number of available resources
} semaphore;
// Warning, altering this definition will require changes to assembly language
// in threads.c.

// A semaphore can be pre-initialized with the number of available resources
// (e.g. a mutex is initialized to 1):
#define available(n) {.count=n}

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
static inline bool is_suspended(semaphore *s)
{
    uint8_t sreg=SREG;
    cli();
    bool b = s->list != NULL;
    SREG=sreg;
    return(b);
}

// Return true if the semaphore has at least one unhandled release
// Note is_suspended() and is_released() cannot both be true at once.
static inline bool is_released(semaphore *s)
{
    uint8_t sreg=SREG;
    cli();
    bool b = s->count != 0;
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

// A thread_frame struct is defined for each thread, which contains a pointer
// to the function, stack array, and stack size (and possibly the name). The
// the address of the thread_frame is appended to the array in the .threads
// section, this allows the thread frames to be iterated at runtime.
// XXX this should move to PROGMEM and .threads should be part of .text.
typedef struct
{
#ifdef DEBUG_STACKS
    char *name;
#endif
    void(* thread)(void);
    int size;
    uint8_t *stack;
} thread_frame;

// The tfsym nonsense is required to generate a unique pointer name. The name
// is never actually used since the pointer is accessed via the .threads
// section, it is required purely to make the pointer definition valid.
#define ___tfsym(a,b) a##b
#define __tfsym(a,b) ___tfsym(a,b)
#define _tfsym(a) __tfsym(a,__LINE__)

// Given a thread function name and a stack size, create a thread_frame,
// allocate its stack, add the thread_frame address to the .threads section.
#ifdef DEBUG_STACKS
// For stack debugging we need to save the thread name as a string
#define ADD_THREAD(_thread,_size) \
    static const thread_frame * _tfsym(_thread) __attribute__((used,section(".threads")))=\
        &(thread_frame){.name=#_thread, .thread=_thread, .size=_size, .stack=(uint8_t[_size]){}};
#else
#define ADD_THREAD(_thread,_size) \
    static const thread_frame * _tfsym(_thread) __attribute__((used,section(".threads")))=\
        &(thread_frame){.thread=_thread, .size=_size, .stack=(uint8_t[_size]){}};
#endif
// ADD_THREAD is ionvoked by the THREAD() macro, it can also be invoked
// directly to add a manually created thread, or to run the same thread
// function as two or more independent threads.

// These point to the start and end of the .threads section, exported by the
// linker, see the .lds file generated by the build.
extern thread_frame *__threads_start, *__threads_end;

// Using the symbols above, start each thread. Note the start order is
// undefined.
void start_threads(void);

#ifdef DEBUG_STACKS
// Given the handle of the serial device, report unused stack size of each
// thread.
void debug_stacks(FILE *serial);
#endif

// Tell interested drivers we're using threads
#define THREADED
