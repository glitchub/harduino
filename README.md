Harduino - support for Arduino hardware without the Arduino IDE.

Requires avr-gcc and friends, and avrdude downloader.

Files include:

    lcd.c - support for generic LCD modules (what Arduino calls
    "LiquidCrystal")

    tick.c - support for background millisecond counter

    wait.c - inline waituS() and waitmS() delay functions, for very short
    delays

    main.c - example application to execise all other functions.

    Makefile - build the designated application.

Currently supports 16Mhz UNO since that's what I have, in theory other
platforms can be added without too much effort.

Make changes to the makefile as necessary for your environment, then 'make' to
build the example (as test.hex), and 'make install' to download it to target.   
