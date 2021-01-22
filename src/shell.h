
/// @brief Stores the working directory username and hostname
typedef struct prompt {
	char *wd;
	char *uname;
	char *hostname;
}prompt;

// typedef struct command{
// 	char *comd;
// 	char **arguments;
// 	int size;
// 	short background;
// }command;

typedef struct command {
    char *cmd;
    char **args;
    int size;
	short background;
	short isBuiltin;
} command;

typedef struct cmdList {
	command *commandList;
	int commandSize;
	char *spcOps;
	int spcSize;
} cmdList;

cmdList *getParsed(char *);

void printParsed();


