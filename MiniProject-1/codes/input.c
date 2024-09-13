#include "input.h"
#include "hop.h"
#include "log.h"
#include "seek.h"
#include "proclore.h"
#include "commands.h"
#include "reveal.h"
#include "pipes.h"
#include <stdio.h>
#include <string.h>
#include<stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <ctype.h>

int check_semicolon(char* comm)
{
    char *ptr = strchr(comm, ';');

    if (ptr != NULL) 
    {
        return 1;
    } 
    else 
    {
        return 0;
    }
}

int count_ampersands(const char* str) 
{
    int count = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '&') {
            count++;
        }
    }
    return count;
}

int check_and(char* comm)
{
    char *ptr = strchr(comm, '&');

    if (ptr != NULL) 
    {
        return 1;
    } 
    else 
    {
        return 0;
    }
}

int check_token_path(char* token)
{
    if(token[0]=='-' && (token[1]=='a' || token[1]=='l'))
        return 1;
    return 0;
}

void search_a(char* token, int* ptr)
{
    for(int i=0;i<strlen(token);i++)
    {
        if(token[i]=='a')
        {
            *ptr=1;
            return;
        }
    }
}

void search_l(char* token, int* ptr)
{
    for(int i=0;i<strlen(token);i++)
    {
        if(token[i]=='l')
        {
            *ptr=1;
            return;
        }
    }
}

void execute_command(char* comm, char* cwd, char* pwd, int* ptr, char* dummy1)
{
    int ampersand = check_and(comm);
    int semicolon = check_semicolon(comm);
    char* result = strstr(comm, "|");
    if(result!=NULL)
    {
        handle_pipes(comm, cwd, pwd, ptr, dummy1);
        return;
    }
    if(ampersand && semicolon)
    {
        char** array=(char**)malloc(sizeof(char*)*100);
        for(int i=0;i<100;i++)
        {
            array[i]=(char*)malloc(sizeof(char)*4096);
        }
        char dummy_array[4096];
        strcpy(dummy_array, comm);
        char* token3=strtok(dummy_array, ";");
        int i=0;
        while(token3!=NULL)
        {
            int j=0, k=0;
            for(int j=0;j<strlen(token3);j++)
            {
                if ((token3[j]==' ') && (k==0)) continue;
                else{
                    array[i][k]=token3[j];
                    k+=1;
                }
            }
            array[i][k]='\0';
            i++;
            token3 = strtok(NULL, ";");
        }
        array[i] = NULL;
        for(int l=0;l<i;l++)
        {
            printf("command %s\n", array[l]);
            char dummy5[4096];
            strcpy(dummy5, array[l]);
            char* result = strstr(dummy5, "&");
            if(result!=NULL)
            {
                execute_command(array[l], cwd, pwd, ptr, dummy1);
            }
            else
            {
                run_foreground(array[l], cwd, pwd, ptr, dummy1);
            }
        }
    }
    if(ampersand && !semicolon)
    {
        char command[4096];
        char** array=(char**)malloc(sizeof(char*)*100);
        for(int i=0;i<100;i++)
        {
            array[i]=(char*)malloc(sizeof(char)*4096);
        }
        char dummy_array[4096];
        strcpy(dummy_array, comm);
        char* token3=strtok(dummy_array, "&");
        int cmd_count=0;
        while(token3!=NULL)
        {
            int j=0, k=0;
            for(j=0;j<strlen(token3);j++)
            {
                if ((token3[j]==' ') && (k==0)) continue;
                else{
                    array[cmd_count][k]=token3[j];
                    k+=1;
                }
            }
            array[cmd_count][k]='\0';
            cmd_count++;
            token3 = strtok(NULL, "&");
        }
        array[cmd_count] = NULL;
        
        int count_and=count_ampersands(comm);
        if(cmd_count==count_and)
        {
            for(int temp=0;temp<cmd_count;temp++)
            {
                run_background(array[temp], store_background, &idx, cwd, ptr );
            }
        }
        else
        {
            int temp;
            for(temp=0;temp<(cmd_count-1);temp++)
            {
                run_background(array[temp], store_background, &idx , cwd, ptr);
            }
            run_foreground(array[temp], cwd, pwd, ptr, dummy1);
        }
    }
    if(!ampersand && semicolon)
    {
        char** array=(char**)malloc(sizeof(char*)*100);
        for(int i=0;i<100;i++)
        {
            array[i]=(char*)malloc(sizeof(char)*4096);
        }
        char dummy_array[4096];
        strcpy(dummy_array, comm);
        char* token3=strtok(dummy_array, ";");
        int i=0;
        while(token3!=NULL)
        {
            int j=0, k=0;
            for(int j=0;j<strlen(token3);j++)
            {
                if ((token3[j]==' ') && (k==0)) continue;
                else{
                    array[i][k]=token3[j];
                    k+=1;
                }
            }
            array[i][k]='\0';
            i++;
            token3 = strtok(NULL, ";");
        }
        array[i] = NULL;
        for(int l=0;l<i;l++)
        {
            run_foreground(array[l],cwd, pwd, ptr, dummy1);
        }
        return;
    }
    if(!ampersand && !semicolon)
    {
        run_foreground(comm, cwd, pwd, ptr, dummy1);
    }
}