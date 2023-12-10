#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "token.h"
#include <stddef.h>

const char *symbols[] = {"&", ">>", ">", ">>>"};

void tokenize(char** tokens, char* line){

    const char *newline = "\n";
    const char *delimiter = " ";
    const char *quote = "\"";
    char *token;

    char *string = strdup(line);

    char *ready = strtok(string, newline);

    token = strtok(ready, delimiter);
    int i = 0;
    while(token != NULL){
        if (token[0] == *quote){
            //there is a quote
            char *concat = strdup(token);
            while (token[strlen(token) - 1] != '\"'){
                //concatenate each token until the end is another quote
                token = strtok(NULL, delimiter);
                if (token != NULL) {
                    concat = realloc(concat, strlen(concat) + strlen(token) + 1);
                    strcat(concat, " ");
                    strcat(concat, token);
                }
            }
            char *trimmed = strdup(concat);
            strncpy(trimmed, (concat + 1), strlen(concat) - 2);
            trimmed[strlen(concat) - 2] = '\000';
            tokens[i] = trimmed;
            i++;
            token = strtok(NULL, delimiter);
        }
        else{
            tokens[i] = strdup(token);
            token = strtok(NULL, delimiter);
            i++;
        }
    }
}

void evaluate(int* data, char** string){
    //find the number of tokens
    int size = 0;
    int pipe = 0;
    int pipeBool = 0;
    int background = 0;
    while (1)
    {
        char *tok = string[size];
        if (tok == NULL) break;
        size++;
    }
    //evaluate last element to see if it is amp
    if (strcmp(string[size-1], (const char*) symbols[0]) == 0) background = 1;
    //evaluate either second to last or third to last element,
    if (size-2-background > 0){
        for (int i = 1; i < 4; i++){
        int result = strcmp(*(string+size-2-background), (const char*) symbols[i]);
        if (result == 0){
            pipe = i;
            pipeBool = 1;
            }
        }
    }
    data[0] = background;
    data[1] = pipe;
    data[2] = (size-1) - background - 2*pipeBool;
}

//int main(){
//    char line[1000];
//    fgets(line, sizeof(line), stdin);
//    char *tokens[1000];
//    tokenize(tokens, line);
//    int data[3];
//    evaluate(data, tokens);
//    for (int i = 0; i < 3; i++){
//        printf("%d", data[i]);
//    }
//}
