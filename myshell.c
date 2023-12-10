#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stddef.h>
#include "eval.h"
#include "execution.h"
#include <fcntl.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <signal.h>

#define MAX_COMMAND_LENGTH 1000
#define MAX_ARGUMENTS 10

#define READ 0
#define WRITE 1

char line[MAX_COMMAND_LENGTH];
const char exitLine[] = "exit\n";
char previous[MAX_COMMAND_LENGTH];


int alias(){

    int state = 0;

    char *aliasStr = line;

    char fileLine[1000];
    FILE *aliases = fopen(".aliases", "a");
    FILE *temp = fopen(".t1", "w");

    char *inputTok[1000];
    tokenize(inputTok, aliasStr);

    int found = 0;
    while(fgets(fileLine, sizeof(fileLine), aliases) != NULL){
        char *fileTok[1000];
        tokenize(fileTok, fileLine);
        char *aliasName = fileTok[2];
        //alias is found
        if (strcmp(aliasName, inputTok[2]) == 0){
            found = 1;
            fprintf(temp, "%s", aliasStr);
        }
        else{
            fprintf(temp, "%s", fileLine);
        }
    }
    if (!found){
        fprintf(temp, "%s", aliasStr);
    }
    state = fclose(aliases);
    state = fclose(temp);
    remove(".aliases");
    rename(".t1", ".aliases");
    return state;
}

int searchAlias(char** new, char** lineTokens){
    int found = 0;
    FILE *aliases = fopen(".aliases", "r");
    char fileLine[1000];
    while(fgets(fileLine, sizeof(fileLine), aliases) != NULL){
        char *fileTok[1000];
        tokenize(fileTok, fileLine);
        char *aliasName = fileTok[1];
        //there is a corresponding alias
        if (strcmp(aliasName, lineTokens[0]) == 0){
            found = 1;
            int i = 0;
            char *aliasTok[1000];
            tokenize(aliasTok, fileTok[3]);
            while (aliasTok[i] != NULL){
                new[i] = aliasTok[i]; i++;
            }
            int j = 0;
            while(lineTokens[j + 1] != NULL){
                new[j+i] = lineTokens[j+1]; j++;
            }
            break;
        }
    }
    fclose(aliases);
    return found;
}


int getps(){
    pid_t current = getpid();
    char * call[6] = {"ps", "--ppid", "asd", "-o", "pid", NULL};
    char pid[7];
    snprintf(pid, 7, "%d", current);
    call[2] = pid;
    spawn(call[0], call, 0, 2, ".proc");
    FILE *file = fopen(".proc", "r");
    int count = -1;
    char ch;
    while (!feof(file)) {
        ch = fgetc(file);
        if (ch == '\n') {
            count++;
        }
    }
    fclose(file);
    remove(".proc");
    return count;
}



//handling pretty printing of shell
void pprint(){
    char *username = getenv("USER");
    char *machine = getenv("NAME");
    char *wd = getenv("PWD");
    fprintf(stderr, "%s@%s %s --- ", username, machine, wd);
}

int bello(){

    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    char timebuf[80];
    strftime(timebuf, 80, "%a %b %d %G %T", timeinfo);
    
    int processes = getps();

    char *username = getenv("USER");
    char *tty = ttyname(STDIN_FILENO);
    char *shell = getenv("SHELL");
    char *home = getenv("HOME");
    char *machine = getenv("NAME");
    printf("%s\n%s\n%s\n%s\n%s\n%s\n%s\n%d\n", username, machine, previous, tty, shell, home, timebuf, processes);
    return 0;
}

void sig_handle(int signum){
    printf("\n");
    pprint();
}

void sig_handle2(int signum){
    printf("\n");
}


//backspace handling
void processString(char *str){
    char *src, *dst;
    for(src = dst = str; *src != '\0'; src++){
        if (*src == '\b'){
            if (dst != str){
                dst--;
            }
            continue;
        }

        *dst++ = *src;
    }
    *dst = '\0';
}




int main(){

    // proc = fopen(".proc", "w");

    while(1){

        signal(SIGINT, sig_handle);

        pprint();
        waitpid(-1, NULL, 0);

        char *tokens[1000];
        for(int i = 0; i < 1000; i++){
            tokens[i] = NULL;
        }
        int data[3];
        for(int i = 0; i < 3; i++){
            data[i] = NULL;
        }
        //termination conditions 
        fgets(line, sizeof(line), stdin);

        processString(line);

        signal(SIGINT, sig_handle2);
        if (strcmp(line, exitLine) == 0 || line==NULL) break;


        //retrieve the tokens from the line
        tokenize(tokens, line);

        if (tokens[0] == NULL) continue;

        char *new[1000];
        for(int i = 0; i < 1000; i++){
            new[i] = NULL;
        }
        int flag = 0;
        flag = searchAlias(new, tokens);
        if (flag){
            evaluate(data, new);
            execute(new, data);
        }

        //retrieve information about the line
        else{
            evaluate(data, tokens);
            execute(tokens, data);
        }
        if (data[1] == 3){
            //read from .temp, append to fileName in reverse, delete .temp
            int numArgs = data[2];
            char* fileName = flag == 0? tokens[numArgs + 2] : new[numArgs + 2];
            int file = open(fileName, O_WRONLY | O_APPEND | O_CREAT, 0777);
            int fd = open(".temp", O_RDONLY, 0777);
            //move file pointer to beginning of file
            int end = lseek(fd, 0, SEEK_END);
            lseek(fd, 0, SEEK_SET);

            char* buffer = malloc(end*sizeof(char));
            read(fd, buffer, end);
            for (off_t i = end - 1; i >= 0; i--) {
                write(file, &buffer[i], 1);
            }
            remove(".temp");
            free(buffer);
        }
        char* command = strtok(line, "\n");
        strncpy(previous, command, MAX_COMMAND_LENGTH);
    }
}