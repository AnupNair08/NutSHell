typedef struct command {
    char *cmd;
    char **args;
    int size;
    int isBuiltin;
} command;

typedef struct pipeCmd {
    command *left;
    command *right;
} pipeCmd;

typedef struct redirCmd {
    command *cmd;
    char *fileName;
} redirCmd;


