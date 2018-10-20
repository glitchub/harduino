# Harduino Makefile

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
    PROJECT:=$(subst .current.,,$(wildcard .current.*))
endif

ifneq (${PROJECT},)

CFLAGS=-g -Os -Wall -Werror -std=gnu99 --save-temps

include ${PROJECT}/make.inc

VPATH=${PROJECT} drivers

.PHONY: ${PROJECT} default
${PROJECT} default: .current.${PROJECT} ${PROJECT}.hex

.current.${PROJECT}:; make distclean && touch $@

${PROJECT}.hex: ${PROJECT}.elf; ${PREFIX}objcopy -O ihex $< $@

${PROJECT}.elf: main.o $(addsuffix .o,${FILES})
	${PREFIX}gcc -mmcu=${CHIP} -Wl,-Map=${PROJECT}.map -o $@ $^
	${PREFIX}objdump -aS $@ > $(basename $@).lst
	${PREFIX}size $@

%.o: %.c %.h ${PROJECT}/main.h
	${PREFIX}gcc -mmcu=${CHIP} -I ./drivers -I ./${PROJECT} -include ${PROJECT}/main.h -c ${CFLAGS} -c -o $@ $< 

.PHONY: install download 
install download: ${PROJECT}.hex; ./download -c ${CHIP} $<

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
clean:; rm -f *.o *.elf *.hex *.lst *.map *.s *.i 

.PHONY: distclean
distclean: clean; rm -f .current.*

# any unknown target
.DEFAULT:; @echo "Unknown target: $@"; echo; $(call help)
