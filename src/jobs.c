#include<stdio.h>
#include<stdlib.h>
#include "shell.h"

jobList *initJobList(){
	jobList *jl = (jobList *)malloc(sizeof(jobList));
	jl->size = 0;
	return jl;
}

void addJob(jobList *jobl, int pid, cmdList *c, int status){
	job j;
	j.jobid = jobl->size + 1;
	j.pid = pid;
	j.c = c;
	j.status = status;
	jobl->jl[jobl->size++] = j;
	return;
}

void setStatus(jobList *jobs,int pId, int status){
	for(int i = 0 ; i < jobs->size ; i++){
		if(jobs->jl[i].pid == pId){
			jobs->jl[i].status = status;
			return;
		}
	}
	return;
}

void deleteJob(jobList *jobs, int jobId){
	int index = 0;
	for(int i = 0 ; i < jobs->size ; i++){
		if(jobs->jl[i].jobid == jobId){
			index = i;
			break;
		}
	}
	for(int i = index ; i < jobs->size - 1; i++){
		jobs->jl[i] = jobs->jl[i+1];
	}
	jobs->size--;
	return;
}


void printJobs(jobList *jobl){
	printf("List of jobs: %d\n", jobl->size);
	for(int i = 0 ; i < jobl->size ; i++){
		printf("[%d] %d %d\n", jobl->jl[i].jobid , jobl->jl[i].pid, jobl->jl[i].status);
	}
	return;
}

void freeJobs(jobList *jobs){
	printJobs(jobs);
	for(int i = 0 ; i < jobs->size ; i++){
		if(jobs->jl[i].status == DONE){
			deleteJob(jobs, jobs->jl[i].jobid);
		}
	}
	return;
}


// int main(){
// 	jobList *jobs = initJobList();
// 	addJob(jobs,100,NULL,FOREGROUND);
// 	addJob(jobs,101,NULL,FOREGROUND);
// 	setStatus(jobs,100,DONE);
// 	// deleteJob(jobs,2);
// 	printJobs(jobs);
// 	freeJobs(jobs);
// }