// Definitions for Arduino Uno R3

#define GPIO00 GPIO(D,0)
#define GPIO01 GPIO(D,1)
#define GPIO02 GPIO(D,2)
#define GPIO03 GPIO(D,3)
#define GPIO04 GPIO(D,4)
#define GPIO05 GPIO(D,5)
#define GPIO06 GPIO(D,6)
#define GPIO07 GPIO(D,7)
#define GPIO08 GPIO(B,0)
#define GPIO09 GPIO(B,1)
#define GPIO10 GPIO(B,2)
#define GPIO11 GPIO(B,3)
#define GPIO12 GPIO(B,4)
#define GPIO13 GPIO(B,5)
#define GPIOA0 GPIO(C,0)
#define GPIOA1 GPIO(C,1)
#define GPIOA2 GPIO(C,2)
#define GPIOA3 GPIO(C,3)
#define GPIOA4 GPIO(C,4)
#define GPIOA5 GPIO(C,5)

// Map outputs for TIMER0 and TIMER1
#define PWM0 GPIO06 // OC0A
#define PWM1 GPIO05 // 0C0B
#define PWM2 GPIO09 // OC1A
#define PWM3 GPIO10 // OC1B

// Map pins for SPI
#define SPI_SS   GPIO10
#define SPI_MOSI GPIO11
#define SPI_MISO GPIO12
#define SPI_SCK  GPIO13
