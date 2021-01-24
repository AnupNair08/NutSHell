#include<stdio.h>
#include<stdlib.h>
#include "shell.h"
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>



jobList *initJobList(){
	jobList *jl = (jobList *)malloc(sizeof(jobList));
	jl->size = 0;
	return jl;
}


// char *getCommand(cmdList *c){
// 	char *cmd = (char *)malloc(128);
// 	char t[2];
// 	for(int i = 0 ;i < c->commandSize ; i++){
// 		strcat(cmd, c->commandList->cmd);
// 		if(i > 0){
// 			t[0] = c->spcOps[i-1];
// 			t[1] = '\0';
// 			strcat(cmd, t);
// 		}
// 	}
// 	return cmd;
// }

char *getStatus(int statusCode){
	switch (statusCode)
	{
	case 1:
		return "Foreground Running";
		break;
	case 2:
		return "Background Running";
		break;
	case 3:
		return "Stopped";
		break;
	case 4:
		return "Done";
		break;
	default:
		return NULL;
		break;
	}
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
	// printf("List of jobs: %d\n", jobl->size);
	for(int i = 0 ; i < jobl->size ; i++){
		printf("[%d] %d %s \n", jobl->jl[i].jobid , jobl->jl[i].pid, getStatus(jobl->jl[i].status));
	}
	return;
}

void checkzombie(jobList *jobs,int pid){
	int status;
	pid_t rpid = waitpid(pid, &status, WNOHANG);
	if(rpid == pid){
		if (WIFEXITED(status)) {
			setStatus(jobs,pid, DONE);
		} else if (WIFSTOPPED(status)) {
			setStatus(jobs,pid, STOP);
		} else if (WIFCONTINUED(status)) {
			setStatus(jobs,pid, CONTINUE);
		}
	}
}

void freeJobs(jobList *jobs){
	for(int i = 0 ; i < jobs->size ; i++){
		if (jobs->jl[i].status == FOREGROUND || jobs->jl[i].status == BACKGROUND){
			checkzombie(jobs,jobs->jl[i].pid);
		}
	}
	printJobs(jobs);
	int k = 0;
	int completed[64];
	for(int i = 0 ; i < jobs->size ; i++){
		if(jobs->jl[i].status == DONE){
			completed[k++] = jobs->jl[i].jobid;
		} 
	}
	for(int i = 0 ; i < k ; i++){
			deleteJob(jobs, jobs->jl[completed[i]].jobid);
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