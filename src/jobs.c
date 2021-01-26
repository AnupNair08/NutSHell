#include<stdio.h>
#include<stdlib.h>
#include "shell.h"
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>


int currentPCB;
int currentJob;
jobList completedBgJobs[16];

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

/**
 * @brief Get the Status string
 * 
 * @param statusCode One of the five possible status code
 * @return char* 
 */
char *getStatus(int statusCode){
	switch (statusCode)
	{
	case 1:
		return "Running[FG]";
		break;
	case 2:
		return "Running[BG]";
		break;
	case 3:
		return "Stopped";
		break;
	case 4:
		return "Done";
		break;
	case 5:
		return "Continued";
		break;
	default:
		return NULL;
		break;
	}
}

/**
 * @brief Adds a job to the JobList
 * 
 * @param jobl Job List in memory
 * @param pid Process ID of the leader
 * @param c Array of commands that have been parrsed
 * @param status Status of the job
 * 
 * @return int
 */
int addJob(jobList *jobl, int pid, cmdList *c, int status){
	job j;
	if(currentPCB == c->pcbid){
		j.jobid = currentJob;
	}
	else{
		j.jobid = jobl->size + 1;
		currentJob = j.jobid;
		currentPCB = c->pcbid;
	}
	j.pid = pid;
	j.c = c;
	j.status = status;
	jobl->jl[jobl->size++] = j;
	return 1;
}

/**
 * @brief Changes the Status of a job
 * 
 * @param jobs 
 * @param pId 
 * @param status 
 * @return int 
 */
int setStatus(jobList *jobs,int pId, int status){
	// Returns 1 on success and 0 if no such job exists
	for(int i = 0 ; i < jobs->size ; i++){
		if(jobs->jl[i].pid == pId){
			if(jobs->jl[i].status == BACKGROUND){
				addJob(completedBgJobs, jobs->jl[i].pid,jobs->jl[i].c,DONE);
			}
			jobs->jl[i].status = status;
			break;
		}
	}
	if(status == DONE){
		freeJobs(jobs);	
	}
	return 1;
}


/**
 * @brief Deletes a job with the given job id
 * 
 * @param jobs 
 * @param jobId 
 * @return int 
 */
int deleteJob(jobList *jobs, int jobId){
	// Returns the number of jobs deleted
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
	return 1;
}

char *get_process_name(const pid_t pid) {
	char *name = (char *)malloc(128);
	char procfile[BUFSIZ];
	sprintf(procfile, "/proc/%d/cmdline", pid);
	FILE* f = fopen(procfile, "r");
	if (f) {
		size_t size;
		size = fread(name, sizeof (char), sizeof (procfile), f);
		if (size > 0) {
			if ('\n' == name[size - 1])
				name[size - 1] = '\0';
		}
		fclose(f);
	}
	return name;
}


/**
 * @brief Utility function to print all jobs
 * 
 * @param jobl 
 * @return int 
 */
int printJobs(jobList *jobl){
	// Returns the number of jobs that were printed
	for(int i = 0 ; i < completedBgJobs->size; i++){
		printf("[%d]+ %d %s %s\n", completedBgJobs->jl[i].jobid , completedBgJobs->jl[i].pid, getStatus(completedBgJobs->jl[i].status), get_process_name(completedBgJobs->jl[i].pid));
	}
	freeJobs(completedBgJobs);
	for(int i = 0 ; i < jobl->size ; i++){
		printf("[%d]+ %d %s %s\n", jobl->jl[i].jobid , jobl->jl[i].pid, getStatus(jobl->jl[i].status), get_process_name(jobl->jl[i].pid));
	}
	return jobl->size;
}

/**
 * @brief Helper function to print a Job given the ID
 * 
 * @param jobl 
 * @param pid 
 */
void printJobID(jobList *jobl, int pid){
	for(int i = 0 ; i < jobl->size ; i++){
		if(jobl->jl[i].pid == pid){
			printf("[%d] %d %s \n", jobl->jl[i].jobid , jobl->jl[i].pid, getStatus(jobl->jl[i].status));
			return;
		}
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
			setStatus(jobs,pid, STOPPED);
		} else if (WIFCONTINUED(status)) {
			setStatus(jobs,pid, CONTINUE);
		}
	}
}


/**
 * @brief Function to delete completed job from the job list
 * 
 * @param jobs 
 * @return int 
 */
int freeJobs(jobList *jobs){
	// Returns the number of jobs freed
	for(int i = 0 ; i < jobs->size ; i++){
		if (jobs->jl[i].status == FOREGROUND || jobs->jl[i].status == BACKGROUND){
			checkzombie(jobs,jobs->jl[i].pid);
		}
	}
	// printJobs(jobs);
	int k = 0;
	int completed[64];
	for(int i = 0 ; i < jobs->size ; i++){
		if(jobs->jl[i].status == DONE){
			completed[k++] = jobs->jl[i].jobid;
		} 
	}
	for(int i = 0 ; i < k ; i++){
			deleteJob(jobs,completed[i]);
	}
	return k;
}


void bringFg(jobList *jobs, int id, int type){
	int flag = type == JOBID ? 1 : 0;
	int status;
	for(int i = 0 ; i < jobs->size; i++){
		if(flag){
			if(jobs->jl[i].jobid == id){
				puts(get_process_name(jobs->jl[i].pid));
				kill(jobs->jl[i].pid, SIGCONT);
				setStatus(jobs,jobs->jl[i].pid,FOREGROUND);
				waitpid(jobs->jl[i].pid,&status,0);
				setStatus(jobs,jobs->jl[i].pid,DONE);
				return;
			}
		}
		else{
			if(jobs->jl[i].pid == id){
				puts(get_process_name(jobs->jl[i].pid));
				kill(jobs->jl[i].pid, SIGCONT);
				setStatus(jobs,jobs->jl[i].pid,FOREGROUND);
				waitpid(jobs->jl[i].pid,&status,0);
				setStatus(jobs,jobs->jl[i].pid,DONE);
				return;
			}
		}
	}
	printf("No such job\n");
	return;
}
