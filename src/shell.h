typedef struct prompt {
	char *wd;
	char *uname;
}prompt;

typedef struct command{
	char *comd;
	char **arguments;
	int size;
	char *fileName;
	short background;
}command;
