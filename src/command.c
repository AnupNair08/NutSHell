#include<stdio.h>
#include"shell.h"
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

/// @brief Array to store all the parsed commands.
command commandList[64];
int commandSize;

/**
 * @brief Array to store all the special operators.
 */
char spcOps[32];
int spcSize;


/// @brief Utility function to print the command
/// @param c Pointer to command that is to be printed
void printCommand(command *c){
    printf("Command : %s\n",c->cmd);
    printf("Arguments: ");
    for(int i = 0 ; i < c->size; i++){
        printf("%s ",c->args[i]);
    }
    printf("\nBuilt in command ? : %d\n", c->isBuiltin);
    puts("\n-------------------------");
}

char *removeQt(char *s){
    char *temp = (char *)malloc(strlen(s));
    int k = 0;
    for(int i = 0 ; i < strlen(s) ; i++){
        if(s[i] == '\"'){
            continue;
        }
        temp[k++] = s[i];
    }
    return temp;
}


/// @brief Function to parse a single command with arguments
/// @param cmd String that is to be parsed
/// @return Pointer to a parsed command 
command *parseOne(char *cmd){
    command *c = (command *)malloc(sizeof(command));
    c->isBackground = 0;
    c->args = (char **)malloc(sizeof(char *));
	char *tok = (char *)malloc(sizeof(char) * 128);
    tok = strtok(cmd," ");
    c->cmd = tok;
    if(strcmp(c->cmd,"exit") == 0 || strcmp(c->cmd, "cd") == 0 || strcmp(c->cmd, "help") == 0 || strcmp(c->cmd,"jobs") == 0 || strcmp(c->cmd,"fg") == 0 || strcmp(c->cmd,"bg") == 0 || strcmp(c->cmd,"history") == 0){
        c->isBuiltin = 1;
    }
    else{
        c->isBuiltin = 0;
    }

    int i = 0;
    int isQt = 0;
    while(tok){
        tok = strtok(NULL, " ");
        if(tok == NULL){
            break;
        }
        if(strcmp(tok,"&") == 0){
            c->isBackground = 1;
            continue;
        }
        if(tok[strlen(tok) - 1] == '\"' && tok[0] == '\"'){
            c->args[i] = (char *)malloc(sizeof(char) * 128);
            c->args[i] = removeQt(tok);
            isQt = 0;
            i++;
        }
        else if(tok[strlen(tok) - 1] == '\"'){
            // c->args[i] = strcat(c->args[i],removeQt(tok));
            sprintf(c->args[i], "%s %s", c->args[i], removeQt(tok));
            isQt = 0;
            i++;
        }
        else if(tok[0] == '\"'){
            c->args[i] = (char *)malloc(sizeof(char) * 128);
            c->args[i] = removeQt(tok);
            isQt = 1;
        }
        else if(isQt == 1){
            // c->args[i] = strcat(c->args[i],tok);
            sprintf(c->args[i], "%s %s", c->args[i], removeQt(tok));
        }
        else{
            c->args[i] = (char *)malloc(sizeof(char) * 128);
            c->args[i] = tok;
            i++;
        }
        // puts(tok);
    }
    c->size = i;
	commandList[commandSize++] = *c;
    return c;
}

/// @brief Function to parse commands based on tokens
/// @param cmd User input on commandline
/// @param start Starting index of the string
/// @param end Ending index of the string
/// @return Tokens based on special operators
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
		char *left = parse(cmd, start, index);
		char *right = parse(cmd,index+1,end);
	}
	else{
		int k = 0;
		char *part = (char *)malloc(128);
		for(int i = start ; i < end ; i++){
			part[k++] = cmd[i];
		}
        if(strlen(part) > 0){
            parseOne(part);
        }
        return part;
	}

}

/**
 * @brief Function to print the array of parsed commands
 * 
 */
void printParsed(){
    for(int i = 0 ; i < commandSize ; i++){
        printCommand(&commandList[i]);
        if(i > 0){
            printf("%c\n",spcOps[i - 1]);
        }
    }
    return;
}

void interpret(command *c, int csize, char *s, int ssize){
    char files[128][128];
    int k = 0;
    for(int i = 0 ; i < ssize ;i++){
        if(s[i] == '>' || s[i] == '<'){
            strcpy(files[k++],c[i+1].cmd);
            puts(files[k-1]);
        }
    }
    return;
}

/**
 * @brief Generates an array of commands and special characters 
 * 
 * @param cmd String of command
 * @return Array of parsed commands alongwith special operators 
 */
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



int main(){
    char whitespace[] = " \t\r\n\v";
    char symbols[] = "<|>&";
    
    char cmd[] = "ls | grep < f.txt > g.txt";
    parse(cmd,0,strlen(cmd) + 1);

    interpret(commandList,commandSize, spcOps, spcSize);
    
    // int fd;
    // for(int i = 0, j = 1 ; i < spcSize ; i++, j++){
    //         switch(spcOps[i]){
    //             case '|' :
    //                 printCommand(&commandList[j-1]);
    //                 printCommand(&commandList[j]);
    //                 break;
    //             case '>' :
    //                 fd = open(commandList[j].cmd, O_CREAT | O_RDWR);
    //                 dup2(fd,1);
    //                 execlp(commandList[j-1].cmd,commandList[j-1].cmd,NULL);
    //                 close(fd);
    //                 break;
    //             case '<' :
    //                 fd = open(commandList[j].cmd, O_RDONLY);
    //                 if(fd == -1){
    //                     perror("Redirection failed: ");
    //                 }
    //                 dup2(fd,0);
    //                 execlp(commandList[j-1].cmd,commandList[j-1].cmd,NULL);
    //                 close(fd);
    //                 break;
    //         }
    //     }
    return 0;
}