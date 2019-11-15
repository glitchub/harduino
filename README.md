Harduino - native support for AVR-based systems such as Arduino. Only
requirements are avr-gcc/binutils, avr-libc, and the avrdude downloader.

Projects:

    Each project has its own directory, containing at least main.c, main.h, and
    make.inc.

    main.c must, of course, contain the project's main().

    main.h is imported before any driver header, it can define:

        BOARD=name - controls board-specific GPIO definitions. Currently the
        only supported value is uno_r3. This is mandatory.

        TICKMS=number - the minimum milliseconds per tick interrupt. Larger
        values reduce idle CPU current but reduce tick resolution. If not
        defined, the default is 4. Only certain values are valid, other values
        will be rounded up (see ticks.c)

        WATCHDOG=number - the minimum milliseconds to allow for blocking
        interrupts before resetting the CPU. Max is 8192. Set to 0 to disable
        the watchdog completely. If not defined, the default is 128.

        Driver-specific definitions, as seen in the driver source files.

    make.inc is included by the Makefile, and defines:

        CHIP=name - this is mandatory, specifies the target AVR device, e.g.
        atmega328p (this needs to agree with BOARD in main.h)

        DRIVERS=list... - this is the list of drivers required by the project,
        from the drivers directory.

Drivers:

    The ./drivers directory contains a .c and a .h for each driver. Some
    drivers require project-specific definitions which are defined in the
    project's main.h.

Core:

    The ./core directory contains files that are always built fo all projects,
    including board-specific gpio definitions and timer support.

Make targets:

    To build a project for the first time, run

        make <project directory name>

    The project name will be stored so you don't need to specify it again
    (unless you want to build a different project).

    Try "make help" to show other targets.
