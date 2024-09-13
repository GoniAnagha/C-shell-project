#include "commands.h"
#include "seek.h"
#include "reveal.h"
#include "hop.h"
#include "input.h"
#include "proclore.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <wait.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <fcntl.h>  
#include <errno.h>
#include <termios.h>
#include<sys/ptrace.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT 80
#define BUFFER_SIZE 4096

void fetch_manpage(char* command)
{
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char *hostname = "man.he.net";
    char request[256];
    sprintf(request, "GET /man1/%s HTTP/1.1\r\n"
                 "Host: man.he.net\r\n"
                 "User-Agent: CClient/1.0\r\n"
                 "Connection: close\r\n"
                 "\r\n", command);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    {
        perror("ERROR opening socket");
        return;
    }
    server = gethostbyname(hostname);
    if (server == NULL) 
    {
        fprintf(stderr, "ERROR, no such host\n");
        return;
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr_list[0], 
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(PORT);
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
    {
        perror("ERROR connecting");
        return;
    }
    if (send(sockfd, request, strlen(request), 0) < 0) 
    {
        perror("ERROR sending request");
        return;
    }
    char buffer[BUFFER_SIZE];
    int bytes_received;
    int in_body = 0;
    int skip = 1;
    while ((bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0)) > 0) 
    {
        for (int i = 0; i < bytes_received; i++) 
        {
            if (skip) 
            {
                if (buffer[i] == '\n' && buffer[i-1] == '\n') 
                {
                    skip = 0;
                }
            } 
            else 
            {
                putchar(buffer[i]);
            }
        }
    }
    if (bytes_received < 0) 
    {
        perror("ERROR receiving response");
    }
    close(sockfd);
    return;
}

void check_print_status()
{
    int stat ;
    int pid = waitpid(-1, &stat, WNOHANG);
    if(pid>0)
    {
        int i;
        for(i=0;i<idx2;i++)
        {
            if(store_background[i]==pid)
                break;
        }
        if (WIFEXITED(stat)) 
        {
            status_background[i]=-2;  // -2 => exited normally
            printf("\033[1;31mProcess exited normally with status %d\033[0m\n", WEXITSTATUS(stat));
        } 
        else if (WIFSIGNALED(stat)) 
        {
            status_background[i]=1; // terminated
            printf("\033[1;31mProcess terminated by signal %d\033[0m\n", WTERMSIG(stat));

        } 
        else if(WIFSTOPPED(stat))
        {
            int sig = WSTOPSIG(stat);
            if (sig == SIGSTOP) {
                printf("\033[1;31mProcess stopped by SIGSTOP signal\033[0m\n");
            } 
            else if (sig == SIGTSTP) {
                printf("\033[1;31mProcess stopped by SIGTSTP signal\033[0m\n");
            } 
            else {
                printf("\033[1;31mProcess stopped by unknown signal %d\033[0m\n", sig);
            }
            status_background[i] = 2; // stopped
        }
        else 
        {
            status_background[i]=-1;
            printf("\033[1;31mProcess did not exit normally\033[0m\n");
        }
        fflush(stdout);
    }
    return;
}

int bring_to_foreground(pid_t pid) 
{
    if (tcsetpgrp(STDIN_FILENO, pid) == -1) {
        perror("Failed to set foreground process group");
        return -1;
    }
     if (kill(pid, SIGCONT) == -1) {
        perror("Failed to send SIGCONT");
        return -1;
    }
    return 0;
}

void execute_activities()
{
    for(int i=0;i<idx2;i++)
    {   
        if(status_background[i]==2 && store_background[i]!=-1)
        {
            printf("%d : %s - ", store_background[i], background_commands[i]);
            printf("stopped\n");
        }
        else if(status_background[i]!=-2 && status_background[i]!=1 && status_background[i]!=-1 && store_background[i]!=-1)
        {
            printf("%d : %s - ", store_background[i], background_commands[i]);
            printf("Running\n");
        }
    }
    return;
}

void run_background(char* command, int* store_background, int* idx, char* cwd, int* ptr1)
{
    back=1;
    char dummy[4096];
    strcpy(dummy, command);
    char* result1 = strstr(command, ">");
    char* result2 = strstr(command, ">>");
    char* result3 = strstr(command, "<");
    if(result1==NULL && result2==NULL && result3==NULL)
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
            back=0;
            return;
        }
        int status=0;
        int initial_time=time(NULL);
        int child = fork();
        if(child<0)
        {
            printf("\033[1;31mfork failed\033[0m\n");
            back=0;
            return;
        }
        else if(child==0)
        {
            if(back)
            {
                setpgid(0, 0);
            }
            execvp(args[0], args);
            printf("\033[1;31mErronous command detected\033[0m\n");
            back=0;
            idx2--;
            exit(0);
        }
        else
        {
            printf("%d\n", child);
            store_background[(idx2)]=child;
            strcpy(background_commands[(idx2)],command); 
            idx2++;
            int final_time=time(NULL);
            int diff_time=final_time - initial_time;
            for (int j = 0; j < i; j++) 
            {
                free(args[j]);
            }
            free(args);
        }
    }
    else if(result2!=NULL && result3==NULL)
    {
        char *token_red = strtok(dummy, ">>");
        char token_dup[4096];
        strcpy(token_dup, token_red);
        if (token_red == NULL) {
            printf("Error: no command found.\n");
            back=0;
            return ;
        }
        char *redirect_file = strtok(NULL, ">> ");
        if (redirect_file == NULL) {
            printf("Error: no file found after redirection.\n");
            back=0;
            return ;
        }
        char current_dir[4096];
        getcwd(current_dir, sizeof(current_dir));
        char full_path[4097];
        if(redirect_file[0]!='/')
        {
            snprintf(full_path, sizeof(full_path), "%s/%s", current_dir, redirect_file);
        }
        else
        {
            strcpy(full_path, redirect_file);
        }
        int fd = open(full_path,  O_CREAT | O_APPEND | O_WRONLY, 0644);
        if (fd == -1) 
        {
            printf("No such input file found\n");
            back=0;
            return;
        }
        int std_out_dup=dup(1);
        dup2(fd, 1);
        close(fd);
        run_background(token_dup, store_background,idx, cwd, ptr1);
        fflush(stdout);
        dup2(std_out_dup, 1);
        close(std_out_dup);
    }
    else if(result1!=NULL && result3==NULL && result2==NULL)
    {
        char* token_red = strtok(dummy, ">");
        char token_red_dup[4096];
        strcpy(token_red_dup, token_red);
        char* redirect_file = strtok(NULL, " ");
        char current_dir[4096];
        getcwd(current_dir, sizeof(current_dir));
        char full_path[4097];
        if(redirect_file[0]!='/')
        {
            snprintf(full_path, sizeof(full_path), "%s/%s", current_dir, redirect_file);
        }
        else
        {
            strcpy(full_path, redirect_file);
        }
        int fd = open(full_path, O_CREAT | O_TRUNC | O_WRONLY, 0644);
        if (fd == -1) 
        {
            printf("No such input file found\n");
            back=0;
            return;
        }
        int std_out_dup=dup(1);
        close(1);
        dup2(fd, 1);
        close(fd);
        run_background(token_red_dup, store_background, idx, cwd, ptr1);
        fflush(stdout);
        dup2(std_out_dup, 1);
        close(std_out_dup);
    }
    else if(result3!=NULL && result1==NULL && result3==NULL)
    {
        char* token_red = strtok(dummy, "<");
        char token_red_dup[4096];
        strcpy(token_red_dup, token_red);
        char* redirect_file = strtok(NULL, " ");
        char current_dir[4096];
        getcwd(current_dir, sizeof(current_dir));
        char full_path[4097];
        if(redirect_file[0]!='/')
        {
            snprintf(full_path, sizeof(full_path), "%s/%s", current_dir, redirect_file);
        }
        else
        {
            strcpy(full_path, redirect_file);
        }
        int fd = open(full_path,  O_RDONLY, 0644);
        if (fd == -1) 
        {
            printf("No such input file found\n");
            back=0;
            return;
        }
        int std_in_dup=dup(0);
        int y=dup2(fd, 0);
        close(fd);
        run_background(token_red_dup, store_background, idx, cwd, ptr1);
        clearerr(stdin);
        dup2(std_in_dup, 0);
        close(std_in_dup);
    }
     else 
    {
        if(result1!=NULL && result2==NULL)
        {
            char dupi[4096];
            strcpy(dupi, command);
            char* token = strtok(dupi, " \t<");
            char* command_dup = token;
            char* opera = strtok(NULL, " ");
            token = strtok(NULL, " \t>");
            char* input_file = token;
            opera = strtok(NULL, " ");
            token = strtok(NULL, " \t>");
            char* output_file = token;
            char current_dir[4096];
            getcwd(current_dir, sizeof(current_dir));
            char full_path[4097];
            char full_path2[4097];
            if(output_file[0]!='/')
            {
                snprintf(full_path2, sizeof(full_path), "%s/%s", current_dir, output_file);
            }
            else
            {
                strcpy(full_path2, output_file);
            }
            int fd = open(full_path2, O_CREAT | O_TRUNC | O_WRONLY, 0644);
            if (fd == -1) {
                perror("open");
                back=0;
                return ;
            }
            if(input_file[0]!='/')
            {
                snprintf(full_path, sizeof(full_path), "%s/%s", current_dir, input_file);
            }
            else
            {
                strcpy(full_path, input_file);
            }
            int fd2 = open(full_path,  O_RDONLY, 0644);
            if (fd2 == -1) 
            {
                printf("No such input file found\n");
                back=0;
                return;
            }
            int std_out_dup=dup(1);
            dup2(fd, 1);
            close(fd);
            int std_in_dup=dup(0);
            int y=dup2(fd2, 0);
            close(fd2);
            run_background(command_dup, store_background, idx, cwd, ptr1);
            clearerr(stdin);
            dup2(std_in_dup, 0);
            close(std_in_dup);
            fflush(stdout);
            dup2(std_out_dup, 1);
            close(std_out_dup);
        }
        else if (result2!=NULL)
        {
            char dupi[4096];
            strcpy(dupi, command);
            char* token = strtok(dupi, " \t<");
            char* command_dup = token;
            char* opera = strtok(NULL, " ");
            token = strtok(NULL, " \t>>");
            char* input_file = token;
            opera = strtok(NULL, " ");
            token = strtok(NULL, " \t>>");
            char* output_file = token;
            char current_dir[4096];
            getcwd(current_dir, sizeof(current_dir));
            char full_path[4097];
            char full_path2[4097];
            if(output_file[0]!='/')
            {
                snprintf(full_path2, sizeof(full_path), "%s/%s", current_dir, output_file);
            }
            else
            {
                strcpy(full_path2, output_file);
            }
            int fd = open(full_path2, O_CREAT |O_APPEND | O_WRONLY, 0644);
            if (fd == -1) {
                perror("open");
                back=0;
                return ;
            }
            if(input_file[0]!='/')
            {
                snprintf(full_path, sizeof(full_path), "%s/%s", current_dir, input_file);
            }
            else
            {
                strcpy(full_path, input_file);
            }
            int fd2 = open(full_path,  O_RDONLY, 0644);
            if (fd2 == -1) 
            {
                printf("No such input file found\n");
                back=0;
                return;
            }
            int std_out_dup=dup(1);
            dup2(fd, 1);
            close(fd);
            int std_in_dup=dup(0);
            int y=dup2(fd2, 0);
            close(fd2);
            run_background(command_dup, store_background, idx, cwd, ptr1);
            clearerr(stdin);
            dup2(std_in_dup, 0);
            close(std_in_dup);
            fflush(stdout);
            dup2(std_out_dup, 1);
            close(std_out_dup);
        }
    }
    return;
}

void run_foreground(char* command,char* cwd, char* pwd, int* ptr, char* dummy1)
{
    char* result1 = strstr(command, ">");
    char* result2 = strstr(command, ">>");
    char* result3 = strstr(command, "<");
    char dummy[4096];
    strcpy(dummy, command);
    if(result1==NULL && result2==NULL && result3==NULL)
    {
        char** args = (char**)malloc(sizeof(char*)*100);
        for(int i=0;i<100;i++)
        {
            args[i]=(char*)malloc(sizeof(char)*4096);
        }
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
        foreground_running=child;
        if(child<0)
        {
            printf("\033[1;31mfork failed\033[0m\n");
            foreground_running=0;
            return;
        }
        else if(child==0)
        {
            if(strcmp(args[0], "ping")==0)
            {
                goto custom;
            }
            execvp(args[0], args);
            *ptr=2;
            if(strcmp(args[0], "hop")!=0 && strcmp(args[0], "log")!=0 && strcmp(args[0], "reveal")!=0 && strcmp(args[0], "proclore")!=0 && strcmp(args[0], "seek")!=0 && strcmp(args[0], "activities")!=0 && strcmp(args[0], "ping")!=0 && strcmp(args[0], "fg")!=0 && strcmp(args[0], "bg")!=0 && strcmp(args[0], "home")!=0 && strcmp(args[0], "reveall")!=0 && strcmp(args[0], "reveala")!=0 && strcmp(args[0], "mk_hop")!=0 && strcmp(args[0], "hop_seek")!=0 && strcmp(args[0], "iMan")!=0)
            {
                printf("\033[1;31mErronous command\033[0m\n"); 
                foreground_running=0;
                return;
            }
            goto custom;
        }
        else
        {
            foreground_running=child;
            status_foreground[idx3] = status;
            store_foreground[idx3]=child;
            strcpy(foreground_commands[idx3++], command);
            waitpid(child, &status, 0);
            foreground_running=0;
            int final_time=time(NULL);
            int diff_time=final_time - initial_time;
            if(diff_time>2 && strcmp(args[0], "hop")!=0 && strcmp(args[0], "log")!=0 && strcmp(args[0], "reveal")!=0 && strcmp(args[0], "proclore")!=0 && strcmp(args[0], "seek")!=0 && strcmp(args[0], "activities")!=0 && strcmp(args[0], "ping")!=0 && strcmp(args[0], "fg")!=0 && strcmp(args[0], "bg")!=0 && strcmp(args[0], "home")!=0 && strcmp(args[0], "reveall")!=0 && strcmp(args[0], "reveala")!=0 && strcmp(args[0], "mk_hop")!=0 && strcmp(args[0], "hop_seek")!=0 && strcmp(args[0], "iMan")!=0)
            {
                *ptr=1;
                strcpy(dummy1, command);
                for (int j = 0; j < i; j++) 
                {
                    free(args[j]);
                }
                free(args);
                return;
            }
            for (int j = 0; j < i; j++) 
            {
                free(args[j]);
            }
            free(args);
        }
        custom:
        char duplicate_dup[4096];
        strcpy(duplicate_dup, command);
        char* token = strtok(duplicate_dup, " ");
        if (token != NULL) 
        { 
            if(strcmp(token, "home")==0)
            {
                execute_hop("~", NULL, pwd, cwd);
            }
            else if(strcmp(token, "reveall")==0)
            {
                char cwd1[4096];
                getcwd(cwd1, sizeof(cwd1));
                execute_reveal_flags(cwd1, 0, 1, pwd, cwd);
            }
            else if(strcmp(token, "reveala")==0)
            {
                char cwd1[4096];
                getcwd(cwd1, sizeof(cwd1));
                execute_reveal_flags(cwd1, 1, 0, pwd, cwd);
            }
            else if(strcmp(token, "mk_hop")==0)
            {
                char* file = strtok(NULL, " ");
                printf("%s\n", file);
                int status = mkdir(file, S_IRWXU | S_IRWXG | S_IRWXO);
                if (status == -1) 
                {
                    perror("mkdir");
                    foreground_running=0;
                    return;
                }
                execute_hop(file, NULL, pwd, cwd);
            }
            else if(strcmp(token, "hop_seek")==0)
            {
                char* file = strtok(NULL, " ");
                execute_hop(file, NULL, pwd, cwd);
                char cwd2[4096];
                getcwd(cwd2, sizeof(cwd2));
                execute_seek_cwd(file, ptr);
            }
            else if(strcmp(token, "iMan")==0)
            {
                char* man_page=strtok(NULL, " ");
                fetch_manpage(man_page);
                return;
            }
            else if(strcmp(token, "hop")==0)
            {
                char* token1 = strtok(NULL, " ");
                if(token1==NULL)
                {
                    execute_hop("~", NULL, pwd, cwd);
                    foreground_running=0;
                    return;
                }
                char* token2 = strtok(NULL, " ");
                execute_hop(token1, token2, pwd, cwd);
                foreground_running=0;
                return;
            }
            else if(strcmp(token, "log")==0)
            {
                char* token1 = strtok(NULL, " ");
                if(token1==NULL)
                {
                    print_log(cwd);
                    foreground_running=0;
                    return;
                }
                char* token2 = strtok(NULL, " ");
                execute_log(token1, token2, cwd, pwd, ptr, dummy1);
                foreground_running=0;
                return;
            }
            else if(strcmp(token, "proclore")==0)
            {
                char* token1 = strtok(NULL, " ");
                execute_proclore(token1);
                foreground_running=0;
                return;
            }
            else if (strcmp(token, "reveal") == 0) 
            {
                char* token1 = strtok(NULL, " ");
                if (token1 == NULL) 
                {
                    execute_only_reveal();
                    foreground_running=0;
                    return;
                }
                int find_a = 0;
                int find_l = 0;
                while (token1 != NULL) 
                {
                    if (token1[0] == '-' && strcmp(token1, "-")!=0) 
                    {
                        for (int i = 1; i < strlen(token1); i++) 
                        {
                            if (token1[i] == 'a') 
                            {
                                find_a = 1;
                            } 
                            else if (token1[i] == 'l') 
                            {
                                find_l = 1;
                            }
                        }
                    } 
                    else 
                    {
                        if (find_a || find_l) 
                        {
                            execute_reveal_flags(token1, find_a, find_l, pwd, cwd);
                            foreground_running=0;
                            return;
                        } 
                        else 
                        {
                            execute_reveal(token1, pwd, cwd);
                            foreground_running=0;
                            return;
                        }
                    }
                    token1 = strtok(NULL, " ");
                }
                if (find_a || find_l) 
                {
                    char cwd1[4096];
                    getcwd(cwd1, sizeof(cwd1));
                    execute_reveal_flags(cwd1, find_a, find_l, pwd, cwd);
                    foreground_running=0;
                    return;
                } 
                else 
                {
                    execute_reveal(pwd, pwd, cwd);
                    foreground_running=0;
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
                        foreground_running=0;
                        return;
                    }
                    char* token3 = strtok(NULL, " ");
                    if(token3==NULL)
                    {
                        execute_seek_path(token1, token2, pwd, cwd, ptr);
                        foreground_running=0;
                        return;
                    }
                }
                else
                {
                    char* path= strtok(NULL, " ");
                    execute_seek(path, token1, find_d, find_e, find_f, pwd, cwd, ptr);
                    foreground_running=0;
                    return;
                }
            }
            else if(strcmp(token, "activities")==0)
            {
                execute_activities();
                foreground_running=0;
                return;
            }
            else if(strcmp(token, "ping")==0)
            {
                char* pid_p = strtok(NULL, " ");
                char* sig_num_p = strtok(NULL, " ");
                if(sig_num_p==NULL)
                {
                    printf("Erronuous command\n");
                    return;
                }
                pid_t pid = atoi(pid_p);
                int sig_num = atoi (sig_num_p);
                int final_signum = sig_num;
                if(sig_num>=32)
                {
                    final_signum = sig_num % 32;
                }
                int sent = kill(pid, final_signum);
                if(sent==0)
                {
                    printf("Sent signal %d to process with pid %d\n", final_signum, pid);
                    int check;
                    if(final_signum==19 || final_signum==20)
                    {
                        for(check=0;check<idx2;check++)
                        {
                            if(store_background[check]==pid)
                                break;
                        }
                        status_background[check]=2;
                    }
                }
                else
                {
                    printf("Failed to send signal\n");
                }
            }
            else if(strcmp(token, "fg")==0)
            {
                char* pid_p = strtok(NULL, " ");
                int num = 0;
                for (int i = 0; pid_p[i] != '\0'; i++) {
                    num = num * 10 + (pid_p[i] - '0');
                }
                int temp_fg;
                if (tcsetpgrp(STDIN_FILENO, num)<0) printf("here");
                signal(SIGTTIN, SIG_IGN);
                signal(SIGTTOU, SIG_IGN);
                //tcsetpgrp(std)
                if(kill(num, SIGCONT)<0)
                {
                    printf("error");
                }
                int status_p;
                waitpid(num, &status_p, WUNTRACED);
                
                for(temp_fg=0;temp_fg<idx2;temp_fg++)
                {
                    if(store_background[temp_fg]==num)
                    {
                        store_foreground[idx3]=num;
                        strcpy(foreground_commands[idx3],background_commands[temp_fg]);
                        idx3++;
                        store_background[temp_fg]=-1;
                        strcpy(background_commands[temp_fg], "Moved to foreground");
                        break;
                    }
                }
                tcsetpgrp(STDIN_FILENO, getpgrp());
                signal(SIGTTIN, SIG_DFL);
                signal(SIGTTOU, SIG_DFL);
            }
            else if(strcmp(token, "bg")==0)
            {
                char* pid_p = strtok(NULL, " ");
                pid_t pid = atoi(pid_p);
                int temp_fg;
                if (kill(pid, SIGCONT) == -1) 
                {
                    if (errno == ESRCH) 
                    {
                        printf("No such process found\n");
                    }
                    else 
                    {
                        perror("Error resuming process");
                    }
                }
                for(temp_fg=0;temp_fg<idx3;temp_fg++)
                {
                    if(store_foreground[temp_fg]==pid)
                    {
                        break;
                    }
                }
                status_foreground[temp_fg]=0;
            }
            foreground_running=0;
            return;
        }
        return;
    }
    else if(result2!=NULL && result3==NULL)
    {
        char *token_red = strtok(dummy, ">>");
        char token_dup[4096];
        strcpy(token_dup, token_red);
        if (token_red == NULL) {
            printf("Error: no command found.\n");
            return ;
        }
        char *redirect_file = strtok(NULL, ">> ");
        if (redirect_file == NULL) {
            printf("Error: no file found after redirection.\n");
            return ;
        }
        char current_dir[4096];
        getcwd(current_dir, sizeof(current_dir));
        char full_path[4097];
        if(redirect_file[0]!='/')
        {
            snprintf(full_path, sizeof(full_path), "%s/%s", current_dir, redirect_file);
        }
        else
        {
            strcpy(full_path, redirect_file);
        }
        int fd = open(full_path,  O_CREAT | O_APPEND | O_WRONLY, 0644);
        if (fd == -1) 
        {
            printf("No such input file found\n");
            return;
        }
        int std_out_dup=dup(1);
        dup2(fd, 1);
        close(fd);
        char** args = (char**)malloc(sizeof(char*)*100);
        for(int i=0;i<100;i++)
        {
            args[i]=(char*)malloc(sizeof(char)*4096);
        }
        char* duplicate=strtok(token_red, " ");
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
            fflush(stdout);
            dup2(std_out_dup, 1);
            close(std_out_dup);
            printf("\033[1;31mfork failed\033[0m\n");
            return;
        }
        else if(child==0)
        {
            execvp(args[0], args);
            *ptr=2;
            if(strcmp(args[0], "hop")!=0 && strcmp(args[0], "log")!=0 && strcmp(args[0], "reveal")!=0 && strcmp(args[0], "proclore")!=0 && strcmp(args[0], "seek")!=0 && strcmp(args[0], "activities")!=0 && strcmp(args[0], "ping")!=0 && strcmp(args[0], "fg")!=0 && strcmp(args[0], "home")!=0 && strcmp(args[0], "reveall")!=0 && strcmp(args[0], "reveala")!=0 && strcmp(args[0], "mk_hop")!=0 && strcmp(args[0], "hop_seek")!=0 && strcmp(args[0], "iMan")!=0)
            {
                fflush(stdout);
                dup2(std_out_dup, 1);
                close(std_out_dup);
                printf("\033[1;31mErronous command\033[0m\n"); 
                return;
            }
            run_foreground(token_dup, cwd, pwd, ptr, dummy1);
            fflush(stdout);
            dup2(std_out_dup, 1);
            close(std_out_dup);
        }
        else
        {
            waitpid(child, &status, 0);
            store_foreground[(idx3)]=child;
            strcpy(foreground_commands[idx3++], command);
            int final_time=time(NULL);
            int diff_time=final_time - initial_time;
            if(diff_time>2 && strcmp(args[0], "hop")!=0 && strcmp(args[0], "log")!=0 && strcmp(args[0], "reveal")!=0 && strcmp(args[0], "proclore")!=0 && strcmp(args[0], "seek")!=0 && strcmp(args[0], "activities")!=0 && strcmp(args[0], "ping")!=0 && strcmp(args[0], "fg")!=0 && strcmp(args[0], "home")!=0 && strcmp(args[0], "reveall")!=0 && strcmp(args[0], "reveala")!=0 && strcmp(args[0], "mk_hop")!=0 && strcmp(args[0], "hop_seek")!=0 && strcmp(args[0], "iMan")!=0)
            {
                *ptr=1;
                strcpy(dummy1, command);
                for (int j = 0; j < i; j++) 
                {
                    free(args[j]);
                }
                free(args);
                fflush(stdout);
                dup2(std_out_dup, 1);
                close(std_out_dup);
                return;
            }
            for (int j = 0; j < i; j++) 
            {
                free(args[j]);
            }
            free(args);
            fflush(stdout);
            dup2(std_out_dup, 1);
            close(std_out_dup);
        }
        return;
    }
    else if(result1!=NULL && result3==NULL && result2==NULL)
    {
        char* token_red = strtok(dummy, ">");
        char token_red_dup[4096];
        strcpy(token_red_dup, token_red);
        char* redirect_file = strtok(NULL, " ");
        char current_dir[4096];
        getcwd(current_dir, sizeof(current_dir));
        char full_path[4097];
        if(redirect_file[0]!='/')
        {
            snprintf(full_path, sizeof(full_path), "%s/%s", current_dir, redirect_file);
        }
        else
        {
            strcpy(full_path, redirect_file);
        }
        int fd = open(full_path,  O_CREAT | O_TRUNC | O_WRONLY, 0644);
        if (fd == -1) 
        {
            printf("No such input file found\n");
            return;
        }
        char** args = (char**)malloc(sizeof(char*)*100);
        for(int i=0;i<100;i++)
        {
            args[i]=(char*)malloc(sizeof(char)*4096);
        }
        char* duplicate=strtok(token_red, " ");
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
        int std_out_dup=dup(1);
        close(1);
        dup2(fd, 1);
        close(fd);
        if(child<0)
        {
            fflush(stdout);
            dup2(std_out_dup, 1);
            close(std_out_dup);
            printf("\033[1;31mfork failed\033[0m\n");
            return;
        }
        else if(child==0)
        {
            execvp(args[0], args);
            *ptr=2;
            if(strcmp(args[0], "hop")!=0 && strcmp(args[0], "log")!=0 && strcmp(args[0], "reveal")!=0 && strcmp(args[0], "proclore")!=0 && strcmp(args[0], "seek")!=0 && strcmp(args[0], "activities")!=0 && strcmp(args[0], "ping")!=0 && strcmp(args[0], "fg")!=0 && strcmp(args[0], "home")!=0 && strcmp(args[0], "reveall")!=0 && strcmp(args[0], "reveala")!=0 && strcmp(args[0], "mk_hop")!=0 && strcmp(args[0], "hop_seek")!=0 && strcmp(args[0], "iMan")!=0)
            {
                fflush(stdout);
                dup2(std_out_dup, 1);
                close(std_out_dup);
                printf("\033[1;31mErronous command\033[0m\n"); 
                return;
            }
            
            run_foreground(token_red_dup, cwd, pwd, ptr, dummy1);
            fflush(stdout);
            dup2(std_out_dup, 1);
            close(std_out_dup);
        }
        else
        {
            waitpid(child, &status, 0);
            store_foreground[(idx3)]=child;
            strcpy(foreground_commands[idx3++], command);
            int final_time=time(NULL);
            int diff_time=final_time - initial_time;
            if(diff_time>2 && strcmp(args[0], "hop")!=0 && strcmp(args[0], "log")!=0 && strcmp(args[0], "reveal")!=0 && strcmp(args[0], "proclore")!=0 && strcmp(args[0], "seek")!=0 && strcmp(args[0], "activities")!=0 && strcmp(args[0], "ping")!=0 && strcmp(args[0], "fg")!=0 && strcmp(args[0], "home")!=0 && strcmp(args[0], "reveall")!=0 && strcmp(args[0], "reveala")!=0 && strcmp(args[0], "mk_hop")!=0 && strcmp(args[0], "hop_seek")!=0 && strcmp(args[0], "iMan")!=0)
            {
                *ptr=1;
                strcpy(dummy1, command);
                for (int j = 0; j < i; j++) 
                {
                    free(args[j]);
                }
                free(args);
                fflush(stdout);
                dup2(std_out_dup, 1);
                close(std_out_dup);
                return;
            }
            for (int j = 0; j < i; j++) 
            {
                free(args[j]);
            }
            free(args);
            fflush(stdout);
            dup2(std_out_dup, 1);
            close(std_out_dup);
            return;
        }
        
        return;
    }
    else if(result3!=NULL && result1==NULL)
    {
        char* token_red = strtok(dummy, "<");
        char token_red_dup[4096];
        strcpy(token_red_dup, token_red);
        char* redirect_file = strtok(NULL, " ");
        char current_dir[4096];
        getcwd(current_dir, sizeof(current_dir));
        char full_path[4097];
        if(redirect_file[0]!='/')
        {
            snprintf(full_path, sizeof(full_path), "%s/%s", current_dir, redirect_file);
        }
        else
        {
            strcpy(full_path, redirect_file);
        }
        int fd = open(full_path,O_RDONLY, 0644);
        if (fd == -1) 
        {
            printf("No such input file found\n");
            return;
        }
        char** args = (char**)malloc(sizeof(char*)*100);
        for(int i=0;i<100;i++)
        {
            args[i]=(char*)malloc(sizeof(char)*4096);
        }
        char* duplicate=strtok(token_red, " ");
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
        int std_in_dup=dup(0);
        int y=dup2(fd, 0);
        close(fd);
        if(child<0)
        {
            clearerr(stdin);
            dup2(std_in_dup, 0);
            close(std_in_dup);
            printf("\033[1;31mfork failed\033[0m\n");
            return;
        }
        else if(child==0)
        {
            execvp(args[0], args);
            *ptr=2;
            if(strcmp(args[0], "hop")!=0 && strcmp(args[0], "log")!=0 && strcmp(args[0], "reveal")!=0 && strcmp(args[0], "proclore")!=0 && strcmp(args[0], "seek")!=0 && strcmp(args[0], "activities")!=0 && strcmp(args[0], "ping")!=0 && strcmp(args[0], "fg")!=0 && strcmp(args[0], "home")!=0 && strcmp(args[0], "reveall")!=0 && strcmp(args[0], "reveala")!=0 && strcmp(args[0], "mk_hop")!=0 && strcmp(args[0], "hop_seek")!=0 && strcmp(args[0], "iMan")!=0)
            {
                clearerr(stdin);
                dup2(std_in_dup, 0);
                close(std_in_dup);
                printf("\033[1;31mErronous command\033[0m\n"); 
                return;
            }
            
            run_foreground(token_red_dup, cwd, pwd, ptr, dummy1);
            clearerr(stdin);
            dup2(std_in_dup, 0);
            close(std_in_dup);
        }
        else
        {
            waitpid(child, &status, 0);
            store_foreground[(idx3)]=child;
            strcpy(foreground_commands[idx3++], command);
            int final_time=time(NULL);
            int diff_time=final_time - initial_time;
            if(diff_time>2 && strcmp(args[0], "hop")!=0 && strcmp(args[0], "log")!=0 && strcmp(args[0], "reveal")!=0 && strcmp(args[0], "proclore")!=0 && strcmp(args[0], "seek")!=0 && strcmp(args[0], "activities")!=0 && strcmp(args[0], "ping")!=0 && strcmp(args[0], "fg")!=0 && strcmp(args[0], "home")!=0 && strcmp(args[0], "reveall")!=0 && strcmp(args[0], "reveala")!=0 && strcmp(args[0], "mk_hop")!=0 && strcmp(args[0], "hop_seek")!=0 && strcmp(args[0], "iMan")!=0)
            {
                *ptr=1;
                strcpy(dummy1, command);
                for (int j = 0; j < i; j++) 
                {
                    free(args[j]);
                }
                free(args);
                clearerr(stdin);
                dup2(std_in_dup, 0);
                close(std_in_dup);
                return;
            }
            for (int j = 0; j < i; j++) 
            {
                free(args[j]);
            }
            free(args);
            clearerr(stdin);
            dup2(std_in_dup, 0);
            close(std_in_dup);
            return;
        }
        return;
    }
    else 
    {
        if(result1!=NULL && result2==NULL)
        {
            char dupi[4096];
            strcpy(dupi, command);
            char* token = strtok(dupi, " \t<");
            char* command_dup = token;
            char* opera = strtok(NULL, " ");
            token = strtok(NULL, " \t>");
            char* input_file = token;
            opera = strtok(NULL, " ");
            token = strtok(NULL, " \t>");
            char* output_file = token;
            char current_dir[4096];
            getcwd(current_dir, sizeof(current_dir));
            char full_path[4097];
            char full_path2[4097];
            if(output_file[0]!='/')
            {
                snprintf(full_path2, sizeof(full_path), "%s/%s", current_dir, output_file);
            }
            else
            {
                strcpy(full_path2, output_file);
            }
            int fd = open(full_path2, O_CREAT | O_TRUNC | O_WRONLY, 0644);
            if (fd == -1) {
                perror("open");
                return ;
            }
            if(input_file[0]!='/')
            {
                snprintf(full_path, sizeof(full_path), "%s/%s", current_dir, input_file);
            }
            else
            {
                strcpy(full_path, input_file);
            }
            int fd2 = open(full_path,  O_RDONLY, 0644);
            if (fd2 == -1) 
            {
                printf("No such input file found\n");
                return;
            }
            int std_out_dup=dup(1);
            dup2(fd, 1);
            close(fd);
            int std_in_dup=dup(0);
            int y=dup2(fd2, 0);
            close(fd2);
            run_foreground(command_dup, cwd, pwd, ptr, dummy1);
            clearerr(stdin);
            dup2(std_in_dup, 0);
            close(std_in_dup);
            fflush(stdout);
            dup2(std_out_dup, 1);
            close(std_out_dup);
            return;
        }
        else if (result2!=NULL && result1==NULL)
        {
            char dupi[4096];
            strcpy(dupi, command);
            char* token = strtok(dupi, " \t<");
            char* command_dup = token;
            char* opera = strtok(NULL, " ");
            token = strtok(NULL, " \t>>");
            char* input_file = token;
            opera = strtok(NULL, " ");
            token = strtok(NULL, " \t>>");
            char* output_file = token;
            char current_dir[4096];
            getcwd(current_dir, sizeof(current_dir));
            char full_path[4097];
            char full_path2[4097];
            if(output_file[0]!='/')
            {
                snprintf(full_path2, sizeof(full_path), "%s/%s", current_dir, output_file);
            }
            else
            {
                strcpy(full_path2, output_file);
            }
            int fd = open(full_path2, O_CREAT |O_APPEND | O_WRONLY, 0644);
            if (fd == -1) {
                perror("open");
                return ;
            }
            if(input_file[0]!='/')
            {
                snprintf(full_path, sizeof(full_path), "%s/%s", current_dir, input_file);
            }
            else
            {
                strcpy(full_path, input_file);
            }
            int fd2 = open(full_path,  O_RDONLY, 0644);
            if (fd2 == -1) 
            {
                printf("No such input file found\n");
                return;
            }
            int std_out_dup=dup(1);
            dup2(fd, 1);
            close(fd);
            int std_in_dup=dup(0);
            int y=dup2(fd2, 0);
            close(fd2);
            run_foreground(command_dup, cwd, pwd, ptr, dummy1);
            clearerr(stdin);
            dup2(std_in_dup, 0);
            close(std_in_dup);
            fflush(stdout);
            dup2(std_out_dup, 1);
            close(std_out_dup);
            return;
        }
    }
    return;
}