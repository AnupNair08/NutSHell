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
// #include<readline/readline.h>
// #include<readline/history.h>
#define RED "\033[1;31m"
#define BLUE "\x1B[34m"
#define RESET "\033[0m"

/// @brief Global pointer to store the current working directory
char *cwd;
int hist;
int fd;

/// @brief List to store the jobs for processing bg and fg operations
jobList *jobs;

int pipefd[2];

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
	int status;
	pid_t childpid = waitpid(getpid(),&status,WNOHANG);
	// printf("parent alerted %d\n",childpid);
	setStatus(jobs,childpid,4);
	signal(SIGCHLD, handleChild);
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
	char *term = (char *)malloc(MAX_SIZE);
    ctermid(term);
    fd = open(term,O_RDONLY);
	// Ignore signals on the foreground processes  
	signal (SIGINT, SIG_IGN);
	signal (SIGTSTP, SIG_IGN);
	signal (SIGTTOU, SIG_IGN);
	signal (SIGCHLD, handleChild);

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
void runCmd(command *p, cmdList *cl){
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
			signal (SIGTTOU, SIG_IGN);	
			tcsetpgrp(fd,getpgid(childpid));
		}
		else{
			// signal (SIGINT, SIG_DFL);
			// signal (SIGTSTP, SIG_DFL);
			// signal (SIGTTOU, SIG_DFL);
			// signal (SIGCHLD, SIG_DFL);
			// signal (SIGTTOU, SIG_IGN);		
			tcsetpgrp(fd,getppid());
		}
		char *arg[p->size + 2];
		arg[0] = p->cmd;
		for(int i = 0 ; i < p->size ; i++){
			arg[i+1] = p->args[i];
		}
		arg[p->size + 1] = 0;
		if(p->pipein == 1 && p->pipeout == 1){
			close(pipefd[1]);
			dup2(pipefd[0],0);
			close(pipefd[0]);
			
			if(pipe(pipefd) == -1){
				perror("");
			};
			close(pipefd[0]);
			dup2(pipefd[1],1);
			close(pipefd[1]);
			// write(1,"hello",5);
		}
		else if(p->pipeout){
			close(pipefd[0]);
			dup2(pipefd[1],1);
		}
		else if(p->pipein){
			close(pipefd[1]);
			dup2(pipefd[0],0);
		}


		if(p->infile){
			int fd = open(p->infile, O_RDONLY);
			if(fd == -1){
				perror("");
				return;
			}
			dup2(fd,0);
			close(fd);
		}
		if(p->outfile){
			// puts(p->outfile);
			int fd2 = open(p->outfile, O_CREAT | O_RDWR);
			if(fd2 == -1){
				perror("");
				return;
			}
			dup2(fd2,1);
			close(fd2);
		}

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
			// printJobID(jobs, pid);
			addJob(jobs,pid,cl,BACKGROUND);
			tcsetpgrp(fd,getpid());
			pid_t ppid = waitpid(pid,&status, WNOHANG);
			if (WIFEXITED(status)){
				setStatus(jobs,pid,DONE);
			}
		}
		else{
			addJob(jobs,pid,cl,FOREGROUND);
			pid_t ppid = waitpid(pid,&status,WUNTRACED|WCONTINUED);
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
				tcsetpgrp(fd,getpid());
			}
			//Brings the shell process to the foreground
		}
	}
	return;
}


void runJob(cmdList *cl){
	int size = cl->commandSize;
	for(int i = 0 ; i < size; i++){
		// printCommand((cl->commandList[i]));
		if(cl->commandList[i].pipein == 1 && cl->commandList[i].pipeout == 1){
			runCmd(&(cl->commandList[i]), cl);
			continue;
		}
		if(cl->commandList[i].pipeout){
			pipe(pipefd);
			runCmd(&(cl->commandList[i]), cl);
			close(pipefd[1]);
		}
		if(cl->commandList[i].pipein){
			runCmd(&(cl->commandList[i]), cl);
			close(pipefd[0]);
		}
		if(cl->commandList[i].pipein == 0 && cl->commandList[i].pipeout == 0){
			runCmd(&(cl->commandList[i]), cl);
		}
	}
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
	// hist = open(".sh_hist", O_CREAT | O_APPEND | O_RDWR);
	// if(hist == -1){
	// 	perror("History feature startup failed\n");
	// }
	cwd = p.wd;
	int pid;
	char *cmd = (char *)malloc(sizeof(char) * CMD_SIZE);
	cmdList *parsedCmd;
	// rl_bind_key('\t', rl_complete);
	while(1){
		p.wd = cwd;
	  	printPrompt(p);
		
		fgets(cmd,MAX_SIZE,stdin);
		// cmd = readline(NULL);
		char *buf;

		
		// Bad input handler
		if(cmd == NULL || strlen(cmd) == 0 || strcmp(cmd,"\n") == 0){
			// puts("");
			continue;	
		}
		// History WIP	
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

		parsedCmd = NULL;
		cmdList *cl = getParsed(strtok(cmd,"\n"));
		if(cl->tokenSize != cl->opSize + 1){
			fprintf(stderr,"Parse error: Unexpected syntax\n");
			continue;
		}
		
		parsedCmd = cl;
		if(parsedCmd == NULL){
			continue;
		}
		
		command *temp = &(parsedCmd->commandList[0]);
		if(temp->isBuiltin){
				if (strcmp(temp->cmd,"exit") == 0) {
				//	printf("I will be Bourne Again.\n");
					close(hist);
					exit(0);
				}
				else if (strcmp(temp->cmd,"help") == 0) {
					printf("Help from the shell\n");
				}
				else if (strcmp(temp->cmd,"cd") == 0) {
					if(temp->args[0] == NULL){
						temp->args[0] = (char *)malloc(MAX_SIZE);
						temp->args[0] = getenv("HOME");
					}
					if (chdir(temp->args[0]) == 0){
						prompt newPrompt = getPrompt();
						cwd = newPrompt.wd;
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
					if(id[0] == '%'){
						//call by jobID
						bringFg(jobs, atoi(id+1),JOBID);
					}
					else{
						//call by process ID
						bringFg(jobs, atoi(id),PROCESSID);
					}
				}
				else if (strcmp(temp->cmd,"bg") == 0){
					char *id = temp->args[0];
					if(id == NULL){
						printf("Usage: <bg [%%]id>\n");
						continue;
					}
					if(id[0] == '%'){
						//call by jobID
						sendBg(jobs, atoi(id+1),JOBID);
					}
					else{
						//call by process ID
						sendBg(jobs, atoi(id),PROCESSID);
					}
				}
				continue;
		}
		runJob(parsedCmd);
		
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

