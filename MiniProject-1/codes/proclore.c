#include "proclore.h"
#include "commands.h"
#include<stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>

void execute_proclore(char* token1)
{
    char path[256];
    char buffer[256];
    FILE *file;
    pid_t pid;
    if(token1!=NULL)
    {
        printf("pid : %s\n", token1);
        int i=0;
        int x=0;
        while(token1[i]!='\0')
        {
            x=(10*x)+token1[i] - '\0';
        }
        pid = x;
    }
    else
    {
        pid = getpid();
        printf("pid : %d\n",pid);
    }
    char stat_path[256];
    FILE *fp;
    char stat_line[256];
    char state;
    int is_foreground = 0;
    sprintf(stat_path, "/proc/%d/stat", pid);
    fp = fopen(stat_path, "r");
    if (fp == NULL) {
        perror("\033[1;31mfopen\033[0m");
        return;
    }
    printf("State : ");
     // Read the state character
    if (fscanf(fp, "%*d %*s %c", &state) == 1) {
        // Check if the process is running and assume foreground
        if (state == 'R') is_foreground = 1;

        // Print the state with color coding and optional '+'
        switch (state) {
            case 'R': printf(is_foreground ? "\033[1;32mr+\033[0m\n" : "\033[1;32mr\033[0m\n"); break;
            case 'S': printf(is_foreground ? "\033[1;33ms+\033[0m\n" : "\033[1;33ms\033[0m\n"); break;
            case 'Z': printf("\033[1;31mz\033[0m\n"); break;
            default: printf("\033[1;37mUnknown\033[0m\n"); break;
        }
    }

    printf("Process Group: %d\n", getpgid(pid));
    char status_path[256];
    FILE *fp1;
    char line[256];
    sprintf(status_path, "/proc/%d/status", pid);
    fp1 = fopen(status_path, "r");
    if (fp1 == NULL) {
        perror("\033[1;31mfopen\033[0m");
        return;
    }
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "VmSize:", 7) == 0) 
        {
            char *mem_value = line + 7;
            while (isspace(*mem_value)) {
                mem_value++;
            }
            char *end = mem_value + strlen(mem_value) - 1;
            while (end > mem_value && isspace(*end)) {
                *end = '\0';
                end--;
            }

            printf("Virtual memory: %s\n", mem_value);
            break;
        }
    }
    fclose(fp1);
    snprintf(path, sizeof(path), "/proc/%d/exe", pid);
    char exe_path[256];
    ssize_t len = readlink(path, exe_path, sizeof(exe_path) - 1);
    if (len == -1) {
        perror("\033[1;31mreadlink failed\033[0m");
        return;
    }
    exe_path[len] = '\0';
    printf("Executable Path: %s\n", exe_path);
}