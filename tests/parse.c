#include<stdio.h>
#include<string.h>

char *cmdlist[12];

void parse(char *cmd, int start, int end){
	static int j = 0;
	int index = start;
	printf("%d %d\n", start, end);
	for(int i = start ; i < end; i++) {
		if(cmd[i] == '|' || cmd[i] == '>' | cmd[i] == '<' ){
			printf("%c\n",cmd[i]);
			index = i;
			break;
		}
	}
	if(index != start) {
		parse(cmd, start, index-1);
		parse(cmd,index+1,end);
	}
	else{
		int k = 0;
		char part[128];
		for(int i = start ; i < end ; i++){
			part[k++] = cmd[i];
		}
		printf("\n");
		return;
	}

}


int main(){
	char *str = "a | b > f";
	parse(str,0,strlen(str)  + 1);
	for(int i = 0 ; i < 3 ; i++){
		puts(cmdlist[i]);
	}
}
