#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>
#include<sys/wait.h>
#include<stdlib.h>
int main(){

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
			execlp("cat","cat","f1.txt",NULL);
		}
		//parent of child 2 i.e child 1 executes prog2
		else{
			close(pidfd[1]);
			dup2(pidfd[0],0);
			close(pidfd[0]);
			execlp("sort","sort",NULL);
			wait(NULL);
		}
	}
	if(pid){
		close(pidfd[0]);
		close(pidfd[1]);
		wait(NULL);
	}
	return 0;
}

/*p -> child */
  /*-> child*/
