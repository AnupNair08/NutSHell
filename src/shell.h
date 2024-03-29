#define MAX_JOB_SIZE 64
#define FOREGROUND 1
#define BACKGROUND 2
#define STOPPED 3
#define DONE 4
#define CONTINUE 5
#define JOBID 1
#define PROCESSID 2
#define CMD_SIZE 128
#define MAX_SIZE 128
#define RED "\033[1;31m"
#define BLUE "\x1B[34m"
#define RESET "\033[0m"



/// @brief Stores the working directory, username and hostname.
typedef struct prompt {
	char wd[MAX_SIZE];
	char uname[MAX_SIZE];
	char hostname[MAX_SIZE];
} prompt;

/// @brief Stores the command, arguments, size of args and flags for other operations.
typedef struct command {
    char *cmd;
    char **args;
    int size;
	short isBackground;
	short isBuiltin;
    short pipein;
    short pipeout;
    char *infile;
    char *outfile;
} command;

/// @brief Stores array of parsed commands, size of the array, array of special operators and its size.
typedef struct cmdList {
	command *commandList;
	int commandSize;
    int opSize;
    int tokenSize;
    int pcbid;
} cmdList;


/// @brief Stores a job which is a collection of processes.
typedef struct job {
    int jobid;
    int pid;
    cmdList *c;
    int status;
} job;

/// @brief Data Structure to keep track of all jobs.
typedef struct jobList {
    job jl[MAX_JOB_SIZE];
    int size;
} jobList;


typedef struct node {
    struct node *next;
    char *data;
} node;

typedef struct stack {
    node *s;
    node *top;
} stack;

// Parsing related functions.
cmdList *getParsed(char *);
int printParsed(cmdList *);
void printCommand(command );

// Job realted functions.
jobList *initJobList();
int addJob(jobList *,int,cmdList *, int);
int setStatus(jobList *, int, int);
int deleteJob(jobList *, int);
int printJobs(jobList *);
int freeJobs(jobList *);
void printJobID(jobList *, int);
void bringFg(jobList *, int, int);
void sendBg(jobList *, int, int);


// Stack related functions.
stack *stackInit();
char *pop(stack *);
void push(stack *, char *);
void printStack(stack *);


