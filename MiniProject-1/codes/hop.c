#include "hop.h"
#include "commands.h"
#include<stdio.h>
#include<string.h>
#include <unistd.h>
#include <stdlib.h>

void execute_only_hop(char* pwd, char* home)
{
    char cwd1[1024];
    getcwd(cwd1, sizeof(cwd1));
    if (chdir(home) != 0)
    {
        perror("\033[1;31mchdir to home failed\033[0m");
        foreground_running=0;
        return;
    }
    strcpy(pwd, cwd1);
    return;
}
void execute_hop(char* tok1, char* tok2, char* pwd, char* home)
{
    char cwd1[1024];
    getcwd(cwd1, sizeof(cwd1));
    char cwd[1024];
    int error=0;
    if (getcwd(cwd1, sizeof(cwd1)) == NULL) {
        perror("\033[1;31mgetcwd failed\033[0m");
        error =1;
        foreground_running=0;
        return;
    }
    char *directory_path = NULL;
    if(strcmp(tok1, "~")==0)
    {
        if (chdir(home) != 0) {
            perror("\033[1;31mchdir to home failed\033[0m");
            error=1;
            foreground_running=0;
            return;
        }
        strcpy(pwd, cwd1);
    }
    else if(strcmp(tok1, "-")==0)
    {
        if(pwd[0]=='\0')
        {
            printf("\033[1;31mno previous working directory\033[0m\n");
        }
        else
        {
            if (chdir(pwd) != 0) {
                perror("\033[1;31mchdir to previous directory failed\033[0m");
                error=1;
                foreground_running=0;
                return;
            }
            strcpy(pwd, cwd1);
        }        
    }
    else if(tok1[0]=='.' && tok1[1]=='.')
    {
        if(chdir("..")!=0)
        {
            perror("\033[1;31mchdir to previous directory failed\033[0m");
            foreground_running=0;
            return;
        }
        strcpy(pwd, cwd1);
        getcwd(cwd, sizeof(cwd));
        printf("%s\n", cwd);
        if(tok1[2]=='/' && tok1[3]=='.' && tok1[4]=='.')
        {
            strcpy(pwd, cwd1);
            chdir("..");
            getcwd(cwd, sizeof(cwd));
            if(tok1[5]=='/')
            {
                char path[1024];
                int i=6;
                int j=0;
                while(i<strlen(tok1))
                {
                    path[j++]=tok1[i++];
                }
                path[j]='\0';
                chdir(path);
            }
        }
        chdir(tok2);
    }
    else if(tok1[0]=='~')
    {
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s%s",home , tok1 + 1);
        if (chdir(full_path) != 0) {
            perror("\033[1;31mchdir to expanded path failed\033[0m");
            foreground_running=0;
            error = 1;
            return;
        }
    }
    else 
    {
        if (chdir(tok1) != 0) {
            perror("\033[1;31mchdir to specified directory failed\033[0m");
            foreground_running=0;
            error=1;
            return;
        }
        strcpy(pwd, cwd1);
    }
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("\033[1;31mgetcwd after chdir failed\033[0m");
        foreground_running=0;
        error=1;
        return;
    }
}