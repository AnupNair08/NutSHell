
// Stores the working directory and the username
typedef struct prompt {
	char *wd;
	char *uname;
	char *hostname;
}prompt;

typedef struct command{
	char *comd;
	char **arguments;
	int size;
	short background;
}command;

void freePrompt(prompt p);
