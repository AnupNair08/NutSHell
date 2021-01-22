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

char *cwd;
command c;
int hist;
// command cmdList[128];

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


/// @brief Utility Function to print colored text
/// @param name hostname cwd
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


// ///  @brief Function to parse the input command and process it for the shell.
// ///
// ///  @param cmd String that has been input by the user into the shell
// /// 
// ///  @return Pointer to a structure that holds the commands and the list of args passed to them or NULL in case of a built in command
// /// 
// command* parseOld(char *cmd){
// 	int i = 0;
// 	char *tok = (char *)malloc(sizeof(char) * MAX_SIZE);
// 	command *c = (command *)malloc(sizeof(command));
// 	tok = strtok(cmd, "\n");
// 	tok = strtok(tok, " ");
// 	c->cmd = tok;
// 	c->size = 0;
// 	c->args = (char **)malloc(sizeof(char *) * MAX_SIZE/4);
// 	c->args[0] = (char *)malloc(sizeof(char) * MAX_SIZE);
// 	c->args[0] = "";
// 	// Tokenise the args list
// 	while(tok) {
// 		tok = strtok(NULL," ");	
// 		if(tok == NULL){
// 			break;
// 		}
// 		if(strcmp(tok,"&") == 0){
// 			c->background = 1;
// 			break;
// 		}
// 		c->args[i] = (char *)malloc(sizeof(char) * MAX_SIZE);
// 		c->args[i] = tok;
// 		c->size++;
// 		i++;
// 	}

// 	// Built in commands
	
// 	return c;
// }

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


void runRedirCmd(command *p, int fd, char *type){
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
			// Including this exit cleanly exits out of a process that ended up in an error, thus not causing the exit loops
			exit(-1);
		}
		close(fd);
	}
	else{
		wait(NULL);
	}
	return;
}



/// 
/// @brief Function to run the shell loop that forks new processes and invokes the exec system call to execute commands
///
/// @param userVal Global variable that holds the username cwdVal Global Variable that holds the current working directory
///
/// @return void
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
						fd = open(cl->commandList[i + 1].cmd, O_CREAT | O_RDWR);
						runRedirCmd(&(cl->commandList[i]),fd,"out");
						break;
					case '<':
						fd = open(cl->commandList[i + 1].cmd, O_RDONLY);
						if(fd == -1){
							perror("");
							break;
						}
						runRedirCmd(&(cl->commandList[i]),fd,"in");
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
		// pid = fork();
		// if(pid < 0) {
		// 	perror("");
		// 	exit(-1);
		// }
		// if(pid == 0){  
		// 	char *arg[parsedCmd->size + 2];
		// 	arg[0] = parsedCmd->cmd;
		// 	for(int i = 0 ; i < parsedCmd->size ; i++){
		// 		arg[i+1] = parsedCmd->args[i];
		// 	}
		// 	arg[parsedCmd->size + 1] = 0;
		// 	if(execvp(parsedCmd->cmd,arg) == -1){
		// 		perror("");
		// 		// Including this exit cleanly exits out of a process that ended up in an error, thus not causing the exit loops
		// 		exit(-1);
		// 	}
		// }
		// else{
		// 	wait(NULL);
		// }
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
	/*char buf[128];*/
	/*fgets(buf,128,stdin);*/
	/*parseHelper(buf);*/
	return 0;
}

