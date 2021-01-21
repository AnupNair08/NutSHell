#include<stdio.h>
#include"shell.h"
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

/// @brief CommandList Array to store all the parsed commands
command commandList[64];
int commandSize;

char spcOps[32];
int spcSize;

void printCommand(command *c){
    printf("Command : %s\n",c->cmd);
    printf("Arguments: ");
    for(int i = 0 ; i < c->size; i++){
        printf("%s ",c->args[i]);
    }
    puts("\n-------------------------");
}

/// @brief Function to parse a single command with arguments
/// @return Pointer to a command
command *parseOne(char *cmd){
    command *c = (command *)malloc(sizeof(command));
    c->args = (char **)malloc(sizeof(char *));
	char *tok = (char *)malloc(sizeof(char) * 128);
    tok = strtok(cmd," ");
    c->cmd = tok;
    int i = 0;
    while(tok){
        tok = strtok(NULL, " ");
        if(tok == NULL){
            break;
        }
        // puts(tok);
        c->args[i] = (char *)malloc(sizeof(char) * 128);
        c->args[i] = tok;
        i++; 
    }
    c->size = i;
	commandList[commandSize++] = *c;
    return c;
}

/// @brief Function to parse an commands recursively
/// @return Parsed command
char* parse(char *cmd, int start, int end){
    static int j = 0;
	int index = start;
	for(int i = start ; i < end; i++) {
		if(cmd[i] == '|' || cmd[i] == '>' | cmd[i] == '<' ){
			// printf("%c\n",cmd[i]);
            spcOps[spcSize++] = cmd[i];
            index = i;
			break;
		}
	}
	if(index != start) {
		char *left = parse(cmd, start, index-1);
		char *right = parse(cmd,index+1,end);
	}
	else{
		int k = 0;
		char *part = (char *)malloc(128);
		for(int i = start ; i < end ; i++){
			part[k++] = cmd[i];
		}
        parseOne(part);
        return part;
	}

}

void printParsed(){
    for(int i = 0 ; i < commandSize ; i++){
        printCommand(&commandList[i]);
        if(i > 0){
            printf("%c\n",spcOps[i - 1]);
        }
    }
}


cmdList *getParsed(char *cmd){
    cmdList *t = (cmdList *) malloc (sizeof(cmdList)); 
    commandSize = 0;
    spcSize = 0;
    parse(cmd,0,strlen(cmd) + 1);
    t->commandList = commandList;
    t->commandSize = commandSize;
    t->spcOps = spcOps;
    t->spcSize = spcSize;
    return t;
}



int caller(){
    char whitespace[] = " \t\r\n\v";
    char symbols[] = "<|>&";
    
    char cmd[] = "ls -l file xyz | sort <pwd.txt | grep pattern";
    parse(cmd,0,strlen(cmd) + 1);

    
    int fd;
    for(int i = 0, j = 1 ; i < spcSize ; i++, j++){
            switch(spcOps[i]){
                case '|' :
                    printCommand(&commandList[j-1]);
                    printCommand(&commandList[j]);
                    break;
                case '>' :
                    fd = open(commandList[j].cmd, O_CREAT | O_RDWR);
                    dup2(fd,1);
                    execlp(commandList[j-1].cmd,commandList[j-1].cmd,NULL);
                    close(fd);
                    break;
                case '<' :
                    fd = open(commandList[j].cmd, O_RDONLY);
                    if(fd == -1){
                        perror("Redirection failed: ");
                    }
                    dup2(fd,0);
                    execlp(commandList[j-1].cmd,commandList[j-1].cmd,NULL);
                    close(fd);
                    break;
            }
        }
    return 0;
}