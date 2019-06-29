Harduino - native support for AVR-based systems such as Arduino. Only requirements are avr-gcc/binutils, avr-libc, and the avrdude downloader.

Projects:

    Each project has its own directory, containing at least main.c, main.h, and
    make.inc.

    main.c must, of course, contain the project's main().

    main.h includes all other headers and contains project-specific definitions
    for each driver. gcc will include main.h automatically when building all
    other files, most explicit #includes should not be required.

    make.inc defines the toolchain PREFIX, the target CPU, and list of all
    FILES to build (other than main).

Drivers:

    Each driver has a .c file and a .h file in the ./drivers directory.
    Drivers typically require static definitions in main.h, e.g. what CPU pins
    to use, what CPU clock to assume, etc.

    If the project includes the 'threads' driver then some other drivers will
    be built with threading support (notably ticks and serial).

    The drivers directory also contains pin definitions for each known Arduino
    board, for example uno_r3.h.

Make targets:

    To build a project for the first time, run

        make <project directory name>

    The project name will be stored so you don't need to specify it again
    (unless you want to build a different project).

    Try "make help" to show other targets.
