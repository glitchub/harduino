#if F_CPU==16000000UL
#define MAXUS 16384
#elif F_CPU==8000000L
#define MAXUS 32768
#else
#error "Invalid F_CPU"
#endif

// Spin delay up to MAXUS microseconds
void waituS(unsigned int uS);

// Spin delay up to 65535 milliseconds
void waitmS(unsigned int mS);
