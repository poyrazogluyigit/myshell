#pragma once
int spawn (char* program, char** arg_list, int wait, int pipe, char *fileName);
void argv(char** argvec, char** line, int numArgs);
int execute(char** line, int* meta);
int findInPath(char *fullPath, char *command);