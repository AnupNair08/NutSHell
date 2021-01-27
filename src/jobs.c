#include<stdio.h>
#include<stdlib.h>
#include "shell.h"
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<signal.h>

int currentPCB;
int currentJob;
jobList completedBgJobs[16];
 /**
  * @brief Initialises the job list to store the list of jobs under control
  * 
  * @return jobList* 
  */
jobList *initJobList(){
	jobList *jl = (jobList *)malloc(sizeof(jobList));
	jl->size = 0;
	return jl;
}


// char *getCommand(cmdList *c){
// 	char *cmd = (char *)malloc(MAX_SIZE);
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


/**
 * @brief Utility function to get the name of the process from it's process id
 * 
 * @param pid Process id 
 * @return char* 
 */
char *getProcName(pid_t pid) {
	char *name = (char *)malloc(MAX_SIZE);
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
		printf("[%d]+ %d %s %s\n", completedBgJobs->jl[i].jobid , completedBgJobs->jl[i].pid, getStatus(completedBgJobs->jl[i].status), getProcName(completedBgJobs->jl[i].pid));
	}
	freeJobs(completedBgJobs);
	for(int i = 0 ; i < jobl->size ; i++){
		printf("[%d]+ %d %s %s\n", jobl->jl[i].jobid , jobl->jl[i].pid, getStatus(jobl->jl[i].status), getProcName(jobl->jl[i].pid));
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

/**
 * @brief Handles the resumption of stopped and background jobs that are brought to the foreground
 * 
 * @param jobs Job List in current context
 * @param j Job to be handled
 * @param fd File descriptor of the terminal process
 */
void waitProcess(jobList *jobs,job j, int fd){
	int status;
	puts(getProcName(j.pid));
	if(j.status == STOPPED){
		kill(j.pid, SIGCONT);
		setStatus(jobs, j.pid,FOREGROUND);
	}
	else if (j.status == BACKGROUND){
		tcsetpgrp(fd,j.pid);
		// printf("%d", tcgetpgrp(fd));
		kill(j.pid, SIGTSTP);
		kill(j.pid, SIGCONT);
		setStatus(jobs,j.pid,FOREGROUND);
	}

	waitpid(j.pid,&status,WUNTRACED);
	pid_t pid = j.pid;
	if(WIFSTOPPED(status)){
		setStatus(jobs,pid,STOPPED);
		tcsetpgrp(fd,getpid());
	}
	else if(WIFCONTINUED(status)){
		setStatus(jobs,pid,CONTINUE);
		tcsetpgrp(fd,pid);
	}
	else if (WIFEXITED(status)){
		setStatus(jobs,pid,DONE);
		tcsetpgrp(fd,getpid());
	}
	else{
		setStatus(jobs,pid,DONE);
	}
	return;
}



/**
 * @brief Brings stopped and background jobs to the foreground
 * 
 * @param jobs Job List in current context
 * @param id Job ID / Process ID to be brought to the foreground
 * @param type Flag to specify if Job ID or PID is passed as argument
 */
void bringFg(jobList *jobs, int id, int type){
	char *term = (char *)malloc(MAX_SIZE);
    ctermid(term);
    int fd = open(term,O_RDONLY);
	int flag = type == JOBID ? 1 : 0;
	int status, pid;
	for(int i = 0 ; i < jobs->size; i++){
			if(flag && jobs->jl[i].jobid == id){
				waitProcess(jobs,jobs->jl[i],fd);
				close(fd);
				return;	
			}
			else if(jobs->jl[i].pid == id){
				waitProcess(jobs,jobs->jl[i],fd);
				close(fd);
				return;
			}
	}
	printf("No such job\n");
	close(fd);
	return;
}

/**
 * @brief Resumes a stopped proces in the background
 * 
 * @param jobs Job List in context
 * @param id Job ID / Process ID to be brought to the foreground
 * @param type Flag to specify if Job ID or PID is passed as argument
 */
void sendBg(jobList *jobs, int id, int type){
	char *term = (char *)malloc(MAX_SIZE);
    ctermid(term);
    int fd = open(term,O_RDONLY);
	int flag = type == JOBID ? 1 : 0;
	pid_t pid;
	int status;
	for(int i = 0 ; i < jobs->size; i++){
		pid = jobs->jl[i].pid;
		if(flag){
			if(jobs->jl[i].jobid == id && jobs->jl[i].status == STOPPED){
				kill(pid, SIGCONT);
				setStatus(jobs,jobs->jl[i].pid,BACKGROUND);
			}
			waitpid(pid, &status, WNOHANG);
			
		}
		else{
			if(jobs->jl[i].pid == id && jobs->jl[i].status == STOPPED){
				kill(pid, SIGCONT);
				setStatus(jobs,jobs->jl[i].pid,BACKGROUND);
			}
		}
	}
	printf("No such job\n");
	close(fd);
	return;
}
