#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
int main(){
	int o,n;
	o = open("f1.txt", O_RDONLY);
	n = open("f2.txt", O_TRUNC | O_CREAT | O_RDWR, 0666);
	
	
	// file > prog 
	// Set the file as the stdin
	dup2(o,0);
	execl("/usr/bin/sort", "/usr/bin/sort",NULL);
	// prog > file
	//We copy the file descriptor to the stdout
	dup2(n,1);
	//Now stdout is the file that we wish to redirect op tp
	execl("/bin/ls","/bin/ls",NULL);




	return 0;
}
