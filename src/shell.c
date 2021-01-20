#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<signal.h>
#include "format.h"
#include "shell.h"
#include "utility.h"
#define CMD_SIZE 128
#define MAX_SIZE 128

char *user, *cwd;
command c;
int hist;

void handleexit(){
	printf("\nExiting...\n");
	close(hist);
	exit(0);
}



///  @brief Function that generates the prompt to be displayed on the shell
///
///
///   @return Structure that contains the username and the current working directory
///
///
prompt getPrompt(){
	prompt p;
	p.wd = (char *)malloc(sizeof(char) * MAX_SIZE);
	p.uname = (char *)malloc(sizeof(char) * MAX_SIZE);
	getcwd(p.wd,MAX_SIZE);
	p.uname = getenv("USER");
	return p;
}

///  @brief Function to parse the input command and process it for the shell.
///
///  @param cmd String that has been input by the user into the shell
/// 
///  @return Pointer to a structure that holds the commands and the list of arguments passed to them or NULL in case of a built in command
/// 
command* parse(char *cmd){
	int i = 0;
	char *tok = (char *)malloc(sizeof(char) * MAX_SIZE);
	
	tok = strtok(cmd, "\n");
	tok = strtok(tok, " ");
	c.comd = tok;
	c.size = 0;
	c.arguments = (char **)malloc(sizeof(char *) * MAX_SIZE/4);
	c.arguments[0] = (char *)malloc(sizeof(char) * MAX_SIZE);
	c.arguments[0] = "";
	// Tokenise the arguments list
	while(tok) {
		tok = strtok(NULL," ");	
		if(tok == NULL){
			break;
		}
		c.arguments[i] = (char *)malloc(sizeof(char) * MAX_SIZE);
		c.arguments[i] = tok;
		c.size++;
		i++;
	}

	// Built in commands
	if (strcmp(c.comd,"exit") == 0) {
	//	printf("I will be Bourne Again.\n");
		close(hist);
		exit(0);
	}
	else if (strcmp(c.comd,"help") == 0) {
		printf("Help from the shell\n");
		printHelp();
		free(c.arguments);
		return NULL;
	}
	else if (strcmp(c.comd,"cd") == 0) {
		c.arguments[0] = strcmp(c.arguments[0],"") != 0 ? c.arguments[0] : getenv("HOME");
		if (chdir(c.arguments[0]) == 0){
			prompt newPrompt = getPrompt();
			cwd = newPrompt.wd;
			free(c.arguments);
			return NULL;
		}
		else{
			perror("");
			return NULL;
		}
	}
	else{
		return &c;
	}
}
/// 
/// @brief Function to run the shell loop that forks new processes and invokes the exec system call to execute commands
///
/// @param userVal Global variable that holds the username cwdVal Global Variable that holds the current working directory
///
///@return void
///
void startShell(char *userVal, char *cwdVal){
	hist = open(".sh_hist", O_CREAT | O_APPEND | O_RDWR);
	if(hist == -1){
		perror("History feature startup failed\n");
	}
	user = userVal;
	cwd = cwdVal;
	int pid;
	char *cmd = (char *)malloc(sizeof(char) * CMD_SIZE);
	while(1){
		red();
		printf("%s ", user);
		blue();
		printf("@ %s: $ ",cwd);
		reset();
		fgets(cmd,MAX_SIZE,stdin);
		if(strcmp(cmd,"\n") == 0){
			continue;
		}	
	//	write(hist,cmd,strlen(cmd));
		command *parsedCmd = parse(cmd);
		if(parsedCmd == NULL){
			continue;
		}
		pid = fork();
		if(pid < 0) {
			perror("");
			exit(-1);
		}
		if(pid == 0){  
			char *arg[parsedCmd->size + 2];
			arg[0] = parsedCmd->comd;
			for(int i = 0 ; i < parsedCmd->size ; i++){
				arg[i+1] = parsedCmd->arguments[i];
			}
			arg[parsedCmd->size + 1] = 0;
			if(execvp(parsedCmd->comd,arg) == -1){
				perror("");
				// Including this exit cleanly exits out of a process that ended up in an error, thus not causing the exit loops
				exit(-1);
			}
		}
		else{
			wait(NULL);

		}
	}
	
	return;
}

int main(){
	signal(SIGINT,handleexit);
	printf("Welcome to Dead Never SHell(DNSh).\n");
	prompt p = getPrompt();
	//doubleStack* hist = readHistory();
	startShell(p.uname, p.wd);
	return 0;
}

