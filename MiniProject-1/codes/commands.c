#include "commands.h"
#include "seek.h"
#include "reveal.h"
#include "hop.h"
#include "input.h"
#include "proclore.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h> 
void check_print_status()
{
    int stat ;
    int pid = waitpid(-1, &stat, WNOHANG);
    if(pid>0)
    {
        int i;
        for(i=0;i<idx;i++)
        {
            if(store_background[i]==pid)
                break;
        }
        if(i==idx)
        {
            return;
        }
        if (WIFEXITED(stat)) 
        {
            printf("\033[1;31mProcess exited normally with status %d\033[0m\n", WEXITSTATUS(stat));
        } 
        else if (WIFSIGNALED(stat)) 
        {
            printf("\033[1;31mProcess terminated by signal %d\033[0m\n", WTERMSIG(stat));

        } 
        else 
        {
            printf("\033[1;31mProcess did not exit normally\033[0m\n");
        }
        fflush(stdout);
    }
    return;
}
void run_background(char* command, int* store_background, int* idx)
{

    char** args = (char**)malloc(sizeof(char*)*100);
    for(int i=0;i<100;i++)
    {
        args[i]=(char*)malloc(sizeof(char)*4096);
    }
    char dummy[4096];
    strcpy(dummy, command);
    char* duplicate=strtok(dummy, " ");
    int i=0;
    while(duplicate!=NULL)
    {
        strcpy(args[i], duplicate);
        i++;
        duplicate = strtok(NULL, " ");
    }
    args[i] = NULL;
    if(!(strcmp(args[0], "hop")!=0 && strcmp(args[0], "log")!=0 && strcmp(args[0], "reveal")!=0 && strcmp(args[0], "proclore")!=0 && strcmp(args[0], "seek")!=0))
    {
        printf("\033[1;31mErronous command\033[0m\n"); 
        return;
    }
    int status=0;
    int initial_time=time(NULL);
    int child = fork();
    if(child<0)
    {
        printf("\033[1;31mfork failed\033[0m\n");
        return;
    }
    else if(child==0)
    {
        execvp(args[0], args);
        printf("\033[1;31mErronous command detected\033[0m\n");
        return;
    }
    else
    {
        printf("%d\n", child);
        store_background[(*idx)++]=child;
        //printf("%d\n", store_background[(*idx)-1]);
        int final_time=time(NULL);
        int diff_time=final_time - initial_time;
        for (int j = 0; j < i; j++) 
        {
            free(args[j]);
        }
        free(args);
    }
    return;
}

void run_foreground(char* command,char* cwd, char* pwd, int* ptr, char* dummy1)
{
    char** args = (char**)malloc(sizeof(char*)*100);
        for(int i=0;i<100;i++)
        {
            args[i]=(char*)malloc(sizeof(char)*4096);
        }
        char dummy[4096];
        strcpy(dummy, command);
        char* duplicate=strtok(dummy, " ");
        int i=0;
        while(duplicate!=NULL)
        {
            strcpy(args[i], duplicate);
            i++;
            duplicate = strtok(NULL, " ");
        }
        args[i] = NULL;
        int status=0;
        int initial_time=time(NULL);
        int child = fork();
        if(child<0)
        {
            printf("\033[1;31mfork failed\033[0m\n");
            return;
        }
        else if(child==0)
        {
            execvp(args[0], args);
            *ptr=2;
            if(strcmp(args[0], "hop")!=0 && strcmp(args[0], "log")!=0 && strcmp(args[0], "reveal")!=0 && strcmp(args[0], "proclore")!=0 && strcmp(args[0], "seek")!=0)
            {
                printf("\033[1;31mErronous command\033[0m\n"); 
                return;
            }
            goto custom;
        }
        else
        {
            waitpid(child, &status, 0);
            int final_time=time(NULL);
            int diff_time=final_time - initial_time;
            if(diff_time>2 && strcmp(args[0], "hop")!=0 && strcmp(args[0], "log")!=0 && strcmp(args[0], "reveal")!=0 && strcmp(args[0], "proclore")!=0 && strcmp(args[0], "seek")!=0)
            {
                *ptr=1;
                strcpy(dummy1, command);
                for (int j = 0; j < i; j++) 
                {
                    free(args[j]);
                }
                return;
            }
            for (int j = 0; j < i; j++) 
            {
                free(args[j]);
            }
            free(args);
        }
        custom:
        char* token = strtok(command, " ");
        if (token != NULL) 
        {
            if(strcmp(token, "hop")==0)
            {
                char* token1 = strtok(NULL, " ");
                if(token1==NULL)
                {
                    execute_only_hop(cwd, pwd);
                }
                char* token2 = strtok(NULL, " ");
                execute_hop(token1, token2, pwd, cwd);
                return;
            }
            else if(strcmp(token, "log")==0)
            {
                char* token1 = strtok(NULL, " ");
                if(token1==NULL)
                {
                    print_log(cwd);
                    return;
                }
                char* token2 = strtok(NULL, " ");
                execute_log(token1, token2, cwd, pwd, ptr, dummy1);
                return;
            }
            else if(strcmp(token, "proclore")==0)
            {
                char* token1 = strtok(NULL, " ");
                execute_proclore(token1);
                return;
            }
            else if(strcmp(token, "reveal")==0)
            {
                char* token1 = strtok(NULL, " ");
                if(token1==NULL)
                {
                    execute_only_reveal();
                    return;
                }
                else if(token1[0]=='-' && (strcmp(token1, "-")>0 || strcmp(token1, "-")<0))
                {
                    int find_a=0;
                    int find_l=0;
                    int* ptr_a=&find_a;
                    int* ptr_l=&find_l;
                    int path=check_token_path(token1);
                    while(!check_token_path(token1))
                    {
                        if(!find_a || !find_l)
                        {
                            if(!find_a)
                            {
                                search_a(token1, ptr_a);
                            }
                            if(!find_l)
                            {
                                search_l(token1, ptr_l);
                            }
                        }
                        token1 = strtok(NULL, " ");
                        if(token1==NULL)
                            break;
                    }
                    if(token1!=NULL)
                    {
                        execute_reveal_flags(token1, find_a, find_l, pwd, cwd);
                        return;
                    }
                    else
                    {
                        char cwd1[4096];
                        getcwd(cwd1, sizeof(cwd1));
                        strcpy(token1, cwd1);
                        execute_reveal_flags(token1, find_a, find_l, pwd, cwd);
                        return;
                    }
                }
                else 
                {
                    execute_reveal(token1, pwd, cwd);
                    return;
                }
            }
            else if(strcmp(token, "seek")==0)
            {
                char* token1 = strtok(NULL, " ");
                int number_of_matches=0;
                int* ptr=&number_of_matches;
                int find_d = 0;
                int find_e=0;
                int find_f=0;
                while(strcmp(token1, "-d")==0 || strcmp(token1, "-e")==0 || strcmp(token1, "-f")==0)
                {
                    fflush(stdout);
                    if(strcmp(token1, "-d")==0)
                        find_d++;
                    if(strcmp(token1, "-e")==0)
                        find_e++;
                    if(strcmp(token1, "-f")==0)
                        find_f++;
                    token1 = strtok(NULL, " ");
                }
                if(!find_d && !find_e && !find_f)
                {
                    char* token2= strtok(NULL, " ");
                    if(token2==NULL)
                    {
                        execute_seek_cwd(token1, ptr);
                        return;
                    }
                    char* token3 = strtok(NULL, " ");
                    if(token3==NULL)
                    {
                        execute_seek_path(token1, token2, pwd, cwd, ptr);
                        return;
                    }
                }
                else
                {
                    char* path= strtok(NULL, " ");
                    execute_seek(path, token1, find_d, find_e, find_f, pwd, cwd, ptr);
                    return;
                }
            }
            return;
        }
        return;
    
}