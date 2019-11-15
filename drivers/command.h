// A command definition has a name, alias (NULL ok), short description (NULL
// ok), and function address,
typedef struct
{
    char *name, *alias, *desc;
    int8_t (*func)(int8_t argc, char *argv[]);
} _command;

// Given name, alias string, and description string, create _command record and command function preface.
#define COMMAND(n, a, d) \
    static int8_t n ## _commandfunc (int8_t argc, char *argv[]); \
    static const _command * n ## _command __attribute__((used, section(".commands"))) = &(_command) { .name = #n, .alias = a, .desc = d, .func = n ## _commandfunc }; \
    static int8_t __attribute__((used)) n ## _commandfunc (int8_t argc, char *argv[])

// Execute a command string and return 0 if success, or non-zero if error
int8_t execute(char *s);

// Command input loop, never returns
void __attribute__((noreturn)) command(const char *prompt);

// Used within a command function, print message and return -1
#define die(...) ({ printf(__VA_ARGS__); return -1; })

