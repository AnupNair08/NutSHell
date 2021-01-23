#include "shell.h"
#define MAX_JOB_SIZE 64
#define FOREGROUND 1
#define BACKGROUND 2
#define STOPPED 3
#define DONE 4

typedef struct job {
    int jobid;
    int pid;
    command *c;
    int status;
    struct job *next;
} job;


typedef struct jobList {
    job jl[MAX_JOB_SIZE];
    int size;
} jobList;
