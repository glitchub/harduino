# Harduino Makefile

# Basename of the generated binary
TARGET=test

# Files to compile
SOURCES=main.c lcd.c wait.c ticks.c serial.c

# CPU, for avr-gcc
CHIP=atmega328p

# BOARD, includes corresponding file "${BOARD}.h'
BOARD=uno_r3

# CPU clock in HZ
CLOCK=16000000UL

# Toolchain prefix
PREFIX=avr-

# Try to find Arduino serial via udev or default to /dev/ttyACM0
SERIAL=$(firstword $(wildcard /dev/serial/by-id/usb-Arduino*) /dev/ttyACM0)

.PHONY: default
default: ${TARGET}.hex

${TARGET}.hex: ${TARGET}.elf
	${PREFIX}objcopy -O ihex $< $@

${TARGET}.elf: $(addsuffix .o, $(basename ${SOURCES}))
	${PREFIX}gcc -mmcu=${CHIP} -Wl,-Map=${TARGET}.map -o $@ $^
	${PREFIX}objdump -aS $@ > $(basename $@).lst

%.o: %.c
	${PREFIX}gcc -DF_CPU=${CLOCK} -mmcu=${CHIP} -include ${BOARD}.h ${DEBUG} -Os -Wall -Werror -std=gnu99 -c -o $@ $< 

.PHONY: install
install: ${TARGET}.hex
	avrdude -q -c arduino -p ${CHIP} -P ${SERIAL} -b 115200 -U flash:w:$<:i

.PHONY: clean
clean:; rm -f *o *.elf *.hex *.lst *.map
