#include "pipes.h"
#include "input.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <ctype.h>
#include <unistd.h>
#include<sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>

#define READ 0
#define WRITE 1
#define MAX_LINE 1024

int check_invalid(char* str)
{
    int i = 0;
    int pipe_found = 0;

    while (str[i] != '\0') {
        if (str[i] == '|') {
            if (pipe_found) 
            {
                return 1;
            }
            pipe_found = 1;  
        } 
        else if (!isspace(str[i])) 
        {
            pipe_found = 0;
        }
        i++;
    }
    return 0;
}

void trim_spaces(char *str) {
    char *end;
    while (*str == ' ') str++;
    end = str + strlen(str) - 1;
    while (end > str && *end == ' ') end--;
    *(end + 1) = '\0';
}


void handle_pipes(char* command, char* cwd, char* pwd, int* ptr, char* dummy1)
{
    size_t len=strlen(command);
    char** commands = (char**)malloc(sizeof(char*)*256);
    for(int i=0;i<256;i++)
    {
        commands[i] = (char*)malloc(sizeof(char)*1024);
    }
    for (int i = 0; i < 256; i++) {
        commands[i] = (char*)malloc(sizeof(char) * 1024);
        if (!commands[i]) {
            perror("malloc failed");
            return;
        }
    }
    if(command[0]=='|')
    {
        printf("Invalid use of pipe\n");
        foreground_running=0;
        return;
    }
    if(command[len-1]=='|')
    {
        printf("Invalid use of pipe\n");
        foreground_running=0;
        return;
    }
    int temp= check_invalid(command);
    if(temp)
    {
        printf("Invalid use of pipe\n");
        foreground_running=0;
        return;
    }
    char* token = strtok(command, "|");   
    int i=0;
    while(token!=NULL)
    {
        strcpy(commands[i], token);
        i++;
        token = strtok(NULL, "|");
    }
    int num_commands = i;
    commands[i] = NULL;
     int pipefd[2];
    int prev_pipe_read = -1;  

    for (int i = 0; i < num_commands; i++) 
    {
        if (i < num_commands - 1 && pipe(pipefd) == -1) {
            perror("pipe failed");
            foreground_running=0;
            return;
        }

        pid_t pid = fork();
        if (pid == -1) {
            perror("fork failed");
            foreground_running=0;
            return;
        }

        if (pid == 0) 
        { 
            if (prev_pipe_read != -1) {
                dup2(prev_pipe_read, STDIN_FILENO);
                close(prev_pipe_read);
            }
            if (i < num_commands - 1) {
                dup2(pipefd[1], STDOUT_FILENO);  
                close(pipefd[1]);
            }
            close(pipefd[0]);
            execute_command(commands[i], cwd, pwd, ptr, dummy1);
            exit(0);
        }
        if (prev_pipe_read != -1) {
            close(prev_pipe_read);
        }
        close(pipefd[1]);
        if (i < num_commands - 1) {
            prev_pipe_read = pipefd[0]; 
        }
    }
    for (int i = 0; i < num_commands; i++) {
        wait(NULL);
    }
    for (int i = 0; i < 256; i++) {
        free(commands[i]);
    }
    free(commands);

    return;
}