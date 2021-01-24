#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<signal.h>
#include<stdlib.h>
int main(){
    char *term = (char *)malloc(128);
    ctermid(term);
    puts(term);
    int fd = open(term,O_RDONLY);
    printf("%d", tcgetpgrp(fd));
    tcsetpgrp(fd,218620);
    signal(SIGINT, SIG_IGN);
    printf("%d", tcgetpgrp(fd));

    // while(1){

    // }
    return 0;
}