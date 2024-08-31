#include "input.h"
#include "hop.h"
#include "log.h"
#include "seek.h"
#include "proclore.h"
#include "commands.h"
#include "reveal.h"
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
        return 0;
    return 1;
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
    // int* store_background = (int*)malloc(sizeof(int)*4096);
    int ampersand = check_and(comm);
    int semicolon = check_semicolon(comm);
    if(ampersand && semicolon)
    {
        char** array=(char**)malloc(sizeof(char*)*100);
        for(int i=0;i<100;i++)
        {
            array[i]=(char*)malloc(sizeof(char)*4096);
            //array[i]=NULL;
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
            //printf("%s\n", array[i]);
            i++;
            token3 = strtok(NULL, ";");
        }
        array[i] = NULL;
        for(int l=0;l<i;l++)
        {
            char dummy5[4096];
            strcpy(dummy5, array[l]);
            char* result = strstr(dummy5, "&");
            if(result!=NULL)
            {
                char command[4096];
                char** sub_array=(char**)malloc(sizeof(char*)*100);
                for(int i=0;i<100;i++)
                {
                    sub_array[i]=(char*)malloc(sizeof(char)*4096);
                }
                char dummy_array2[4096];
                strcpy(dummy_array2, array[l]);
                char* token3=strtok(dummy_array2, "&");
                int cmd_count=0;
                //printf("%s\n", token3);
                while(1)
                {
                    if(token3==NULL)
                    {
                        break;
                   }
                    int j=0, k=0;
                    for(int j=0;j<strlen(token3);j++)
                    {
                        if ((token3[j]==' ') && (k==0)) continue;
                        else
                        {
                            sub_array[cmd_count][k]=token3[j];
                            k+=1;
                        }
                    }
                    sub_array[cmd_count][k]='\0';
                    //printf("%s\n", array[i]);
                    cmd_count++;
                    token3 = strtok(NULL, "&");
                }
                sub_array[cmd_count] = NULL;
                int count_and=count_ampersands(sub_array[l]);
                printf("%d\n", count_and);
                if(cmd_count==count_and)
                {
                    for(int temp=0;temp<cmd_count;temp++)
                    {
                        //printf("%s\n", array[temp]);
                        printf("hii");
                        run_background(array[temp], store_background, &idx );
                    }
                }
                else
                {
                    int  temp;
                    for(temp=0;temp<(cmd_count-1);temp++)
                    {
                        //printf("%s\n", array[temp]);
                        run_background(array[temp], store_background, &idx );
                    }
                    run_foreground(array[temp], cwd, pwd, ptr, dummy1);
                }
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
            //printf("%s\n", array[j]);
            cmd_count++;
            token3 = strtok(NULL, "&");
        }
        array[cmd_count] = NULL;
        
        int count_and=count_ampersands(comm);
        //printf("%d\n", cmd_count);
        if(cmd_count==count_and)
        {
            for(int temp=0;temp<cmd_count;temp++)
            {
                //printf("%s\n", array[temp]);
                run_background(array[temp], store_background, &idx );
            }
        }
        else
        {
            int temp;
            for(temp=0;temp<(cmd_count-1);temp++)
            {
                //printf("%s\n", array[temp]);
                run_background(array[temp], store_background, &idx );
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
            char** args = (char**)malloc(sizeof(char*)*100);
            for(int i=0;i<100;i++)
            {
                args[i]=(char*)malloc(sizeof(char)*4096);
            }
            char dummy[4096];
            strcpy(dummy, array[l]);
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
                printf("fork failed\n");
                return;
            }
            else if(child==0)
            {
                execvp(args[0], args);
                *ptr=2;
                if(strcmp(args[0], "hop")!=0 && strcmp(args[0], "log")!=0 && strcmp(args[0], "reveal")!=0 && strcmp(args[0], "proclore")!=0 && strcmp(args[0], "seek")!=0)
                {
                    printf("Erronous command detected\n"); 
                    continue;   
                }
                goto custom1;
            }
            else
            {
                waitpid(child, &status, 0);
                int final_time=time(NULL);
                int diff_time=final_time - initial_time;
                if(diff_time>2 && strcmp(args[0], "hop")!=0 && strcmp(args[0], "log")!=0 && strcmp(args[0], "reveal")!=0 && strcmp(args[0], "proclore")!=0 && strcmp(args[0], "seek")!=0)
                {
                    *ptr=1;
                    strcpy(dummy1, array[l]);
                    for (int j = 0; j < i; j++) 
                    {
                        free(args[j]);
                    }
                    continue;
                }
                for (int j = 0; j < i; j++) 
                {
                    free(args[j]);
                }
            }
            custom1:
            char* token = strtok(array[l], " ");
            if (token != NULL) 
            {
                //printf("hii\n");
                if(strcmp(token, "hop")==0)
                {
                    char* token1 = strtok(NULL, " ");
                    if(token1==NULL)
                    {
                        execute_only_hop(cwd, pwd);
                        continue;
                    }
                    char* token2 = strtok(NULL, " ");
                    execute_hop(token1, token2, pwd, cwd);
                    continue;
                }
                else if(strcmp(token, "log")==0)
                {
                    char* token1 = strtok(NULL, " ");
                    if(token1==NULL)
                    {
                        print_log(cwd);
                        continue;
                    }
                    char* token2 = strtok(NULL, " ");
                    execute_log(token1, token2, cwd, pwd, ptr, dummy1);
                    continue;
                }
                else if(strcmp(token, "proclore")==0)
                {
                    char* token1 = strtok(NULL, " ");
                    execute_proclore(token1);
                    continue;
                }
                else if(strcmp(token, "reveal")==0)
                {
                    char* token1 = strtok(NULL, " ");
                    if(token1==NULL)
                    {
                        execute_only_reveal();
                        continue;
                    }
                    else if(token1[0]=='-' && (strcmp(token1, "-")>0 || strcmp(token1, "-")<0))
                    {
                        //printf("hii");
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
                            continue;
                        }
                        else
                        {
                            char cwd1[4096];
                            getcwd(cwd1, sizeof(cwd1));
                            strcpy(token1, cwd1);
                            //printf("%s\n", token1);
                            execute_reveal_flags(token1, find_a, find_l, pwd, cwd);
                            continue;
                        }
                    }
                    else 
                    {
                        //printf("hiii\n");
                        execute_reveal(token1, pwd, cwd);
                        continue;
                    }
                    // execute_reveal(token1, token2);
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
                        // printf("%s\n", token1);
                        //fflush(stdout);
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
                            continue;
                        }
                        char* token3 = strtok(NULL, " ");
                        if(token3==NULL)
                        {
                            execute_seek_path(token1, token2, pwd, cwd, ptr);
                            continue;
                        }
                    }
                    else
                    {
                        char* path= strtok(NULL, " ");
                        execute_seek(path, token1, find_d, find_e, find_f, pwd, cwd, ptr);
                        continue;
                    }
                }
            }
        }
        return;
    }
    if(!ampersand && !semicolon)
    {
        char** args = (char**)malloc(sizeof(char*)*100);
        for(int i=0;i<100;i++)
        {
            args[i]=(char*)malloc(sizeof(char)*4096);
        }
        char dummy[4096];
        strcpy(dummy, comm);
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
            printf("fork failed\n");
            return;
        }
        else if(child==0)
        {
            execvp(args[0], args);
            *ptr=2;
            if(strcmp(args[0], "hop")!=0 && strcmp(args[0], "log")!=0 && strcmp(args[0], "reveal")!=0 && strcmp(args[0], "proclore")!=0 && strcmp(args[0], "seek")!=0)
            {
                printf("Erronous command\n"); 
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
                strcpy(dummy1, comm);
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
        }
        custom:
        char* token = strtok(comm, " ");
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
}