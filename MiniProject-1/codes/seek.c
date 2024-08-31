#include "reveal.h"
#include "hop.h"
#include "commands.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h> 
#include <libgen.h>

int has_file_extension(char *filename) 
{
    const char *dot = strrchr(filename, '.');
    if (dot != NULL && dot != filename && *(dot + 1) != '\0') 
    {
        return 1; 
    }
    return 0; 
}

void search_file(const char *folder, const char *base_folder, const char *filename, int* ptr) 
{
    struct dirent *de; 
    DIR *dr = opendir(folder);
    if (dr == NULL)  
    {
        printf("\033[1;31merror opening directory\033[1;31m\n");
        return;
    }
    char **filenames = NULL;
    size_t count = 0;
    
    while ((de = readdir(dr)) != NULL) {
        if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0 && de->d_name[0] != '.') {
            filenames = realloc(filenames, sizeof(char*) * (count + 1));
            filenames[count] = strdup(de->d_name);
            count++;
        }
    }
    closedir(dr);
    
    qsort(filenames, count, sizeof(char*), compare);
    
    for (size_t i = 0; i < count; i++) {
        struct stat st;
        char full_path[4096];
        snprintf(full_path, sizeof(full_path), "%s/%s", folder, filenames[i]);
        
        if (stat(full_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) 
            {
                if (strncmp(filenames[i], filename, strlen(filename)) == 0) 
                {
                    (*ptr)++;
                    printf(".%s\n", full_path + strlen(base_folder));
                }
                search_file(full_path, base_folder, filename, ptr);
            } 
            else 
            {
                if (strncmp(filenames[i], filename, strlen(filename)) == 0) 
                {
                    (*ptr)++;
                    printf(".%s\n", full_path + strlen(base_folder));
                }
            }
        } 
        else 
        {
            printf("\033[1;31merror in checking status of the file\033[1;31m\n");
        }

        free(filenames[i]);
    }
    free(filenames); 
}

void execute_seek_cwd(char* token, int* ptr)
{
    int if_file_ext = has_file_extension(token);
    char cwd1[1024];
    if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
    {
        perror("\033[1;31mgetcwd\033[1;31m");
        return;
    }
    if(if_file_ext)
    {
        struct stat path_stat;
        char full_path[4097];
        snprintf(full_path, sizeof(full_path), "%s/%s", cwd1, token);
        if (stat(full_path, &path_stat) == 0 && S_ISREG(path_stat.st_mode))
        {
            search_file(cwd1, cwd1, token, ptr);
        }
        return;
    }
    else
    {
        search_file(cwd1, cwd1,  token, ptr);
    }
}

void execute_seek_path(char* file, char* target, char* pwd, char* home, int* ptr)
{
    int if_file_ext = has_file_extension(file);
    if(!if_file_ext)
    {
        if(strcmp(target, "~")==0)
        {
            search_file(home, home,  file, ptr);
            return;
        }
        if(strcmp(target, "-")==0)
        {
            search_file(pwd, pwd, file, ptr);
            return;
        }
        if(strcmp(target, ".")==0)
        {
            char cwd1[1024];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[1;31m");
                return;
            }
            search_file(cwd1, cwd1, file, ptr);
            return;
        }
        if(strcmp(target, "..")==0)
        {
            char cwd1[1024];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[1;31m");
                return;
            }
            char *parent_dir = strdup(cwd1);
            parent_dir = dirname(parent_dir);
            search_file(parent_dir, parent_dir, file, ptr);
            return;
        }
        if(target[0] == '/')
        {
            search_file(target, target, file, ptr);
            return;
        }
        if(target[0]=='~' && target[1]=='/')
        {
            char full_path[4096];
            snprintf(full_path, sizeof(full_path), "%s%s",home , target + 1);
            search_file(full_path, full_path, file, ptr);
            return;
        }
        if(target[0]=='.' && target[1]=='/')
        {
            char cwd1[1024];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[1;31m");
                return;
            }
            char full_path[4096];
            snprintf(full_path, sizeof(full_path), "%s%s",home , target + 1);
            search_file(full_path, full_path, file, ptr);
            return;
        }
    }
}

void search_only_folders(const char *folder, const char *base_folder, const char *filename, int* ptr)
{
    struct dirent *de; 
    DIR *dr = opendir(folder);
    if (dr == NULL)  
    {
        printf("\033[1;31merror opening directory\033[1;31m\n");
        return;
    }
    char **filenames = NULL;
    size_t count = 0;
    
    while ((de = readdir(dr)) != NULL) {
        if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0 && de->d_name[0] != '.') {
            filenames = realloc(filenames, sizeof(char*) * (count + 1));
            filenames[count] = strdup(de->d_name);
            count++;
        }
    }
    closedir(dr);
    
    qsort(filenames, count, sizeof(char*), compare);
    for (size_t i = 0; i < count; i++) {
        struct stat st;
        char full_path[4096];
        snprintf(full_path, sizeof(full_path), "%s/%s", folder, filenames[i]);
        
        if (stat(full_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) 
            {
                if (strncmp(filenames[i], filename, strlen(filename)) == 0) 
                {
                    (*ptr)++;
                    printf(".%s\n", full_path + strlen(base_folder));
                }
                search_only_folders(full_path, base_folder, filename, ptr);
            } 
        } 
        else 
        {
            printf("\033[1;31merror in checking status of the file\033[1;31m\n");
        }

        free(filenames[i]);
    }
    free(filenames); 
}

void search_only_files(const char *folder, const char *base_folder, const char *filename, int* ptr)
{
    struct dirent *de; 
    DIR *dr = opendir(folder);
    if (dr == NULL)  
    {
        printf("\033[1;31merror opening directory\033[1;31m\n");
        return;
    }
    char **filenames = NULL;
    size_t count = 0;
    
    while ((de = readdir(dr)) != NULL) {
        if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0 && de->d_name[0] != '.') {
            filenames = realloc(filenames, sizeof(char*) * (count + 1));
            filenames[count] = strdup(de->d_name);
            count++;
        }
    }
    closedir(dr);
    
    qsort(filenames, count, sizeof(char*), compare);
    
    for (size_t i = 0; i < count; i++) {
        struct stat st;
        char full_path[4096];
        snprintf(full_path, sizeof(full_path), "%s/%s", folder, filenames[i]);
        
        if (stat(full_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) 
            {
                search_file(full_path, base_folder, filename, ptr);
            } 
            else 
            {
                if (strncmp(filenames[i], filename, strlen(filename)) == 0) 
                {
                    (*ptr)++;
                    printf(".%s\n", full_path + strlen(base_folder));
                }
            }
        } 
        else 
        {
            printf("\033[1;31merror in checking status of the file\033[1;31m\n");
        }

        free(filenames[i]);
    }
    free(filenames); 
}

void search_file_ed(const char *folder, const char *base_folder, const char *filename, int *ptr, int d, int f, char *found) 
{
    struct dirent *de; 
    DIR *dr = opendir(folder);
    if (dr == NULL)  
    {
        printf("\033[1;31merror opening directory\033[1;31m\n");
        return;
    }
    char **filenames = NULL;
    size_t count = 0;
    
    while ((de = readdir(dr)) != NULL) {
        if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0 && de->d_name[0] != '.') {
            filenames = realloc(filenames, sizeof(char*) * (count + 1));
            filenames[count] = strdup(de->d_name);
            count++;
        }
    }
    closedir(dr);
    
    qsort(filenames, count, sizeof(char*), compare);
    for (size_t i = 0; i < count; i++) {
        struct stat st;
        char full_path[4096];
        snprintf(full_path, sizeof(full_path), "%s/%s", folder, filenames[i]);
        
        if (stat(full_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) 
            {
                if (strncmp(filenames[i], filename, strlen(filename)) == 0) 
                {
                    (*ptr)++;
                    if((*ptr)>1)
                    {
                        strcpy(found, "more_files");
                        return;
                    }
                    strcpy(found, full_path);
                }
                search_file_ed(full_path, base_folder, filename, ptr, d, f, found);
            } 
        } 
        else 
        {
            printf("\033[1;31merror in checking status of the file\033[1;31m\n");
        }

        free(filenames[i]);
    }
    free(filenames); 
}

void search_file_ef(const char *folder, const char *base_folder, const char *filename, int *ptr, int d, int f, char *found)
{
    struct dirent *de; 
    DIR *dr = opendir(folder);
    if (dr == NULL)  
    {
        printf("\033[1;31merror opening directory\033[1;31m\n");
        return;
    }
    char **filenames = NULL;
    size_t count = 0;
    
    while ((de = readdir(dr)) != NULL) {
        if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0 && de->d_name[0] != '.') {
            filenames = realloc(filenames, sizeof(char*) * (count + 1));
            filenames[count] = strdup(de->d_name);
            count++;
        }
    }
    closedir(dr);
    
    qsort(filenames, count, sizeof(char*), compare);
    
    for (size_t i = 0; i < count; i++) {
        struct stat st;
        char full_path[4096];
        snprintf(full_path, sizeof(full_path), "%s/%s", folder, filenames[i]);
        
        if (stat(full_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) 
            {
                search_file_ef(full_path, base_folder, filename, ptr, d, f, found);;
            } 
            else 
            {
                if (strncmp(filenames[i], filename, strlen(filename)) == 0) 
                {
                    (*ptr)++;
                    if((*ptr)>1)
                    {
                        strcpy(found, "more_files");
                        return;
                    }
                    strcpy(found, full_path);
                }
            }
        } 
        else 
        {
            printf("\033[1;31merror in checking status of the file\033[1;31m\n");
        }

        free(filenames[i]);
    }
    free(filenames); 
}

void search_file_e(const char *folder, const char *base_folder, const char *filename, int *ptr, int d, int f, char *found)
{
    struct dirent *de; 
    DIR *dr = opendir(folder);
    if (dr == NULL)  
    {
        printf("\033[1;31merror opening directory\033[1;31m\n");
        return;
    }
    char **filenames = NULL;
    size_t count = 0;
    
    while ((de = readdir(dr)) != NULL) {
        if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0 && de->d_name[0] != '.') {
            filenames = realloc(filenames, sizeof(char*) * (count + 1));
            filenames[count] = strdup(de->d_name);
            count++;
        }
    }
    closedir(dr);
    
    qsort(filenames, count, sizeof(char*), compare);
    
    for (size_t i = 0; i < count; i++) {
        struct stat st;
        char full_path[4096];
        snprintf(full_path, sizeof(full_path), "%s/%s", folder, filenames[i]);
        
        if (stat(full_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) 
            {
                if (strncmp(filenames[i], filename, strlen(filename)) == 0) 
                {
                    (*ptr)++;
                    if((*ptr)>1)
                    {
                        strcpy(found, "more_files");
                        return;
                    }
                    strcpy(found, full_path);
                }
                search_file_e(full_path, base_folder, filename, ptr, d, f, found);
            } 
            else 
            {
                if (strncmp(filenames[i], filename, strlen(filename)) == 0) 
                {
                    (*ptr)++;
                    if((*ptr)>1)
                    {
                        strcpy(found, "more_files");
                        return;
                    }
                    strcpy(found, full_path);
                }
            }
        } 
        else 
        {
            printf("\033[1;31merror in checking status of the file\033[1;31m\n");
        }

        free(filenames[i]);
    }
    free(filenames); 
}

void execute_seek(char* path,char* file, int d, int e, int f, char* pwd, char* home, int* ptr)
{
    if(d!=0 && f!=0)
    {
        printf("\033[1;31mInvalid Flags\033[1;31m\n");
        return;
    }
    if(d!=0 && e==0 && f==0)
    {
        *ptr=0;
        if(path==NULL)
        {
            char cwd1[4096];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[1;31m");
                return;
            }
            search_only_folders(cwd1, cwd1, file, ptr);
            return;
        }
        if(strcmp(path, "~")==0)
        {
            search_only_folders(home, home,  file, ptr);
            return;
        }
        if(strcmp(path, "-")==0)
        {
            search_only_folders(pwd, pwd, file, ptr);
            return;
        }
        if(strcmp(path, ".")==0)
        {
            char cwd1[1024];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[1;31m");
                return;
            }
            search_only_folders(cwd1, cwd1, file, ptr);
            return;
        }
        if(strcmp(path, "..")==0)
        {
            char cwd1[1024];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[1;31m");
                return;
            }
            char *parent_dir = strdup(cwd1);
            parent_dir = dirname(parent_dir);
            search_only_folders(parent_dir, parent_dir, file, ptr);
            return;
        }
        if(path[0]=='/')
        {
            search_only_folders(path, path, file, ptr);
            return;
        }
        if(path[0]=='~' && path[1]=='/')
        {
            char full_path[4096];
            snprintf(full_path, sizeof(full_path), "%s%s",home , path + 1);
            search_only_folders(full_path, full_path, file, ptr);
            return;
        }
        if(path[0]=='.' && path[1]=='/')
        {
            char cwd1[1024];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[1;31m");
                return;
            }
            char full_path[4096];
            snprintf(full_path, sizeof(full_path), "%s%s",home , path + 1);
            search_only_folders(full_path, full_path, file, ptr);
            return;
        }
    }
    if(f!=0 && e==0 && d==0)
    {
        *ptr=0;
        if(path==NULL)
        {
            char cwd1[4096];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[1;31m");
                return;
            }
            search_only_files(cwd1, cwd1, file, ptr);
            return;
        }
        if(strcmp(path, "~")==0)
        {
            search_only_files(home, home,  file, ptr);
            return;
        }
        if(strcmp(path, "-")==0)
        {
            search_only_files(pwd, pwd, file, ptr);
            return;
        }
        if(strcmp(path, ".")==0)
        {
            char cwd1[1024];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[1;31m");
                return;
            }
            search_only_files(cwd1, cwd1, file, ptr);
            return;
        }
        if(strcmp(path, "..")==0)
        {
            char cwd1[1024];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[1;31m");
                return;
            }
            char *parent_dir = strdup(cwd1);
            parent_dir = dirname(parent_dir);
            search_only_files(parent_dir, parent_dir, file, ptr);
            return;
        }
        if(path[0]=='/')
        {
            search_only_files(path, path, file, ptr);
            return;
        }
        if(path[0]=='~' && path[1]=='/')
        {
            char full_path[4096];
            snprintf(full_path, sizeof(full_path), "%s%s",home , path + 1);
            search_only_files(full_path, full_path, file, ptr);
            return;
        }
        if(path[0]=='.' && path[1]=='/')
        {
            char cwd1[1024];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[1;31m");
                return;
            }
            char full_path[4096];
            snprintf(full_path, sizeof(full_path), "%s%s",home , path + 1);
            search_only_files(full_path, full_path, file, ptr);
            return;
        }   
    }
    if(e!=0 && d!=0)
    {
        *ptr=0;
        char* found=(char*)malloc(sizeof(char)*4096);
        if(path==NULL)
        {
            char cwd1[4096];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[1;31m");
                return;
            }
            search_file_ed(cwd1, cwd1, file, ptr, d, f, found);
        }
        else if(strcmp(path, "~")==0)
        {
            search_file_ed(home, home,  file, ptr, d,f, found);
        }
        else if(strcmp(path, "-")==0)
        {
            search_file_ed(pwd, pwd, file, ptr, d , f, found);
        }
        else if(strcmp(path, ".")==0)
        {
            char cwd1[1024];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[1;31m");
                return;
            }
            search_file_ed(cwd1, cwd1, file, ptr, d, f, found);
        }
        else if(strcmp(path, "..")==0)
        {
            char cwd1[1024];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[1;31m");
                return;
            }
            char *parent_dir = strdup(cwd1);
            parent_dir = dirname(parent_dir);
            search_file_ed(parent_dir, parent_dir, file, ptr, d, f, found);
        }
        else if(path[0]=='/')
        {
            search_file_ed(path, path, file, ptr, d, f, found);
        }
        else if(path[0]=='~' && path[1]=='/')
        {
            char full_path[4096];
            snprintf(full_path, sizeof(full_path), "%s%s",home , path + 1);
            search_file_ed(full_path, full_path, file, ptr, d, f, found);
        }
        else if(path[0]=='.' && path[1]=='/')
        {
            char cwd1[1024];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[1;31m");
                return;
            }
            char full_path[4096];
            snprintf(full_path, sizeof(full_path), "%s%s",home , path + 1);
            search_file_ed(full_path, full_path, file, ptr, d, f, found);
        }
        if((*ptr)==1)
        {
            struct stat path_stat;
            if (stat(found, &path_stat) != 0) 
            {
                perror("\033[1;31mstat\033[1;31m");
                return;
            }

            if (S_ISDIR(path_stat.st_mode)) 
            {
                if (access(found, X_OK) == -1) 
                {
                    printf("\033[1;31mMissing permissions for Task\033[1;31m");
                    return;
                }
                char cwd1[1024];
                getcwd(cwd1, sizeof(cwd1));
                if(chdir(found)!=0)
                {
                    perror("\033[1;31mchdir to specified directory failed\033[1;31m");
                    return;
                }
                strcpy(pwd, cwd1);
                return;
            } 
            return;
        }
    }
    if(e!=0 && f!=0)
    {
        *ptr = 0;
        char* found=(char*)malloc(sizeof(char)*4096);
        if(path==NULL)
        {
            char cwd1[4096];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[1;31m");
                return;
            }
            search_file_ef(cwd1, cwd1, file, ptr, d, f, found);
        }
        else if(strcmp(path, "~")==0)
        {
            search_file_ef(home, home,  file, ptr, d,f, found);
        }
        else if(strcmp(path, "-")==0)
        {
            search_file_ef(pwd, pwd, file, ptr, d , f, found);
        }
        else if(strcmp(path, ".")==0)
        {
            char cwd1[1024];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[1;31m");
                return;
            }
            search_file_ef(cwd1, cwd1, file, ptr, d, f, found);
        }
        else if(strcmp(path, "..")==0)
        {
            char cwd1[1024];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[1;31m");
                return;
            }
            char *parent_dir = strdup(cwd1);
            parent_dir = dirname(parent_dir);
            search_file_ef(parent_dir, parent_dir, file, ptr, d, f, found);
        }
        else if(path[0]=='/')
        {
            search_file_ed(path, path, file, ptr, d, f, found);
        }
        else if(path[0]=='~' && path[1]=='/')
        {
            char full_path[4096];
            snprintf(full_path, sizeof(full_path), "%s%s",home , path + 1);
            search_file_ef(full_path, full_path, file, ptr, d, f, found);
        }
        else if(path[0]=='.' && path[1]=='/')
        {
            char cwd1[1024];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[1;31m");
                return;
            }
            char full_path[4096];
            snprintf(full_path, sizeof(full_path), "%s%s",home , path + 1);
            search_file_ef(full_path, full_path, file, ptr, d, f, found);
        }
        if((*ptr)==1)
        {
            printf("%s\n", found);
            if (access(found, R_OK) == -1) 
            {
                printf("\033[1;31mMissing permissions for Task\033[1;31m");
                return ;
            }

            int fd;
            ssize_t bytesRead;
            char *buffer;
            size_t bufferSize = 4096;
            size_t totalBytesRead = 0;
            fd = open(found, O_RDONLY);
            if (fd == -1) 
            {
                perror("\033[1;31mError opening the file\033[1;31m");
                return;
            }
            buffer = (char *)malloc(bufferSize);
            if (buffer == NULL) 
            {
                perror("\033[1;31mError allocating memory\033[1;31m");
                close(fd);
                return;
            }
            while ((bytesRead = read(fd, buffer + totalBytesRead, bufferSize - totalBytesRead)) > 0) 
            {
                totalBytesRead += bytesRead;
                if (totalBytesRead == bufferSize) 
                {
                    bufferSize *= 2; 
                    buffer = (char *)realloc(buffer, bufferSize);
                    if (buffer == NULL) 
                    {
                        perror("\033[1;31mError reallocating memory\033[1;31m");
                        close(fd);
                        return;
                    }
                }
            }

            if (bytesRead == -1) 
            {
                perror("\033[1;31mError reading the file\033[1;31m");
                free(buffer);
                close(fd);
                return;
            }
            buffer[totalBytesRead] = '\0';
            printf("%s", buffer);
            close(fd);
            free(buffer);
        }
    }
    if(e!=0)
    {
        *ptr = 0;
        char* found=(char*)malloc(sizeof(char)*4096);
        if(path==NULL)
        {
            char cwd1[4096];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[1;31m");
                return;
            }
            search_file_e(cwd1, cwd1, file, ptr, d, f, found);
        }
        else if(strcmp(path, "~")==0)
        {
            search_file_e(home, home,  file, ptr, d,f, found);
        }
        else if(strcmp(path, "-")==0)
        {
            search_file_e(pwd, pwd, file, ptr, d , f, found);
        }
        else if(strcmp(path, ".")==0)
        {
            char cwd1[1024];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[1;31m");
                return;
            }
            search_file_e(cwd1, cwd1, file, ptr, d, f, found);
        }
        else if(strcmp(path, "..")==0)
        {
            char cwd1[1024];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[1;31m");
                return;
            }
            char *parent_dir = strdup(cwd1);
            parent_dir = dirname(parent_dir);
            search_file_e(parent_dir, parent_dir, file, ptr, d, f, found);
        }
        else if(path[0]=='/')
        {
            search_file_e(path, path, file, ptr, d, f, found);
        }
        else if(path[0]=='~' && path[1]=='/')
        {
            char full_path[4096];
            snprintf(full_path, sizeof(full_path), "%s%s",home , path + 1);
            search_file_e(full_path, full_path, file, ptr, d, f, found);
        }
        else if(path[0]=='.' && path[1]=='/')
        {
            char cwd1[1024];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[1;31m");
                return;
            }
            char full_path[4096];
            snprintf(full_path, sizeof(full_path), "%s%s",home , path + 1);
            search_file_e(full_path, full_path, file, ptr, d, f, found);
        }
        if((*ptr)==1)
        {
            struct stat path_stat;
            if (stat(found, &path_stat) != 0) 
            {
                perror("\033[1;31mstat\033[1;31m");
                return;
            }

            if (S_ISDIR(path_stat.st_mode)) 
            {
                if (access(found, X_OK) == -1) 
                {
                    printf("\033[1;31mMissing permissions for Task\033[1;31m");
                    return;
                }
                char cwd1[1024];
                getcwd(cwd1, sizeof(cwd1));
                if(chdir(found)!=0)
                {
                    perror("\033[1;31mchdir to specified directory failed\033[1;31m");
                    return;
                }
                strcpy(pwd, cwd1);
                return;
            } 
            else if (S_ISREG(path_stat.st_mode))
            {
                if (access(found, R_OK) == -1) {
                    printf("\033[1;31mMissing permissions for Task\033[1;31m");
                    return ;
                }
                int fd;
                ssize_t bytesRead;
                char *buffer;
                size_t bufferSize = 4096;
                size_t totalBytesRead = 0;
                fd = open(found, O_RDONLY);
                if (fd == -1) {
                    perror("\033[1;31mError opening the file\033[1;31m");
                    return;
                }
                buffer = (char *)malloc(bufferSize);
                if (buffer == NULL) {
                    perror("\033[1;31mError allocating memory\033[1;31m");
                    close(fd);
                    return;
                }
                while ((bytesRead = read(fd, buffer + totalBytesRead, bufferSize - totalBytesRead)) > 0) 
                {
                    totalBytesRead += bytesRead;
                    if (totalBytesRead == bufferSize) 
                    {
                        bufferSize *= 2;
                        buffer = (char *)realloc(buffer, bufferSize);
                        if (buffer == NULL) {
                            perror("\033[1;31mError reallocating memory\033[1;31m");
                            close(fd);
                            return;
                        }
                    }
                }
                if (bytesRead == -1) {
                    perror("\033[1;31mError reading the file\033[1;31m");
                    free(buffer);
                    close(fd);
                    return;
                }
                buffer[totalBytesRead] = '\0';
                printf("%s", buffer);
                close(fd);
                free(buffer);
            }
            return;
        }
    }
}