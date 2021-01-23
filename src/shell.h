
/// @brief Stores the working directory, username and hostname
typedef struct prompt {
	char *wd;
	char *uname;
	char *hostname;
} prompt;

/// @brief Stores the command, arguments, size of args and flags for other operations
typedef struct command {
    char *cmd;
    char **args;
    int size;
	short isBackground;
	short isBuiltin;
} command;

/// @brief Stores array of parsed commands, size of the array, array of special operators and its size
typedef struct cmdList {
	command *commandList;
	int commandSize;
	char *spcOps;
	int spcSize;
} cmdList;

cmdList *getParsed(char *);
void printParsed();


