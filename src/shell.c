#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<signal.h>
#include "shell.h"
#define CMD_SIZE 128
#define MAX_SIZE 128

/// @brief Global pointer to store the current working directory
char *cwd;
int hist;

/**
 * @brief Function to handle Ctrl+C
 * 
 */
void handleexit(){
	printf("\nExiting...\n");
	close(hist);
	exit(0);
}


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
void printPrompt(char *name, char *hostname, char *cwd){
	printf("\033[1;31m");
	printf("%s@", name);
	printf("%s:", hostname);
	printf("\x1B[34m");
	printf("%s",cwd);
	printf("\033[0m");
	printf("$ ");
	return;
}


/**
 * @brief Function to execute a single command 
 * 
 * @param p Pointer to the command to be executed
 */
void runCmd(command *p){
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
		if(execvp(p->cmd,arg) == -1){
			perror("");
			// Including this exit cleanly exits out of a process that ended up in an error, thus not causing the exit loops
			exit(-1);
		}
	}
	else{
		wait(NULL);
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
void startShell(prompt p){
	// hist = open(".sh_hist", O_CREAT | O_APPEND | O_RDWR);
	// if(hist == -1){
	// 	perror("History feature startup failed\n");
	// }
	cwd = p.wd;
	int pid;
	char *cmd = (char *)malloc(sizeof(char) * CMD_SIZE);
	command *parsedCmd;
	while(1){

	  	printPrompt(p.uname, p.hostname, cwd);
		
		fgets(cmd,MAX_SIZE,stdin);
		// Empty input handler
		if(strcmp(cmd,"\n") == 0 || strlen(cmd) == 0){
			continue;
		}	
		//write(hist,cmd,strlen(cmd));
		parsedCmd = NULL;
		cmdList *cl = getParsed(strtok(cmd,"\n"));
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
		
		
		if(parsedCmd->background){
			printf("BG PROCESS");
		}
		if(parsedCmd->isBuiltin){
				if (strcmp(parsedCmd->cmd,"exit") == 0) {
				//	printf("I will be Bourne Again.\n");
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
				continue;
		}
		runCmd(parsedCmd);
	}
	
	return;
}

int main(int argc, char *argv[]){
	signal(SIGINT,handleexit);
	printf("Welcome to Dead Never SHell(DNSh).\n");
	prompt p = getPrompt();
	//doubleStack* hist = readHistory();
	startShell(p);
	return 0;
}

