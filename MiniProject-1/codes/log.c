#include "log.h"
#include "input.h"
#include "commands.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>  
#include <unistd.h> 
#include <stdlib.h>

void execute_log(char* token1, char* token2, char* home, char* pwd,int* ptr, char* dummy1) 
{
    if (strcmp(token1, "purge") == 0) 
    {
        purge_log(home);
    } 
    else if (strcmp(token1, "execute") == 0) 
    {
        int idx = atoi(token2);
        if (idx < 1 || idx > 15) 
        { 
            printf("\033[1;31mIndex out of range. Valid range is 1 to 15.\033[0m\n");
            foreground_running=0;
            return;
        }
        char log_file_path[1024];
        snprintf(log_file_path, sizeof(log_file_path), "%s/log.txt", home);
        int log_file_fd = open(log_file_path, O_RDONLY);
        if (log_file_fd == -1) 
        {
            perror("\033[1;31mopen\033[0m");
            foreground_running=0;
            return;
        }
        int total_lines = 0;
        char ch;
        off_t file_size = lseek(log_file_fd, 0, SEEK_END);
        lseek(log_file_fd, 0, SEEK_SET);  
        while (read(log_file_fd, &ch, 1) == 1) 
        {
            if (ch == '\n') 
            {
                total_lines++;
            }
        }
        if (file_size > 0) 
        {
            lseek(log_file_fd, -1, SEEK_END);
            read(log_file_fd, &ch, 1);
            if (ch != '\n') 
            {
                total_lines++; 
            }
        }
        if (total_lines < idx) 
        {
            printf("Not enough commands in the log. Total commands: %d\n", total_lines);
            foreground_running=0;
            close(log_file_fd);
            return;
        }
        lseek(log_file_fd, 0, SEEK_SET); 
        int target_line = total_lines - idx + 1;
        int current_line = 0;
        int line_index = 0;
        char command[4096];
        while (read(log_file_fd, &ch, 1) == 1) 
        {
            if (ch == '\n') 
            {
                current_line++;
                if (current_line == target_line) 
                {
                    command[line_index] = '\0';
                    break;
                }
                line_index = 0;
            } 
            else if (current_line == target_line - 1) 
            {
                if (line_index < sizeof(command) - 1) 
                {
                    command[line_index++] = ch;
                }
            }
        }
        if (current_line == target_line - 1) 
        {
            command[line_index] = '\0'; 
        }
        if (current_line >= target_line - 1) 
        {
            char cwd[1024];
            getcwd(cwd, sizeof(cwd));
            execute_command(command, cwd, pwd, ptr, dummy1);
        } 
        else 
        {
            printf("\033[1;31mCommand not found at the specified index.\033[0m\n");
        }
        close(log_file_fd);
    }
}

void print_log(char* home)
{
    char filename[1024];
    snprintf(filename, sizeof(filename), "%s/log.txt", home);
    ssize_t bytesRead;
    char buffer[4096];
    int flag=0;
    int fd = open(filename, O_RDONLY, 0644);
    if (fd < 0) 
    {
        printf("\033[1;31mError opening file\033[0m");
        foreground_running=0;
        return;
    }
     while ((bytesRead = read(fd, buffer, 4096)) > 0) {
        if (write(STDOUT_FILENO, buffer, bytesRead) == -1) {
            perror("\033[1;31mFailed to write to stdout\033[0m");
            foreground_running=0;
            close(fd);
            exit(EXIT_FAILURE);
        }
        flag=1;
    }
    if(flag==1)
        printf("\n");
    if (bytesRead == -1) {
        perror("\033[1;31mFailed to read file\033[0m");
        foreground_running=0;
        close(fd);
        exit(EXIT_FAILURE);
    }
    close(fd);
}

void store_log(char* command, char* home)
{
    char *result = strstr(command, "log ");
    if (result) 
    {
        return;
    }
    char* result2 = strstr(command , "log");
    if(result2)
    {
        return;
    }
    if(!result && !result2)
    {
       char log_file_path[1024];
        snprintf(log_file_path, sizeof(log_file_path), "%s/log.txt", home);
        int log_fd = open(log_file_path, O_RDWR | O_APPEND);
        if (log_fd == -1) 
        {
            perror("\033[1;31mCouldn't open the file\033[0m");
            foreground_running=0;
            return;
        }

        char buf[4096];
        int nb_read;
        int line_count = 0;
        off_t file_size = lseek(log_fd, 0, SEEK_END);
        if (file_size == -1) 
        {
            perror("\033[1;31mLseek error\033[0m");
            foreground_running=0;
            close(log_fd);
            return;
        }
        lseek(log_fd, 0, SEEK_SET);
        if (file_size == 0)
        {
            write(log_fd, command, strlen(command));
            return;
        }
        lseek(log_fd, 0, SEEK_SET);
        int no_of_lines = 1;
        char ch;
        while (read(log_fd, &ch, 1) == 1)
        {
            if (ch == '\n')
            {
                no_of_lines++;
            }
        }
        if(no_of_lines<15)
        {
            char last_line[4096] = {0};
            if(no_of_lines==1)
            {
                lseek(log_fd, 0, SEEK_SET);
                int i=0;
                while (read(log_fd, &ch, 1) == 1 && ch != '\n') 
                {
                    if (i < 4096 - 1) 
                    {
                        last_line[i++] = ch;
                    }
                }
                last_line[i] = '\0';
                if(strcmp(last_line, command)!=0)
                {
                    lseek(log_fd, 0, SEEK_END);
                    write(log_fd, "\n", 1);
                    write(log_fd, command, strlen(command));
                    close(log_fd);
                    return;
                }
            }
            else
            {
                off_t offset = file_size - 1;
                while (offset >= 0) 
                {
                    lseek(log_fd, offset, SEEK_SET);
                    nb_read = read(log_fd, buf, 1);
                    if (nb_read == -1) 
                    {
                        perror("\033[1;31mRead error\033[0m");
                        foreground_running=0;
                        close(log_fd);
                        return;
                    }

                    if (buf[0] == '\n') 
                    {
                        lseek(log_fd, offset + 1, SEEK_SET);
                        break;
                    }
                    offset--;
                }
                int last_line_len = 0;
                while ((nb_read = read(log_fd, buf, sizeof(buf))) > 0) 
                {
                    for (int i = 0; i < nb_read; i++) 
                    {
                        if (buf[i] == '\n') 
                        {
                            break;
                        }
                        last_line[last_line_len++] = buf[i];
                    }
                    if (buf[nb_read - 1] == '\n') 
                    {
                        break;
                    }
                }

                if (nb_read == -1) 
                {
                    perror("\033[1;31mRead error\033[0m");
                    foreground_running=0;
                    close(log_fd);
                    return;
                }
                if(strcmp(command, last_line)!=0)
                {
                    lseek(log_fd, 0, SEEK_END);
                    write(log_fd, "\n", 1);
                    write(log_fd, command, strlen(command));
                    close(log_fd);
                    return;
                }
            }
        }
        else if(no_of_lines==15)
        {
            
            char last_line[4096] = {0};
            off_t offset = file_size - 1;
                while (offset >= 0) 
                {
                    lseek(log_fd, offset, SEEK_SET);
                    nb_read = read(log_fd, buf, 1);
                    if (nb_read == -1) 
                    {
                        perror("\033[1;31mRead error\033[0m");
                        foreground_running=0;
                        close(log_fd);
                        return;
                    }

                    if (buf[0] == '\n') 
                    {
                        lseek(log_fd, offset + 1, SEEK_SET);
                        break;
                    }
                    offset--;
                }
                int last_line_len = 0;
                while ((nb_read = read(log_fd, buf, sizeof(buf))) > 0) 
                {
                    for (int i = 0; i < nb_read; i++) 
                    {
                        if (buf[i] == '\n') 
                        {
                            break;
                        }
                        last_line[last_line_len++] = buf[i];
                    }
                    if (buf[nb_read - 1] == '\n') 
                    {
                        break;
                    }
                }

                if (nb_read == -1) 
                {
                    perror("\033[1;31mRead error\033[0m");
                    foreground_running=0;
                    close(log_fd);
                    return;
                }
                if(strcmp(command, last_line)!=0)
                {
                    int fd_in, fd_out;
                    char buffer[4096];
                    ssize_t bytes_read;
                    char log_file_path[1024];
                    snprintf(log_file_path, sizeof(log_file_path), "%s/log.txt", home);
                    fd_in = open(log_file_path, O_RDONLY);
                    char log_file_path1[1024];
                    snprintf(log_file_path1, sizeof(log_file_path1), "%s/dup.txt", home);
                    fd_out = open(log_file_path1, O_WRONLY | O_CREAT | O_TRUNC, 0644);

                    if (fd_in == -1 || fd_out == -1) {
                        perror("\033[1;31mError opening files\033[0m");
                        foreground_running=0;
                        return;
                    }
                    bytes_read = read(fd_in, buffer, sizeof(buffer));
                    if (bytes_read <= 0) 
                    {
                        perror("\033[1;31mError reading from input file\033[0m");
                        close(fd_in);
                        close(fd_out);
                        foreground_running=0;
                        return;
                    }
                    char *newline_ptr = strchr(buffer, '\n');
                    if (newline_ptr != NULL) 
                    {
                        newline_ptr++;  
                        write(fd_out, newline_ptr, bytes_read - (newline_ptr - buffer));
                    }
                    while ((bytes_read = read(fd_in, buffer, sizeof(buffer))) > 0) 
                    {
                        write(fd_out, buffer, bytes_read);
                    }
                    if (bytes_read == -1)
                    {
                        perror("\033[1;31mError reading from input file\033[0m");
                        foreground_running=0;
                        return;
                    }

                    close(fd_in);
                    close(fd_out);
                    fd_in = open(log_file_path1, O_RDONLY);
                    if (fd_in == -1) {
                        perror("\033[1;31mError opening input file\033[0m");
                        foreground_running=0;
                        return;
                    }
                    fd_out = open(log_file_path, O_WRONLY  | O_TRUNC, 0644);
                    if (fd_out == -1) {
                        perror("\033[1;31mError opening output file\033[0m");
                        foreground_running=0;
                        close(fd_in);
                        return;
                    }
                    while ((bytes_read = read(fd_in, buffer, sizeof(buffer))) > 0) 
                    {
                        if (write(fd_out, buffer, bytes_read) != bytes_read) 
                        {
                            perror("\033[1;31mError writing to output file\033[0m");
                            close(fd_in);
                            close(fd_out);
                            foreground_running=0;
                            return;
                        }
                    }
                    if (bytes_read == -1) 
                    {
                        perror("\033[1;31mError reading from input file\033[0m");
                        foreground_running=0;
                    }
                    close(fd_in);
                    close(fd_out);
                    lseek(log_fd, 0, SEEK_END);
                    write(log_fd, "\n", 1);
                    write(log_fd, command, strlen(command));
                    close(log_fd);
                    return;
                }
        }
    }
}

void purge_log(char* home) {
    char log_file_path[1024];
    snprintf(log_file_path, sizeof(log_file_path), "%s/log.txt", home);
    int log_fd = open(log_file_path, O_TRUNC);
    if (log_fd == -1) 
    {
        perror("\033[1;31mCouldn't open the file\033[0m");
        foreground_running=0;
        return;
    }
    close(log_fd);
}