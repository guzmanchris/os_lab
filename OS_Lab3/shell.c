#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define TOKEN_DELIMETERS " \t\r\n\a"
#define DEFAULT_SIZE 10

char *history[DEFAULT_SIZE];
char *argv[DEFAULT_SIZE];
int hist_count = 0;
int hist_num = 0;
int position = 0;
short isHistNum = 0;

void read_line(char *line){
    if(isHistNum){
        line = memcpy(line,history[hist_num-1],DEFAULT_SIZE);
        printf("\n");
    }
    else{
        gets(line);
    }
    isHistNum = 0;
    memcpy(history[hist_count],line,DEFAULT_SIZE);
    hist_count++;
}

void parse_line(char *line,char *argv[]){
    char *token;
    position = 0;
    token = strtok(line,TOKEN_DELIMETERS);
    while(token!=NULL){
        argv[position] = token;
        position++;
        token =strtok(NULL,TOKEN_DELIMETERS);
    }
}

int nat_history(char *argv[]){
    if(position==2){
        hist_num = atoi(argv[1]);
        for(int k = 1; k<=hist_count;k++){
            if(hist_num == k){
                isHistNum = 1;
            }
        }
    }
    if(isHistNum==0){
        for(int i =0; i <hist_count;i++)
            printf(" %d %s\n",(i+1),history[i]);
    }
    return 1;
}

void execute(char *argv[]){

    /* Check if command is valid as a native command */
    char help[10];  strcpy(help, "help");
    char ext[10];	strcpy(ext, "exit");
    char cd[10];	strcpy(cd, "cd");
    char hist[10];	strcpy(hist, "history");

    if(strcmp(argv[0], help) == 0) {  /* Help Command */
        printf("To use this CLI simply write a command and press enter.\n"
               "The following native commands can be used:\n"
               "\thelp: displays this help message\n"
               "\texit: closes the CLI\n"
               "\tcd: executes the cd (change directory) command\n"
               "\thistory: without parameters, the history command shows a list of all executed commands\n"
               "Alternatively, you can execute any command found in the /bin directory.\n"
               "To view the list of commands run: ls /bin\n");
    }
    else if(strcmp(argv[0], ext) == 0) {  /* Exit Command */
        exit(0);
    }
    else if(strcmp(argv[0], cd) == 0) {  /* Change Directory Command */
        if (chdir(argv[1]) < 0) printf("ERROR: cd operation failed.\n");
    }
    else if(strcmp(argv[0], hist) == 0){  /* History Command */
        nat_history(argv);
    }
    else {  /* Non-Native Commands */

        /* Fork a child process to execute the command */
        pid_t pid = fork();

        if (pid < 0) printf("ERROR: Failed to fork main process\n");
        else if (pid == 0) {  /* Child process executes command */
            char path[DEFAULT_SIZE] = "/bin/";
            strcat(path, argv[0]);
            execv(path, argv);
        }
        else if (pid > 0) {  /* Parent process waits for child to terminate before continuing */
            int status;
            pid_t wait_return = wait(&status);
            if (wait_return < 0) printf("ERROR: Wait function returned with an error\n");
        }
    }
}

int main(int argc, char *argv[]){
    int valid = 0;
    char *line = (char*)malloc(DEFAULT_SIZE);
    for(int i = 0;i<DEFAULT_SIZE;i++)
        history[i] = (char*)malloc(DEFAULT_SIZE);
    long size;
    char *buf;
    char *ptr;
    while(1){
        printf("Shell->");
        read_line(line);
        parse_line(line,argv);
        execute(argv);
        for(int j = 0; j< DEFAULT_SIZE;j++){
            argv[j] = NULL;
        }
    }
}