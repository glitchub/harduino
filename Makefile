# Harduino Makefile

ifeq ($(filter clean spotless,$(MAKECMDGOALS)),) 

# default to last project
PROJECT=$(shell readlink .*.ln 2>/dev/null)
ifeq ($(PROJECT),) 
$(error "No project defined, please run 'make PROJECT=project'")
endif

include ${PROJECT}/make.inc

VPATH=${PROJECT} drivers

.PHONY: default
default: .${PROJECT}.ln ${PROJECT}.hex

.${PROJECT}.ln:; make spotless && ln -sf ${PROJECT} $@

${PROJECT}.hex: ${PROJECT}.elf; ${PREFIX}objcopy -O ihex $< $@

${PROJECT}.elf: main.o $(addsuffix .o,${FILES})
	${PREFIX}gcc -mmcu=${CHIP} -Wl,-Map=${PROJECT}.map -o $@ $^
	${PREFIX}objdump -aS $@ > $(basename $@).lst

%.o: %.c; ${PREFIX}gcc -mmcu=${CHIP} -I ./drivers -I ./${PROJECT} -include ${PROJECT}/main.h -g -Os -Wall -Werror -std=gnu99 --save-temps -c -o $@ $< 

.PHONY: download 
download: ${PROJECT}.hex; ./download -c ${CHIP} $<

endif

.PHONY: clean
clean:; rm -f *.o *.elf *.hex *.lst *.map *.s *.i 

.PHONY: spotless 
spotless: clean; rm -f .*.ln

