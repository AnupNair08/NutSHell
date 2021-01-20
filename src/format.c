#include<stdio.h>

void red () {
	  printf("\033[1;31m");
	  return;
}

void reset(){
	printf("\033[0m");
	return;
}

void blue(){
	printf("\x1B[34m");
	return;
}
