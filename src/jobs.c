#include<stdio.h>
#include<stdlib.h>
#include "jobs.h";

jobList *init(){
	jobList jl = (jobList *)malloc(sizeof(jobList));
	jl->size = 0;
	return jl;
}

void add(jobList *jl, int pid, command *c, int status){
	job *j = (job *)malloc(sizeof(job));
	j->jobid = jl->size + 1;
	j->pid = pid;
	j->c = c;
	j->status = status;
	j->next = NULL; 
	return;
}