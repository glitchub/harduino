// A command definition has a name, alias (NULL ok), short description (NULL
// ok), and function address,
typedef struct
{
    char *name, *alias, *desc;
    int8_t (*func)(int8_t argc, char *argv[]);
} _command;

// Given name, alias, usage, command, create _command record and add its
// address tp the .commands section.
#define COMMAND(n, a, d, f) \
  static const _command * f ## _command __attribute__((used,section(".commands"))) = &(_command){ .name=n, .alias=a, .desc=d, .func=f }

// Execute a command string and return 0 if success, or non-zero if error
int8_t execute(char *s);

// Command input loop, never returns
void __attribute__((noreturn)) command(const char *prompt);

// Used within a command function, print message and return -1
#define die(...) ({ printf(__VA_ARGS__); return -1; })

