#define MAX_JOB_SIZE 64
#define FOREGROUND 1
#define BACKGROUND 2
#define STOPPED 3
#define DONE 4

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


cmdList *getParsed(char *);
void printParsed();

jobList *initJobList();
void addJob(jobList *,int,cmdList *, int);
void setStatus(jobList *, int, int);
void deleteJob(jobList *, int);
void printJobs(jobList *);
void freeJobs(jobList *);


