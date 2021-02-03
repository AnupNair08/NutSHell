#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<ctype.h>
#include "shell.h"

void printStack(stack *st){
    node *t = st->s;
    while(t){
        puts(t->data);
        t = t->next;
    }
    return;
}
void push(stack *st,char *cmd){
    node *temp = (node *) malloc(sizeof(node));
    temp->data = cmd;
    temp->next = NULL;
    if(st->s == NULL){
        st->s = temp;
        st->top = temp;
        return;
    }
    st->top->next = temp;
    st->top = temp;
    return;
}

char *pop(stack *st){
    printStack(st);
    if(st->top == NULL){
        puts("No history stored");
        return NULL;
    } 
    char *temp = (char *)malloc(strlen(st->top->data)); 
    strcpy(temp,st->top->data);
    node *freenode = st->top;
    node *p = st->s;
    node *q = NULL;
    while(p != freenode){
        q = p;
        p = p->next;
    }
    st->top = q;
    if(q) q->next = NULL;
    free(freenode); 
    puts(temp);
    return temp;
}
stack* stackInit(){
    stack *st = (stack *) malloc(sizeof(stack));
    int fd = open(".sh_hist", O_RDONLY);
    if(fd == -1){
        return st;
    }
    char buf[1024];
    int wordSize = read(fd,buf,1024);
    char *tok = (char *)malloc(MAX_SIZE);
    tok = strtok(buf,"\n");
    push(st,tok);
    while(tok){
        tok = strtok(NULL,"\n");
        if(tok == NULL || !isascii(tok[0])){
            break;
        }
        // puts(tok);
        push(st,tok);
    }
    return st;
}


// int main(){
//     stack *s = stackInit();
//     while(1){
//         char * k = pop(s);
//         if(k){
//             puts(k);
//         }
//         else{
//             break;
//         }
//     }
//     // printStack(s);
//     return 0;
// }