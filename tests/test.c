#include<stdio.h>
#include<signal.h>


void handlestop(){
    printf("F");
}

int main(){
    signal(SIGTSTP,handlestop);
    while(1){

    };
    return 0;
}