// A command definition has a name, alias (NULL ok), short description (NULL
// ok), and function address,
typedef struct
{
    char *name, *alias, *desc;
    void (*func)(int8_t argc, char *argv[]);
} _command;

// Given name, alias string, and description string, create _command record and command function preface.
#define COMMAND(n, a, d) \
    static void n ## _commandfunc (int8_t argc, char *argv[]); \
    static const _command * n ## _command __attribute__((used, section(".commands"))) = &(_command) { .name = #n, .alias = a, .desc = d, .func = n ## _commandfunc }; \
    static void __attribute__((used)) n ## _commandfunc (__attribute__((unused)) int8_t argc, __attribute__((unused)) char *argv[])

// Execute a command string and return 0 if success, or non-zero if error
int8_t execute(char *s);

// Command input loop, never returns
void __attribute__((noreturn)) command(const char *prompt);

// Used within a command function, print message and return
#define die(...) ({ pprintf(__VA_ARGS__); return; })

