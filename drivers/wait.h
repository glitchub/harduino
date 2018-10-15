#if F_CPU==16000000UL
#define MAXUS 16384
#elif F_CPU==8000000L
#define MAXUS 32768
#else
#error "F_CPU not supported"
#endif

// Spin delay up to MAXUS microseconds
void waituS(uint16_t uS);

// Spin delay up to 65535 milliseconds
void waitmS(uint16_t mS);
