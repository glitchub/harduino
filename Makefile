# Harduino Makefile

PREFIX=avr-

BUILD=./build

# A project is any subdirectory containing 'make.inc'
PROJECTS:=$(shell for f in */make.inc; do echo $${f%/*}; done)

# But make sure there's no overlap with .PHONY targets in this file
DISALLOW=$(filter ${PROJECTS},$(shell grep '^\s*\..*:' Makefile))
ifneq (${DISALLOW},)
$(error Error, found disallowed project directory name "${DISALLOW}")
endif

GOAL=$(firstword $(MAKECMDGOALS))

ifneq ($(filter $(GOAL),$(PROJECTS)),)
    PROJECT:=$(GOAL)
else
    # default to current project
    PROJECT:=$(subst .,,$(suffix $(wildcard ${BUILD}/.current.*)))
endif

ifneq (${PROJECT},)

CFLAGS=-g -Os -Wall -Werror -std=gnu99
#CFLAGS+=-save-temps

# this defines ${FILES}
include ${PROJECT}/make.inc

OBJS=$(addprefix ${BUILD}/,main.o $(addsuffix .o,${FILES}))

VPATH=${PROJECT} drivers

.PHONY: ${PROJECT} default
${PROJECT} default: ${BUILD}/${PROJECT}.hex
        # print memory usage
	@${PREFIX}size -A $(basename $<).elf | \
	awk '/A __data_load_end/ { flashuse=strtonum("0x" $$1) } \
	     /N __heap_start/ { ramuse=(strtonum("0x" $$1) % 65536) - 256 } \
	     /W __stack/ { ramsize=(strtonum("0x" $$1) % 65536) - 255 } \
	     END { print "$< requires",ramuse,"bytes of RAM,",flashuse,"bytes of FLASH"; if (ramuse > ramsize) { print "ERROR, RAM EXCEEDS",ramsize,"BYTES"; exit(1) } }'

${BUILD}/${PROJECT}.hex: ${BUILD}/${PROJECT}.elf; ${PREFIX}objcopy -O ihex $< $@

${BUILD}/${PROJECT}.elf: ${BUILD}/${PROJECT}.lds ${OBJS}
	${PREFIX}gcc -mmcu=${CHIP} -T$< -Wl,-Map=$(basename $@).map -o $@ ${OBJS}
	${PREFIX}objdump -aS $@ > $(basename $@).lst

# insert .threads section immediately before end of .text and emit start and end symbols
${BUILD}/${PROJECT}.lds: ${BUILD}/default.lds
	{ \
	  cat $< | \
	  awk '/^ *_edata *= *\. *;$$/{print "PROVIDE (__threads_start = .);\n*(.threads)\nPROVIDE (__threads_end = .);";ok=1}{print}END{exit !ok}' | \
	  awk '/^ *_edata *= *\. *;$$/{print "PROVIDE (__commands_start = .);\n*(.commands)\nPROVIDE (__commands_end = .);";ok=1}{print}END{exit !ok}' \
	; } >$@

# gcc outputs default linker script with leading and trailing line of ==='s, use awk to extracts it to file or fail if couldn't
${BUILD}/default.lds: ${BUILD}/.current.${PROJECT}
	${PREFIX}gcc -mmcu=${CHIP} -Wl,-verbose 2>/dev/null | awk '/^=+$$/{n++; next}n==1{print}END{exit n!=2}' > $@

${BUILD}/%.o: %.c %.h main.h ${BUILD}/.current.${PROJECT}
	${PREFIX}gcc -mmcu=${CHIP} -I ./drivers -I ./${PROJECT} -include ${PROJECT}/main.h ${CFLAGS} -c -o $@ $<
	${PREFIX}objdump -aS $@ > $(basename $@).lst

# create empty build directory if necessary
${BUILD}/.current.${PROJECT}:
	rm -rf ${dir $@}
	mkdir ${dir $@}
	touch $@

.PHONY: install
install: ${BUILD}/${PROJECT}.hex
	avrdude -q -c arduino -p "${CHIP} -P "$(firstword $(wildcard /dev/serial/by-id/usb-Arduino*) /dev/ttyACM0)" -b 115200 -U "flash:w:$<:i"

# Project simulation: 'make sim' in one window, and then 'make gdb' in another.
# simavr is from https://github.com/buserror/simavr
.PHONY: sim gdb
sim: ${BUILD}/${PROJECT}.elf; simavr -m ${CHIP} -g $<
gdb: ${BUILD}/${PROJECT}.elf; avr-gdb -tui -ex "target remote :1234" -ex "break main" -ex "cont" $<

endif

# generic rules go here, help should be first
define help
echo 'Current project    : $(if ${PROJECT},${PROJECT},NOT DEFINED!)'
echo 'Available projects : ${PROJECTS}'
echo
echo 'make project-name  - remember project-name as "current", and build it'
echo 'make               - build current project'
echo 'make download      - build and download current project to target Arduino'
echo 'make clean         - remove generated files for current project'
echo 'make distclean     - same as above but also forget "current"'
echo 'make help          - (or any invalid target) print this text'
endef

.PHONY: help
help:; @$(call help)

.PHONY: clean
clean:; rm -f ${BUILD}/* *.s *.i

.PHONY: distclean
distclean:; rm -rf ${BUILD}

# any unknown target
.DEFAULT:; @echo "Unknown target: $@"; echo; $(call help)
