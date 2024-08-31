#include "input.h"
#include "log.h"
#include "commands.h"
#include<stdio.h>
#include<string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <fcntl.h>  
#include <signal.h>
#include <sys/stat.h>

int store_background[4096] = {0};
int idx=0;
char dummy2[4096]={0};

int main()
{
    char command[4096];
    int i=0;
    //idx=0;
    char ch;
     int log_fd;
    int no_of_lines = 0;
    log_fd = open("log.txt", O_RDWR | O_APPEND | O_CREAT, 0644);
    if (log_fd == -1) {
        perror("open");
        return 1;
    }
    close(log_fd);
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    char pwd[1024];
    strcpy(pwd, cwd);
    int num=0;
    int* ptr1 =&num;
    signal(SIGCHLD, check_print_status);
    while(1)
    {
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
        scanf("%[^\n]", command);
        getchar();
        printf("%s\n", command);
        int flag=0;
        if(strcmp(command, "exit")==0)
        {
            flag=1;
        }
        int temp=0;
        if (flag==0){
            char cmd1[4096];
            strcpy(cmd1, command);
            store_log(command, cwd);
            execute_command(cmd1, cwd, pwd, ptr1, dummy2);
        }
        else if (flag==1){
            temp=1;
            break;
        }
        if (temp==1) break;

    }
    return 0;
}