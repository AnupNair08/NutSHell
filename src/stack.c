#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include "shell.h"
stack* stackInit(){
    stack *st = (stack *) malloc(sizeof(stack));
    st->top = 0;
    return st;
}

stack *makeSpace(stack *st){
    printf("called");
    for(int i = 0 ; i < 63 ;i++){
        // st->s[i] = st->s[i + 63];
        strcpy(st->s[i],st->s[i+63]);
    }
    st->top = 63;
    return st;
}

void push(stack *st,char *cmd){
    if(st->top > 63) {
        st = makeSpace(st);
    }
    strcpy(st->s[st->top++],cmd);
    return;
}

char *pop(stack *st){
    if(st->top == 0 || st->s[st->top - 1] == NULL){
        return NULL;
    }
    return st->s[--st->top];
}

// int main(){
//     stack *s = stackInit();
//     push(s,"ls");
//     push(s,"pwd");
//     push(s,"pwd");
//     push(s,"pwd");

//     puts(pop(s));
//     puts(pop(s));

//     return 0;
// }