#define MAX_JOB_SIZE 64
#define FOREGROUND 1
#define BACKGROUND 2
#define STOPPED 3
#define DONE 4
#define CONTINUE 5

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
    int pipein;
    int pipeout;
    char *infile;
    char *outfile;
} command;

/// @brief Stores array of parsed commands, size of the array, array of special operators and its size
typedef struct cmdList {
	command *commandList;
	int commandSize;
    int opSize;
    int tokenSize;
    int pcbid;
} cmdList;



typedef struct job {
    int jobid;
    int pid;
    cmdList *c;
    int status;
    struct job *next;
} job;


typedef struct jobList {
    job jl[MAX_JOB_SIZE];
    int size;
} jobList;


typedef struct stack {
    char s[128][128];
    int top;
} stack;


cmdList *getParsed(char *);
int printParsed(cmdList *);

void printCommand(command );

jobList *initJobList();
void addJob(jobList *,int,cmdList *, int);
void setStatus(jobList *, int, int);
void deleteJob(jobList *, int);
void printJobs(jobList *);
void freeJobs(jobList *);
void printJobID(jobList *, int);
stack *stackInit();
char *pop(stack *);
void push(stack *, char *);


