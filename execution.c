#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "myshell.h"
#include <signal.h>

extern int procNum;
extern FILE *proc;

int findInPath(char *fullPath, char *command)
{
  char *PATH = getenv("PATH");
  char *pathCpy = strdup(PATH);
  char *dir = strtok(pathCpy, ":");

  while (dir != NULL)
  {
    snprintf(fullPath, 2000, "%s/%s", dir, command);
    if (access(fullPath, F_OK) != -1)
      return 0;
    dir = strtok(NULL, ":");
  }
  snprintf(fullPath, 2000, "%s", command);
  if (access(fullPath, F_OK) != -1)
    return 0;

  return -1;
}

int builtin_alias_check(char *program)
{
  if (strcmp(program, "bello") == 0)
  {
    return 1;
  }
  else if (strcmp(program, "alias") == 0)
  {
    return 2;
  }
  else return 0;
}

void pipe_handle(int pipe, char* fileName){
      int file;
      if (pipe != 0){
        if (pipe == 2){
        file = open(fileName, O_WRONLY | O_CREAT, 0777);
      }
      else if (pipe == 1){
        file = open(fileName, O_WRONLY | O_APPEND | O_CREAT, 0777);

      }
      else{
        file = open(".temp", O_WRONLY | O_CREAT, 0777);

      }
      dup2(file, STDOUT_FILENO);
      close(file);
      }
}

int spawn(char *program, char **arg_list, int wait, int pipe, char *fileName)
{
  pid_t child_pid;
  int check = builtin_alias_check(program);
  int saved_stdout = dup(1);
  if (check == 1){
    if (pipe) pipe_handle(pipe, fileName);
    int b = bello(); 
    dup2(saved_stdout, STDOUT_FILENO);
    return b;
  }
  else if (check==2){
    if (pipe) pipe_handle(pipe, fileName);
    int a = alias();
    dup2(saved_stdout, STDOUT_FILENO);
    return a;
  }

  /* Duplicate this process.  */
  child_pid = fork();
  if (child_pid != 0)
  {
    /* This is the parent process.  */
    waitpid(child_pid, 0, wait);
    // if the child is a background process, print pid
    if (wait)
    {
      printf("[%d]\n", child_pid);
    }
    return child_pid;
  }

  else
  {
    // if output is piped, open a file
    signal(SIGINT, SIG_DFL);
    if (pipe)
    {
      int file;
      if (pipe == 2)
        file = open(fileName, O_WRONLY | O_CREAT, 0777);
      else if (pipe == 1)
        file = open(fileName, O_WRONLY | O_APPEND | O_CREAT, 0777);
      else
        file = open(".temp", O_WRONLY | O_CREAT, 0777);
      dup2(file, STDOUT_FILENO);
      close(file);
    }
      char fullPath[1000];
      if (findInPath(fullPath, program))
      {
        fprintf(stderr, "myshell: command %s not found\n", program);
        return -1;
      }
      /* Now execute PROGRAM, searching for it in the path.  */
      int r = execvp(program, arg_list);
      /* The execvp function returns only if an error occurs.  */
      fprintf(stderr, "%d, an error occurred in execvp\n", r);
      abort();
  }
}

void argv(char **argvec, char **line, int numArgs)
{
  for (int i = 0; i < numArgs + 1; i++)
  {
    argvec[i] = line[i];
  }
  argvec[numArgs + 1] = NULL;
}

void execute(char **tokens, int *meta)
{
  int background = meta[0];
  int pipe = meta[1];
  int numArgs = meta[2];
  char *fileName = tokens[numArgs + 2];

  char *argvec[numArgs + 2];
  argv(argvec, tokens, numArgs);
  spawn(argvec[0], argvec, background, pipe, fileName);

  // additional stuff required for background processing etc.
}
// int main(){
//
//     char string[100];
//     fgets(string, sizeof(string), stdin);
//
//
//     char **ls = NULL;
//     tokens(&ls, string);
//     int* test = convert(ls);
//
//     char ** vector = (char**) malloc(sizeof(char**));
//     argv(&vector, ls, test[2]);
//
//     free(test);
// }