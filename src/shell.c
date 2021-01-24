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
#define CMD_SIZE 128
#define MAX_SIZE 128
#define RED "\033[1;31m"
#define BLUE "\x1B[34m"
#define RESET "\033[0m"

/// @brief Global pointer to store the current working directory
char *cwd;
int hist;
int fd;
/// @brief List to store the jobs for processing bg and fg operations
jobList *jobs;

///
///  @brief Function that generates the prompt to be displayed on the shell
///  @return Structure that contains the username and the current working directory
prompt getPrompt(){
	prompt p;
	p.wd = (char *)malloc(sizeof(char) * MAX_SIZE);
	p.uname = (char *)malloc(sizeof(char) * MAX_SIZE);
	p.hostname = (char *)malloc(sizeof(char) * MAX_SIZE);
	getcwd(p.wd,MAX_SIZE);
	gethostname(p.hostname,MAX_SIZE);
	p.uname = getenv("USER");
	return p;
}


/**
 * @brief Utility function to print colored text on terminal
 * 
 * @param name Username of the system
 * @param hostname Hostname of the system
 * @param cwd Current working directory
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
	// pid_t childpid = wait(NULL);
	// printf("parent alerted %d\n",childpid);
	// setStatus(jobs,childpid,4);
	printf("Ctrl + Z recieved");
	return;
}

void handleStop(){
	printf("Stopped");
	return;
}

/**
 * @brief Initialises the shell and makes it run as foreground process.
 *  	  Gets process id of the shell and sets the process group id equal to it
 * 		  Gives the control of the terminal to the process group id  
 * 
 */
void initShell(){
	// Get default terminal
	char *term = (char *)malloc(128);
    ctermid(term);
    fd = open(term,O_RDONLY);
	// Ignore signals on the foreground processes  
	signal (SIGINT, SIG_IGN);
	signal (SIGTSTP, SIG_IGN);
	signal (SIGTTOU, SIG_IGN);
	signal (SIGCHLD, SIG_IGN);

	// Get the process id of the shell's main process
	int shellpid = getpid();
	// Set the shell process as the group leader
	setpgid(shellpid, shellpid);
	// Transfer the control to the shell
	tcsetpgrp(fd,shellpid);
	return;
}



/**
 * @brief Function to execute a single command 
 * 
 * @param cl Pointer to the command list
 */
void runCmd(cmdList *cl){
	command *p = &(cl->commandList[0]);
	pid_t pid = fork();
	if(pid < 0) {
		perror("");
		exit(-1);
	}
	if(pid == 0){  
		// Get the child's process id and make it the group leader if there is none
		int childpid = getpid();
		if (getpgid(childpid) == 0){
			setpgid(childpid,childpid);
		}
		// Takes signals from the terminal if it is a foreground process
		if(!p->isBackground){
			signal (SIGINT, SIG_DFL);
			signal (SIGTSTP, SIG_DFL);
			signal (SIGTTOU, SIG_DFL);
			signal (SIGCHLD, SIG_DFL);
			// signal (SIGTTOU, SIG_DFL);	
			tcsetpgrp(fd,getpgid(childpid));
			
		}
		char *arg[p->size + 2];
		arg[0] = p->cmd;
		for(int i = 0 ; i < p->size ; i++){
			arg[i+1] = p->args[i];
		}
		arg[p->size + 1] = 0;
		if(execvp(p->cmd,arg) == -1){
			perror("");
			// Including this exit cleanly exits out of a process that ended up in an error, thus not causing the exit loops
			exit(-1);
		}
	}
	else{
		int status;
		if (p->isBackground){
		// Run background processes with WNOHANG as it does not wait for the child process to exit
			p->isBackground = 0;
			printJobID(jobs, pid);
			addJob(jobs,pid,cl,BACKGROUND);
			waitpid(pid,&status, WNOHANG);
		}
		else{
			addJob(jobs,pid,cl,FOREGROUND);
			pid_t ppid = waitpid(pid,&status,WUNTRACED|WCONTINUED);
			if(WIFSTOPPED(status)){
				setStatus(jobs,pid,STOPPED);
			}
			else if(WIFCONTINUED(status)){
				setStatus(jobs,pid,CONTINUE);
			}
			else if(WIFEXITED(status)){
				setStatus(jobs,pid,DONE);
			}
		}
	}
	return;
}

/**
 * @brief Runs commands that have redirection of I/O
 * 
 * @param p Pointer to command to be executed 
 * @param fileName Name of the file to be used as I/O
 * @param type String denoting the type of redirection
 */
void runRedirCmd(command *p, char *fileName, char *type){
	int fd;
	if(type == "out"){
		fd = open(fileName, O_CREAT | O_RDWR);
	}
	else{
		fd = open(fileName, O_RDONLY);
	}
	if(fd == -1){
		perror("");
		return;
	}
	int pid = fork();
	if(pid < 0) {
		perror("");
		exit(-1);
	}
	if(pid == 0){  
		char *arg[p->size + 2];
		arg[0] = p->cmd;
		for(int i = 0 ; i < p->size ; i++){
			arg[i+1] = p->args[i];
		}
		arg[p->size + 1] = 0;
		if(fd != -1 && strcmp(type,"out") == 0){
			dup2(fd,1);
		}
		else if(fd != -1 && strcmp(type, "in") == 0){
			dup2(fd,0);
		}
		if(execvp(p->cmd,arg) == -1){
			perror("");
			exit(-1);
		}
		close(fd);
	}
	else{
		wait(NULL);
	}
	return;
}

/**
 * @brief Runs piped commands
 * 
 * @param l Pointer to the left command
 * @param right Pointer to the right command
 */
void runPipe(command *l, command *right){
	// prog 1 | prog2
	// prog1 writes to stdout which can be linked to a pipefd1
	// prog2 reads from stdin which can be linked tp pipefd0
	
	/*parent writes to fd1 and child reads from fd0*/
	/*parent closes the read end and child closes the write end*/
	
	//we have a pipe made such that fd[1] stdout --- stdin fd[0]

	//			       --> read its inp from stdin fd[0]
	//			       |
	//	parent --> prog1 --> prog2
	//		    |
	//		    -> writes its op to stdout fd[1]
	
	char buf[10];
	int pidfd[2];
	int pid = fork();
	int r=pipe(pidfd);
	if(r < 0){
		perror("");
		exit(1);
	}
	if(pid == -1){
		perror("");
		exit(1);
	}
	//child 1
	if(pid == 0){
		int pid2 = fork();
		//child of child 1
		//executes prog1
		if(pid2 == 0){
			close(pidfd[0]);
			dup2(pidfd[1],1);
			close(pidfd[1]);
			
			char *arg[l->size + 2];
			arg[0] = l->cmd;
			for(int i = 0 ; i < l->size ; i++){
				arg[i+1] = l->args[i];
			}
			arg[l->size + 1] = 0;
			if(execvp(l->cmd,arg) == -1){
				perror("");
				exit(-1);
			}
		}
		//parent of child 2 i.e child 1 executes prog2
		else{
			close(pidfd[1]);
			dup2(pidfd[0],0);
			close(pidfd[0]);
			char *arg[right->size + 2];
			arg[0] = right->cmd;
			for(int i = 0 ; i < right->size ; i++){
				arg[i+1] = right->args[i];
			}
			arg[right->size + 1] = 0;
			if(execvp(right->cmd,arg) == -1){
				perror("");
				exit(-1);
			}
			wait(NULL);
		}
	}
	if(pid){
		close(pidfd[0]);
		close(pidfd[1]);
		wait(NULL);
	}
	return;
}



/// 
/// @brief Function to run the shell loop that forks new processes and invokes the exec system call to execute commands
///
/// @param p Prompt structure to be printed on the terminal
///
void startShell(prompt p, stack *s){
	hist = open(".sh_hist", O_CREAT | O_APPEND | O_RDWR);
	if(hist == -1){
		perror("History feature startup failed\n");
	}
	cwd = p.wd;
	int pid;
	char *cmd = (char *)malloc(sizeof(char) * CMD_SIZE);
	command *parsedCmd;
	while(1){
		p.wd = cwd;
	  	printPrompt(p);
		
		fgets(cmd,MAX_SIZE,stdin);
		char *buf;

		
		// Bad input handler
		if(cmd == NULL || strcmp(cmd,"\n") == 0 || strlen(cmd) == 0){
			continue;	
		}
		// History WIP	
		if(strcmp(cmd,"!!\n") == 0){
			// buf = handleArrowUp(h);
			buf = pop(s);
			if(buf == NULL || strcmp(buf,"") == 0){
				fprintf(stderr,"No history stored\n");
				continue;
			}
			cmd = buf;
			printPrompt(p);
			printf("%s", cmd);
		}
		else{
			write(hist,cmd,strlen(cmd));
			push(s,cmd);
		}

		parsedCmd = NULL;
		cmdList *cl = getParsed(strtok(cmd,"\n"));
		// printf("%d %d",cl->commandSize, cl->spcSize);
		if(cl->commandSize != cl->spcSize + 1){
			fprintf(stderr,"Parse error: Unexpected syntax\n");
			continue;
		}
		// Handle only single command with arguments
		if (cl->commandSize == 1){
			parsedCmd = &(cl->commandList[0]);
		}
		else {
			int fd;
			for(int i = 0 ; i < cl->spcSize ; i++){
				switch(cl->spcOps[i]){
					case '>':
						runRedirCmd(&(cl->commandList[i]),cl->commandList[i + 1].cmd,"out");
						break;
					case '<':
						runRedirCmd(&(cl->commandList[i]),cl->commandList[i+1].cmd,"in");
						break;
					case '|':
						runPipe(&(cl->commandList[i]),&(cl->commandList[i+1]));
						break;
					default:
						break;
				}
			}
			continue;
		}
		
		
		
		if(parsedCmd == NULL){
			continue;
		}
		
		if(parsedCmd->isBuiltin){
				if (strcmp(parsedCmd->cmd,"exit") == 0) {
				//	printf("I will be Bourne Again.\n");
					close(hist);
					exit(0);
				}
				else if (strcmp(parsedCmd->cmd,"help") == 0) {
					printf("Help from the shell\n");
				}
				else if (strcmp(parsedCmd->cmd,"cd") == 0) {
					if(parsedCmd->args[0] == NULL){
						parsedCmd->args[0] = (char *)malloc(128);
						parsedCmd->args[0] = getenv("HOME");
					}
					if (chdir(parsedCmd->args[0]) == 0){
						prompt newPrompt = getPrompt();
						cwd = newPrompt.wd;
					}
					else{
						perror("");
					}
				}
				else if (strcmp(parsedCmd->cmd,"history") == 0){
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
				else if (strcmp(parsedCmd->cmd,"jobs") == 0){
					freeJobs(jobs);
				}
				continue;
		}
		runCmd(cl);
		
	}
	
	return;
}

int main(int argc, char *argv[]){
	initShell();
	jobs = initJobList();
	printf("Welcome to Dead Never SHell(DNSh).\n");
	prompt p = getPrompt();
	// doubleStack* h = readHistory();
	stack *s = stackInit();
	startShell(p, s);
	return 0;
}

