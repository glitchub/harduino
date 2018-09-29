# Harduino Makefile

# Basename of the generated binary
TARGET=test

# Files to compile
SOURCES=main.c lcd.c wait.c ticks.c

# CPU, for avr-gcc
CHIP=atmega328p

# CPU clock in HZ
CLOCK=16000000UL

# Toolchain prefix (include path if needed)
PREFIX=avr-

# avrdude serial device  
SERIAL=/dev/ttyACM0

# comment out for production build
DEBUG=-g

${TARGET}.hex: ${TARGET}.elf
	${PREFIX}objcopy -O ihex $< $@

${TARGET}.elf: $(addsuffix .o, $(basename ${SOURCES}))
	${PREFIX}gcc -mmcu=${CHIP} -o $@ $^
	${PREFIX}objdump -aS $@ > $(basename $@).lst

%.o: %.c
	${PREFIX}gcc -DF_CPU=${CLOCK} -mmcu=${CHIP} ${DEBUG} -Os -Wall -Werror -std=gnu99 -c -o $@ $< 

.PHONY: install
install: ${TARGET}.hex
	avrdude -q -c arduino -p ${CHIP} -P ${SERIAL} -b 115200 -U flash:w:$<:i

.PHONY: clean
clean:; rm -f *.elf *.hex *.o *.lst
