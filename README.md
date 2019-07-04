Harduino - native support for AVR-based systems such as Arduino. Only requirements are avr-gcc/binutils, avr-libc, and the avrdude downloader.

Projects:

    Each project has its own directory, containing at least main.c, main.h, and
    make.inc.

    main.c must, of course, contain the project's main().

    main.h is imported before any driver header, so can specify driver-specific
    defintions. It must also define:

        BOARD=..., controls board-specific GPIO definitions. Currently the
        only supported value is uno_r3.

        MHZ=..., the CPU clcok frequency, required if BOARD is not specified.
        Supported values at 8 and 16.

        TICKMS=... - number of milliseconds per tick interrupt. Only certain
        values are supported, see tick.c. Larger values reduce idle CPU current
        A good default is 4.

    make.inc is included by the Makefile, and defines:
        
        CHIP=..., this is mandatory, specifies the target AVR device, e.g.
        atmega328p (this needs to agree with BOARD in main.h)

        DRIVERS=..., this is the list of drivers to build. It's optional, if
        main.c only manipulates gpios.

Core:

    The ./core directory contains files that are always built fo all projects,
    including board-specific gpio definitions, timer and thread support.

Drivers:

    The ./drivers directory contains a .c and a .h for each driver.

Make targets:

    To build a project for the first time, run

        make <project directory name>

    The project name will be stored so you don't need to specify it again
    (unless you want to build a different project).

    Try "make help" to show other targets.
