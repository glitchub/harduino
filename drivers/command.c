// Command line parser and execution

#ifndef THREAD
#error "Command processing requires the thread driver"
#endif

// String scanning states
#define WHITE 0
#define UNQUOTE 1
#define SINGLE 2
#define DOUBLE 3

// lookup command by name or alias
extern _command *__commands_start, *__commands_end;
static _command *lookup(char *name)
{
    for (_command **c = &__commands_start; c < &__commands_end; c++)
        if (!strcmp(name, (*c)->name) || !(!(*c)->alias || strcmp(name, (*c)->alias)))
            return *c;
    return NULL;
}

// Delete character at *s, slide all following characters left
static inline void del(char *s) { for(;*s;s++) *s = *(s+1); }

// adjust these as required to save stack
#define MAXARGS 6 // max tokens per line, including the command
#define MAXLEN 64 // max command line length

// parse and execute command string, return 0 on success, non-zero on error
int8_t execute(char *s)
{
    char *argv[MAXARGS];
    int8_t argc=0;
    uint8_t state = WHITE;

    while(*s)
    {
        switch (state)
        {
            case WHITE:
                // skipping whitespace
                if (*s != ' ')
                {
                    if (argc >= MAXARGS)
                    {
                        printf("Too many params!\n");
                        return -1;
                    }
                    argv[argc++]=s;
                    state=UNQUOTE;
                    continue;
                }
                break;

            case UNQUOTE:
                // unquoted
                if (*s == ' ')
                {
                    *s++=0;
                    state = WHITE;
                    continue;
                }
                if (*s == '"')
                {
                    del(s);
                    state = DOUBLE;
                    continue;
                }
                if (*s == '\'')
                {
                    del(s);
                    state = SINGLE;
                    continue;
                }
                break;

            case DOUBLE:
                // double-quoted
                if (*s == '"')
                {
                    del(s);
                    state = UNQUOTE;
                    continue;
                }
                break;

            case SINGLE:
                // single-quoted
                if (*s == '\'')
                {
                    del(s);
                    state = UNQUOTE;
                    continue;
                }
                break;
        }
        if (*s == '\\') del(s); // delete escape character, and skip whatever is next
        if (!*s) break;         // end of string
        s++;
    }
    if (!argc) return 1;        // blank line

    _command *c = lookup(argv[0]);
    if (!c)
    {
        printf("Invalid command (try 'help')\n");
        return -1;
    }
    return (c->func)(argc, argv);
}

// Generic commands
COMMAND(help, "?", "show this list")
{
    (void)argc; (void)argv;

    for (_command **c = &__commands_start; c < &__commands_end; c++)
    {
        printf("%-10s : %s", (*c)->name, (*c)->desc ?: "no description");
        if ((*c)->alias) printf(" (alias '%s')", (*c)->alias);
        printf("\n");
    }
    return 0;
}

// read memory
COMMAND(mem, NULL, "read/write memory")
{
    if (argc < 2 || argc > 3) die("Usage: mem address [byte]\n");
    uint16_t addr = (uint16_t)strtoul(argv[1], NULL, 0);
    uint8_t byte;
    if (argc == 3)
    {
        byte = (uint8_t)strtoul(argv[2], NULL, 0);
        *(uint8_t *)addr = byte;
    } else
        byte = *(uint8_t *)addr;
    printf("%s %04X = %02X\n", (argc==3)?"Wrote":"Read", addr, byte);
    return 0;
}

// show uptime
COMMAND(uptime, NULL, "show uptime")
{
    (void)argv; (void) argc;
    uint32_t t=get_ticks();
    printf("%ld.%03d seconds\n", t/1000, (int)(t%1000));
    return 0;
}

// show fuse configuration
COMMAND(fuses, NULL, "show fuses")
{
    (void)argc; (void)argv;

    cli();
    uint8_t lb = boot_lock_fuse_bits_get(GET_LOCK_BITS);
    uint8_t hf = boot_lock_fuse_bits_get(GET_HIGH_FUSE_BITS);
    uint8_t lf = boot_lock_fuse_bits_get(GET_LOW_FUSE_BITS);
    uint8_t xf = boot_lock_fuse_bits_get(GET_EXTENDED_FUSE_BITS);
    sei();
    printf("H=%02X L=%02X X=%02X lock=%02X\n", hf, lf, xf, lb);
    return 0;
}

#ifdef DEBUG_STACKS
COMMAND(stacks, NULL, "show unused stacks")
{
    (void) argc; (void) argv;
    debug_stacks();
    return 0;
}
#endif

// reset CPU
COMMAND(reset, NULL, "reset the CPU")
{
    cli();                  // interrupts off
    wdt_enable(WDTO_15MS);  // enable watchdog
    while(1);               // spin until it expires
    return 0;               // won't get here
}

// user command loop, never returns
void __attribute__((noreturn)) command(const char *prompt)
{
    char cmdline[MAXLEN];
    next: while(1)
    {
        uint8_t n=0;                                // number of chars
        printf("%s ", prompt);                      // prompt

        while(1)
        {
            char c=getchar();                       // wait for a char
            switch(c)
            {
                case '\n':
                    printf("\n");
                    cmdline[n]=0;                   // zero terminate
                    execute(cmdline);
                    goto next;

                case '\b':                          // backspace
                    if (n)                          // if we have chars
                    {
                        n--;                        // go back one
                        printf("\b \b");
                    }
                    break;

                case ' ':
                    if (!n) break;                  // avoid leading spaces
                    // fall thru

                case '!' ... '~':                   // printable
                    if (n < sizeof(cmdline)-1)      // if it will fit
                    {
                        cmdline[n++]=c;             // append it
                        putchar(c);
                    }
                    break;
            }
        }
    }
}
