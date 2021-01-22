#include<stdio.h>
#include<stdlib.h>
#include "shell.h"
#define FG 0
#define BG 1
#define PAUSED 2
#define DONE 3

typedef struct jobs{
	long jobID;
	long pID;
	int status;
	commandList cmd;
} jobs;

typedef struct joblist{
	int size;
	jobs *j;
} jobList;

jobs *init(){
	jobList *jl;
	jl->size = 0;
	jl->j = NULL;
	return jl;
}

void add(jobList *jl,long pid,int status, commandList c){
	int size = jl->size;
	jobs *temp = (jobs *)malloc(sizeof(jobs));
	temp->jobID = size + 1;
	temp->pID = pid;
	temp->status = status;
	temp->commandList = c;

}
