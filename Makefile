# Harduino Makefile

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
        awk '{a[$$1]=$$2}END{print "$< requires",a[".data"]+a[".bss"],"bytes of RAM,",a[".data"]+a[".text"]+a[".noinit"],"bytes of FLASH"}'

${BUILD}/${PROJECT}.hex: ${BUILD}/${PROJECT}.elf; ${PREFIX}objcopy -O ihex $< $@

${BUILD}/${PROJECT}.elf: ${BUILD}/${PROJECT}.lds ${OBJS}
	${PREFIX}gcc -mmcu=${CHIP} -T$< -Wl,-Map=$(basename $@).map -o $@ ${OBJS}
	${PREFIX}objdump -aS $@ > $(basename $@).lst

# insert .threads section immediately before end of .text and emit start and end symbols
${BUILD}/${PROJECT}.lds: ${BUILD}/default.lds
	awk '/^ *_edata *= *\. *;$$/{print "PROVIDE (__threads_start = .);\n*(.threads)\nPROVIDE (__threads_end = .);";ok=1}{print}END{exit !ok}' $< > $@

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

.PHONY: install download
install download: ${BUILD}/${PROJECT}.hex; ./download -c ${CHIP} $<

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
clean:; rm -f ${BUILD}/*

.PHONY: distclean
distclean:; rm -rf ${BUILD}

# any unknown target
.DEFAULT:; @echo "Unknown target: $@"; echo; $(call help)
