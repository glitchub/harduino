Harduino - support for Arduino hardware without the Arduino IDE.

Requires avr-gcc and friends, and avrdude downloader.

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

    Each driver has a .c file and a .h file, in the ./drivers directory.
    Drivers typically require static definitions in main.h, e.g. what CPU pins
    to use, what CPU clock to assume, etc.

    The drivers directory also contains pin definitions for each known Arduino
    board, for example uno_r3.h.

Make argets:

    To build a particular project for the first time, run "make
    PROJECT=basename". This will create a symlink to the project directory, so
    you don't need to specify PROJECT again (unless you want to build a
    different project).

    Other targets include:

        make download - download the project hex file. You can also run the
        "download" script directly if you need to specify a differnt serial
        device.

        make clean - remove generated files

        make spotless - remove generated files and the .project.ln symlink.
