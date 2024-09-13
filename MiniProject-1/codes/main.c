#include "input.h"
#include "log.h"
#include "commands.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <fcntl.h>  
#include <signal.h>
#include <sys/stat.h>
#include <termios.h>
#include <sys/wait.h>

int store_background[4096] = {0};
char background_commands[256][4096];
int store_foreground[4096] = {0};
char foreground_commands[256][4096];
int status_foreground[256] = {0};
int status_background[256]={0};
int foreground_running = 0;
int back=0;
int idx=0;
int idx2=0; // for background
int idx3=0;
int idx4=0;
char dummy2[4096]={0};
char cwd[1024] = {0};
int* ptr1;

void sigint_handler()
{
    if(foreground_running!=0)
    {
        kill(foreground_running, SIGINT);
        foreground_running=0;
    }
    return;
}

void sigtstp_handler(int sig) 
{
    if(foreground_running==0)
    {
        printf("\n\e[36mNo foreground process to stop.\e[0m\n");
        return;
    }
    if(foreground_running!=0)
    {
        int pid = foreground_running;
        int i;
        for(i=0;i<idx2;i++)
        {
            if(store_background[i]==pid)
                break;
        }
        if(i==idx2)
        {
            
            store_background[idx2]=pid;
            status_background[idx2]=2;
            int j;
            for(j=0;j<idx3;j++)
            {
                if(store_foreground[j]==pid)
                    break;
            }
            if(j<idx3)
            {
                store_foreground[j]=-1;
                strcpy(background_commands[idx2], foreground_commands[j]);
                idx2++;
            }
            return;
        }
        pid_t fg_pid = tcgetpgrp(foreground_running);
        tcsetpgrp(STDIN_FILENO, fg_pid);
    }
}

int main()
{
    char command[4096];
    int i=0;
    char ch;
     int log_fd;
    int no_of_lines = 0;
    log_fd = open("log.txt", O_RDWR | O_APPEND | O_CREAT, 0644);
    if (log_fd == -1) {
        perror("open");
        return 1;
    }

    close(log_fd);
    getcwd(cwd, sizeof(cwd));
    char pwd[1024];
    strcpy(pwd, cwd);
    int num=0;
    ptr1 =&num;

    struct sigaction sa_tstp;
    struct sigaction sa_chld;
    sigset_t mask;
    signal(SIGINT, sigint_handler);

    
    sa_tstp.sa_handler = sigtstp_handler;

    sigemptyset(&mask);  

    sa_tstp.sa_mask = mask;  

    sa_tstp.sa_flags = 0;

    sigaction(SIGTSTP, &sa_tstp, NULL);

    signal(SIGCHLD, check_print_status);
    while(1)
    {
        if (feof(stdin)) 
        {
            killpg(getpgrp(), SIGTERM);
            exit(0);
        }
        char cwd1[1024];
        getcwd(cwd1, sizeof(cwd1));
        char* username = getenv("USER");
        struct utsname detect;char pwd[1024];
        uname(&detect);
        printf("<\033[1;32m%s@%s\033[0m:", username, detect.sysname);
        if(*ptr1!=1)
        {
            if (strncmp(cwd1, cwd, strlen(cwd)) == 0) 
            {
                const char* relative_path = cwd1 + strlen(cwd);
                if (strlen(relative_path) == 0 || strcmp(relative_path, "/") == 0)
                {
                    printf("\033[1;34m~>\033[0m");  // Blue color for "~>"
                }
                else
                {
                    printf("\033[1;34m~%s>\033[0m", relative_path);  // Blue color for "~relative_path>"
                }
            }
            else 
            {
                printf("\033[1;34m%s>\033[0m", cwd1);  // Blue color for "cwd1>"
            }

        }
        else 
        {
            if (strncmp(cwd1, cwd, strlen(cwd)) == 0) 
            {
                const char* relative_path = cwd1 + strlen(cwd);
                if (strlen(relative_path) == 0 || strcmp(relative_path, "/") == 0)
                {
                    printf("\033[1;34m~>\033[0m %s>", dummy2);
                    *ptr1=0;
                }
                else
                {
                    printf("\033[1;34m~%s>\033[0m %s>", relative_path, dummy2);
                    *ptr1=0;
                }
            }
            else 
            {
                printf("\033[1;34m%s>\033[0m %s>", cwd1, dummy2);
                *ptr1=0;
            }
        }
        fgets(command, 4096, stdin);
        command[strcspn(command, "\n")] = 0; 
        int flag=0;
        if(strcmp(command, "exit")==0)
        {
            flag=1;
            command[0]='\0';
            break;
        }
        int temp=0;
        char cmd1[4096];
        if (flag==0 && command[0]!='\0')
        {
            strcpy(cmd1, command);
            store_log(command, cwd);
            execute_command(cmd1, cwd, pwd, ptr1, dummy2);
            command[0]='\0';
        }
        command[0]='\0';
    }
    return 0;
}