#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<signal.h>
#include "shell.h"

int hist;
/// @brief Global pointer to store the current working directory.
char *cwd;
/// @brief File descriptor to store the context of current terminal.
int terminalFd;
/// @brief Prompt that is displayed on the terminal.
prompt p;
/// @brief List to store the jobs for processing bg and fg operations.
jobList *jobs;


/**
 * @brief Function that generates the prompt to be displayed on the shell.
 * @return Structure that contains the username and the current working directory.
 */
prompt getPrompt(){
	prompt p;
	getcwd(p.wd,MAX_SIZE);
	gethostname(p.hostname,MAX_SIZE);
	strcpy(p.uname,getenv("USER"));
	return p;
}

/**
 * @brief Utility function to print colored text on terminal.
 * 
 * @param name Username of the system.
 * @param hostname Hostname of the system.
 * @param cwd Current working directory.
 */
void printPrompt(prompt p){
	printf(RED"%s@", p.uname);
	printf("%s:", p.hostname);
	printf(BLUE"%s",p.wd);
	printf(RESET"$ ");
	return;
}

/**
 * @brief Signal handler to set completed background process status as done.
 * 
 */
void handleChild(){
	int status;
	pid_t childpid = waitpid(-1,&status,WNOHANG);
	// printf("parent alerted %d\n",childpid);
	setStatus(jobs,childpid,DONE);
	signal(SIGCHLD, handleChild);
	return;
}

/**
 * @brief Initialises the shell and makes it run as foreground process.
 *  	  Gets process id of the shell and sets the process group id equal to it.
 * 		  Gives the control of the terminal to the process group id.  
 * 
 */
void initShell(){
	// Get default terminal
	char *term = (char *)malloc(MAX_SIZE);
    ctermid(term);
    terminalFd = open(term,O_RDONLY);

	// Ignore signals on the shell  
	signal (SIGINT, SIG_IGN);
	signal (SIGTSTP, SIG_IGN);
	signal (SIGTTOU, SIG_IGN);
	signal (SIGCHLD, handleChild);

	int shellpid = getpid();
	// Get the process id of the shell
	// Set the shell process as the group leader
	setpgid(shellpid, shellpid);
	// Transfer the control to the shell
	tcsetpgrp(terminalFd,shellpid);
	free(term);
	return;
}

/**
 * @brief Command to be executed.
 * 
 * @param p Pointer to the structure of the command.
 * @param cmdSize Number of commands in the group.
 * @param pipefd Array of input and output files for the command.
 * @param i Sequence number of the command.
 * @return Process ID of the executed command.
 */
pid_t runCmd(command *p, int cmdSize, int pipefd[2], int i){
	pid_t pid = fork();
	if(pid < 0) {
		perror("");
		exit(EXIT_FAILURE);
	}
	if(pid == 0){  
		// Get the child's process id and make it the group leader if there is none
		int childpid = getpid();
		int pgid = getpgid(childpid);
		
		if (pgid == 0){
			setpgid(childpid,childpid);
		}
		else{
			setpgid(childpid,pgid);
		}
		
		// Takes signals from the terminal for child processes
		signal (SIGINT, SIG_DFL);
		signal (SIGTSTP, SIG_DFL);
		signal (SIGTTOU, SIG_DFL);
		signal (SIGCHLD, SIG_DFL);
		signal (SIGTTOU, SIG_IGN);

		if(!p->isBackground){
			tcsetpgrp(terminalFd,getpgid(childpid));
		}
		
		char *arg[p->size + 2];
		arg[0] = p->cmd;
		for(int i = 0 ; i < p->size ; i++){
			arg[i+1] = p->args[i];
		}
		arg[p->size + 1] = 0;
		

		// All but the first process should have stdin as op of prev proc
		if(i != 0){
			if(dup2(pipefd[0],0) < 0){
				perror("");
				exit(EXIT_FAILURE);
			}
		}
		// All but the last process should output to the stdout
		if(i != cmdSize - 1){
			if(dup2(pipefd[1],1) < 0){
				perror("");
				exit(EXIT_FAILURE);
			}
		}

		// If there is an input file
		if(p->infile){
			int fd = open(p->infile, O_RDONLY);
			if(fd == -1 || dup2(fd,0) < 0){
				perror("");
				return -1;
			}
			close(fd);
		}
		// If there is an output file
		if(p->outfile){
			int fd2 = open(p->outfile, O_CREAT | O_RDWR);
			if(fd2 == -1 || dup2(fd2,1) < 0){
				perror("");
				return -1;
			}
			close(fd2);
		}

		if(execvp(p->cmd,arg) == -1){
			perror("");
			// Including this exit cleanly exits out of a process that ended up in an error, thus not causing the exit loops
			exit(EXIT_FAILURE);
		}
	}

	// freeCommandList(p);
	// free(p->args); 
	return pid;
}

/**
 * @brief Runs the parsed set of commands that are in the form of a process group.
 * 
 * @param cl List of parsed commands.
 */
void runJob(cmdList *cl){
	int size = cl->commandSize;
	int pid;
	int isBg = 0;
	int pipeArray[size -1][2];
	int temp[2] = {0,1};
	
	for(int i = 0 ; i < size - 1;i++){
		pipe(pipeArray[i]);
	}
	
	for(int i = 0 ; i < size; i++){

		if(cl->commandList[i].isBackground) isBg = 1;
		
		if(cl->commandList[i].pipein == 1 && cl->commandList[i].pipeout == 1){
			int temp[2] = {pipeArray[i-1][0], pipeArray[i][1]} ;
			pid = runCmd(&(cl->commandList[i]), cl->commandSize, temp,i);
			close(pipeArray[i][1]);
			continue;
		}
		if(cl->commandList[i].pipeout){
			pid = runCmd(&(cl->commandList[i]), cl->commandSize, pipeArray[i],i);
			close(pipeArray[i][1]);
		}
		if(cl->commandList[i].pipein){
			pid = runCmd(&(cl->commandList[i]), cl->commandSize,pipeArray[i-1],i);
			close(pipeArray[i][0]);
		}
		if(cl->commandList[i].pipein == 0 && cl->commandList[i].pipeout == 0){
			pid = runCmd(&(cl->commandList[i]), cl->commandSize,temp,i);
		}
	}
	if(pid < 0){
		exit(EXIT_FAILURE);
	}
	int status;
	if (isBg){
		// Run background processes with WNOHANG as it does not wait for the child process to exit
		addJob(jobs,pid,cl,BACKGROUND);
		waitpid(pid,&status, WNOHANG) ;
	}
	else{
		addJob(jobs,pid,cl,FOREGROUND);
		waitpid(pid,&status,WUNTRACED);

		if(WIFSTOPPED(status)){
			setStatus(jobs,pid,STOPPED);
			tcsetpgrp(terminalFd,getpid());
		}
		else if (WIFEXITED(status)){
			setStatus(jobs,pid,DONE);
			tcsetpgrp(terminalFd,getpid());
		}
		else{
			setStatus(jobs,pid,DONE);
			tcsetpgrp(terminalFd,getpid());
		}
	}
}

/** 
* @brief Function to run the shell loop that forks new processes and invokes the exec system call to execute commands.
*
* @param p Prompt structure to be printed on the terminal.
*/
void startShell(prompt p){
	// hist = open(".sh_hist", O_CREAT | O_APPEND | O_RDWR);
	// if(hist == -1){
	// 	perror("History feature startup failed\n");
	// }
	cwd = p.wd;
	int pid;
	cmdList *parsedCmd = NULL;
	char *cmd = (char *)malloc(sizeof(char) * MAX_SIZE);
	while(1){
		strcpy(p.wd,cwd);
	  	printPrompt(p);
		fgets(cmd,MAX_SIZE,stdin);
		
		// Bad input handler
		if(cmd == NULL || strlen(cmd) == 0 || strcmp(cmd,"\n") == 0){
			continue;	
		}

		// History WIP	
		// char *buf;
		// if(strcmp(cmd,"!!\n") == 0){
		// 	// buf = handleArrowUp(h);
		// 	buf = pop(s);
		// 	if(buf == NULL || strcmp(buf,"") == 0){
		// 		fprintf(stderr,"No history stored\n");
		// 		continue;
		// 	}
		// 	cmd = buf;
		// 	printPrompt(p);
		// 	// printf("%s", cmd);
		// }
		// else{
		// 	write(hist,cmd,strlen(cmd));
		// 	push(s,cmd);
		// }

		parsedCmd = getParsed(strtok(cmd,"\n"));
		if(parsedCmd == NULL || parsedCmd->tokenSize != parsedCmd->opSize + 1){
			fprintf(stderr,"Parse error: Unexpected syntax\n");
			continue;
		}
		
		
		command *temp = &(parsedCmd->commandList[0]);
		if(temp->isBuiltin){
				if (strcmp(temp->cmd,"exit") == 0) {
				//	printf("I will be Bourne Again.\n");
					close(terminalFd);
					free(jobs);
					free(cmd);
					puts("Exiting...");
					exit(EXIT_SUCCESS);
				}
				else if (strcmp(temp->cmd,"help") == 0) {
					printf("Help from the shell\n");
				}
				else if (strcmp(temp->cmd,"cd") == 0) {
					if(temp->args[0] == NULL){
						temp->args[0] = (char *)malloc(sizeof(char) * MAX_SIZE);
						temp->args[0] = getenv("HOME");
					}
					if (chdir(temp->args[0]) == 0){
						getcwd(cwd, MAX_SIZE);
						if(!temp->args[0]) free(temp->args[0]);
					}
					else{
						perror("");
					}
				}
				else if (strcmp(temp->cmd,"history") == 0){
					printf("Shell History:\n");
					int fd = open(".sh_hist", O_RDONLY);
					struct stat s;
					stat(".sh_hist",&s);
					char *buf = (char *)malloc(sizeof(char)*s.st_size);
					read(fd,buf,s.st_size);
					printf("%s", buf);
					free(buf);
					close(fd);				
				}
				else if (strcmp(temp->cmd,"jobs") == 0){
					printJobs(jobs);
				}
				else if (strcmp(temp->cmd,"fg") == 0){
					char *id = temp->args[0];
					if(id == NULL){
						printf("Usage: <fg [%%]id>\n");
						continue;
					}
					//call by jobID
					if(id[0] == '%'){
						bringFg(jobs, atoi(id+1),JOBID);
					}
					//call by process ID
					else{
						bringFg(jobs, atoi(id),PROCESSID);
					}
				}
				else if (strcmp(temp->cmd,"bg") == 0){
					char *id = temp->args[0];
					if(id == NULL){
						printf("Usage: <bg [%%]id>\n");
						continue;
					}
					//call by jobID
					if(id[0] == '%'){
						sendBg(jobs, atoi(id+1),JOBID);
					}
					//call by process ID
					else{
						sendBg(jobs, atoi(id),PROCESSID);
					}
				}
				if(parsedCmd){
					// freeCommandList(parsedCmd->commandList, parsedCmd->commandSize);
					free(parsedCmd);

				} 
				continue;
		}
		runJob(parsedCmd);
		if(parsedCmd){
			// freeCommandList(parsedCmd->commandList, parsedCmd->commandSize);
			free(parsedCmd);
		} 
		
	}
	
	return;
}

int main(int argc, char *argv[]){
	initShell();
	jobs = initJobList();
	printf("Welcome to Dead Never SHell(DNSh).\n");
	p = getPrompt();
	startShell(p);
	return 0;
}

