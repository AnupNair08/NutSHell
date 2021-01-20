#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include"utility.h"


doubleStack *init(){
	doubleStack *d = (doubleStack *)malloc(sizeof(doubleStack));
	d->s1 = (stack *)malloc(sizeof(stack));
	d->s2 = (stack *)malloc(sizeof(stack));
	d->s1->size = 0;
	d->s2->size = 0;
	d->s1->head = NULL, d->s1->top = NULL;
	d->s2->head = NULL, d->s2->top = NULL;
	return d;
}

void push(stack *s, char *val){
	if(s->size == 0){
		stackNode *temp = (stackNode *)malloc(sizeof(stackNode));
		temp->comd = val;
		temp->next = NULL;
		s->head = temp;
		s->top = temp;
		s->size = 1;
		/*puts(val);*/
		return;
	}
	stackNode *temp = (stackNode *)malloc(sizeof(stackNode));
	temp->comd = val;
	temp->next = NULL;
	s->top->next = temp;
	s->top = temp;
	s->size++;
	/*puts(val);*/
	return;
}

char *pop(stack *s){
	if(s->size == 0) {
		return NULL;
	}
	char *temp = (char *)malloc(128);
	temp = s->top->comd;
	stackNode *p = s->head;
	for(int i = 1 ; i < s->size - 1; i++){
		//puts(p->comd);
		p = p->next;
	}
	s->size--;
	stackNode *t = s->top;
	s->top = p;
	//free(t);
	return temp;
}

doubleStack *readHistory(){
	doubleStack *history = init();
	char *data = (char *)malloc(sizeof(char) * 2048);
	int h = open(".sh_hist", O_RDONLY);
	read(h,data,2048);
	char *tok = (char *)malloc(128);
	tok = strtok(data,"\n" );
	while(tok){
		push(history->s1, tok);
		/*puts(tok);*/
		tok = strtok(NULL, "\n");
		
	}
	return history;
}

char* handleArrowUp(doubleStack *history){
	char *c	= pop(history->s1);
	if(c == NULL){
		return NULL;
	}
	push(history->s2,c);
	return c;
}
char* handleArrowDown(doubleStack *history){
	char *c	= pop(history->s2);
	if(c == NULL ){
		return NULL;
	}
	push(history->s1,c);
	return c;
}

void printHelp(){
	printf("Author: Anup N\n");
}

/*int main(){*/
	/*doubleStack *history = readHistory();*/
	/*puts(handleArrowUp(history));*/
	/*puts(handleArrowUp(history));*/
	/*puts(handleArrowDown(history));*/
	/*puts(handleArrowDown(history));*/
/*}*/
