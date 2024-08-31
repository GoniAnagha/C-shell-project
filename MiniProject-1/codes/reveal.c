#include "reveal.h"
#include "commands.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libgen.h>
#include <grp.h>
#include <time.h>
#include <pwd.h>

int compare(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

void execute_only_reveal() {
    struct dirent *de; 
    char cwd1[1024];
    if (getcwd(cwd1, sizeof(cwd1)) == NULL) {
        perror("\033[1;31mgetcwd\033[0m");
        return;
    }
    DIR *dr = opendir(cwd1);
    if (dr == NULL)  
    {
        printf("\033[1;31merror opening direcotry\033[0m\n");
        return;
    }
    char **filenames = NULL;
    size_t count = 0;
    
    while ((de = readdir(dr)) != NULL) {
        if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0 && de->d_name[0]!='.') {
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
        snprintf(full_path, sizeof(full_path), "%s/%s", cwd1, filenames[i]);
        if (stat(full_path, &st) == 0)
        {
            if (S_ISDIR(st.st_mode)) 
            {
                printf("\033[34m%s\033[0m\n", filenames[i]); 
            }
            else if (st.st_mode & S_IXUSR)
            {
                printf("\033[32m%s\033[0m\n", filenames[i]); 
            }
            else
            {
                printf("\033[37m%s\033[0m\n", filenames[i]); 
            }
        } 
        else 
        {
            printf("\033[1;31merror in cheking status of the file\\033[0mn");
        }

        free(filenames[i]);
    }
    free(filenames); 
}


void execute_reveal(char* token1, char* pwd, char* home)
{
    struct dirent *de;
    DIR *dr ;
        char directory[4096];
        if(strcmp(token1, ".")==0)
        {
            char cwd1[1024];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[0m");
                return;
            }
            dr = opendir(cwd1);
            strcpy(directory, cwd1);
        }
        else if(strcmp(token1, "-")==0)
        {
            dr = opendir(pwd);
            strcpy(directory, pwd);
        }
        else if(strcmp(token1, "..")==0)
        {
            char cwd1[1024];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[0m");
                return;
            }
            char *parent_dir = strdup(cwd1);
            parent_dir = dirname(parent_dir);
            dr = opendir(parent_dir);
            strcpy(directory, parent_dir);
        }
        else if(strcmp(token1, "~")==0)
        {
            dr = opendir(home);
            strcpy(directory, home);
        }
        else if(token1[0]=='~' && token1[1]=='/')
        {
            char full_path[4096];
            snprintf(full_path, sizeof(full_path), "%s%s",home , token1 + 1);
            dr = opendir(full_path);
            strcpy(directory, full_path);
        }
        else if(token1[0]=='.' && token1[1]=='/')
        {
            char cwd1[1024];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[0m");
                return;
            }
            char full_path[4096];
            snprintf(full_path, sizeof(full_path), "%s%s",cwd1 , token1 + 1);
            dr = opendir(full_path);
            strcpy(directory, full_path);
        }
        else if(token1[0]=='/')
        {
            dr = opendir(token1);
            strcpy(directory, token1);
        }
        else if(strcmp(token1, "../..")==0)
        {
            char cwd1[4096];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[0m");
                return;
            }
            char *parent_dir = strdup(cwd1);
            parent_dir = dirname(parent_dir);
            char* parent_to_parent = strdup(parent_dir);
            parent_to_parent = dirname(parent_to_parent);
            dr = opendir(parent_to_parent);
            strcpy(directory, parent_to_parent);
        }
        else
        {
            char current_dir[4096];
            getcwd(current_dir, sizeof(current_dir));
            struct stat path_stat;
            char full_path[4097];
            snprintf(full_path, sizeof(full_path), "%s/%s", current_dir, token1);
            if (stat(full_path, &path_stat) == 0)
            {
               if (S_ISREG(path_stat.st_mode)) 
               {
                    if (path_stat.st_mode & S_IXUSR)
                    {
                        printf("\033[32m%s\033[0m\n", token1); 
                        return;
                    }
                    printf("%s\n", token1);
                    return;
               } 
               else if (S_ISDIR(path_stat.st_mode)) 
               {
                    strcpy(directory, full_path);
                    dr = opendir(directory);
               } 
               else 
               {
                   printf("\033[1;31mits is neither a regular file nor a directory.\033[0m\n");
               }
            } 
            else 
            {
                printf("\033[1;31merror\033[0m\n");
                return;
            }
        }
        if (dr == NULL)  
        {
            printf("\033[1;31merror opening direcotry\033[0m");
            return;
        }
        char **filenames = NULL;
        size_t count = 0;
        
        while ((de = readdir(dr)) != NULL) {
            if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0 && de->d_name[0]!='.') {
                filenames = realloc(filenames, sizeof(char*) * (count + 1));
                filenames[count] = strdup(de->d_name);
                count++;
            }
        }
        closedir(dr);
        qsort(filenames, count, sizeof(char*), compare);
        for (size_t i = 0; i < count; i++) {
        struct stat st;
        char full_path[4097];
        snprintf(full_path, sizeof(full_path), "%s/%s", directory, filenames[i]);
        if (stat(full_path, &st) == 0)
        {
            if (S_ISDIR(st.st_mode)) 
            {
                printf("\033[34m%s\033[0m\n", filenames[i]); 
            }
            else if (st.st_mode & S_IXUSR)
            {
                printf("\033[32m%s\033[0m\n", filenames[i]); 
            }
            else
            {
                printf("\033[37m%s\033[0m\n", filenames[i]); 
            }
        } 
        else 
        {
            printf("\033[1;31merror\033[0m\n");
        }

        free(filenames[i]);
    }
    free(filenames); 
    
}

void execute_reveal_flags(char* token1, int find_a, int find_l, char* pwd, char* home)
{
    if(find_l && find_a)
    {
        struct dirent *de;
        DIR *dr;
        char directory[4096];
        if(strcmp(token1, ".")==0)
        {
            char cwd1[1024];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[0m");
                return;
            }
            dr = opendir(cwd1);
            strcpy(directory, cwd1);
        }
        else if(strcmp(token1, "-")==0)
        {
            dr = opendir(pwd);
            strcpy(directory, pwd);
        }
        else if(strcmp(token1, "..")==0)
        {
            char cwd1[1024];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[0m");
                return;
            }
            char *parent_dir = strdup(cwd1);
            parent_dir = dirname(parent_dir);
            dr = opendir(parent_dir);
            strcpy(directory, parent_dir);
        }
        else if(strcmp(token1, "~")==0)
        {
            dr = opendir(home);
            strcpy(directory, home);
        }
        else if(token1[0]=='~' && token1[1]=='/')
        {
            char full_path[4096];
            snprintf(full_path, sizeof(full_path), "%s%s",home , token1 + 1);
            dr = opendir(full_path);
            strcpy(directory, full_path);
        }
        else if(token1[0]=='/')
        {
            dr = opendir(token1);
            strcpy(directory, token1);
        }
        else if(strcmp(token1, "../..")==0)
        {
            char cwd1[4096];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[0m");
                return;
            }
            char *parent_dir = strdup(cwd1);
            parent_dir = dirname(parent_dir);
            char* parent_to_parent = strdup(parent_dir);
            parent_to_parent = dirname(parent_to_parent);
            dr = opendir(parent_to_parent);
            strcpy(directory, parent_to_parent);
        }
        else
        {
            char current_dir[4096];
            getcwd(current_dir, sizeof(current_dir));
            struct stat path_stat;
            char full_path[4097];
            snprintf(full_path, sizeof(full_path), "%s/%s", current_dir, token1);
            if (stat(full_path, &path_stat) == 0)
            {
               if (S_ISREG(path_stat.st_mode)) 
               {
                    struct stat fileStat;
                    if (stat(full_path, &fileStat) < 0) 
                    {
                        perror("\033[1;31mstat\033[0m");
                        return; 
                    }
                    printf((S_ISDIR(fileStat.st_mode)) ? "d" : "-");
                    printf((fileStat.st_mode & S_IRUSR) ? "r" : "-");
                    printf((fileStat.st_mode & S_IWUSR) ? "w" : "-");
                    printf((fileStat.st_mode & S_IXUSR) ? "x" : "-");
                    printf((fileStat.st_mode & S_IRGRP) ? "r" : "-");
                    printf((fileStat.st_mode & S_IWGRP) ? "w" : "-");
                    printf((fileStat.st_mode & S_IXGRP) ? "x" : "-");
                    printf((fileStat.st_mode & S_IROTH) ? "r" : "-");
                    printf((fileStat.st_mode & S_IWOTH) ? "w" : "-");
                    printf((fileStat.st_mode & S_IXOTH) ? "x" : "-");
                    printf(" ");
                    
                    printf("%4ld ", fileStat.st_nlink);
                    struct passwd *pwd1 = getpwuid(fileStat.st_uid);
                    struct group *grp = getgrgid(fileStat.st_gid);
                    printf("%-8s %-8s ", pwd1->pw_name, grp->gr_name);
                    printf("%8ld ", fileStat.st_size);
                    
                    char timeBuf[80];
                    struct tm *tm_info = localtime(&fileStat.st_mtime);
                    strftime(timeBuf, sizeof(timeBuf), "%b %d %H:%M", tm_info);
                    printf("%s ", timeBuf);
                    if (S_ISDIR(fileStat.st_mode)) 
                    {
                        printf("\033[34m%s\033[0m\n", token1); 
                    }
                    else if (fileStat.st_mode & S_IXUSR)
                    {
                        printf("\033[32m%s\033[0m\n", token1); 
                    }
                    else
                    {
                        printf("\033[37m%s\033[0m\n", token1); 
                    }

                    return;
               } 
               else if (S_ISDIR(path_stat.st_mode)) 
               {
                    strcpy(directory, full_path);
                    dr = opendir(directory);
               } 
               else 
               {
                   printf("\033[1;31mits is neither a regular file nor a directory.\033[0m\n");
               }
            } 
            else 
            {
                printf("\033[1;31merror in cheking status of the file\033[0m\n");
            }
        }
        if (dr == NULL)  
        {
            printf("\033[1;31merror opening direcotry\033[0m");
            return;
        }
        char **filenames = NULL;
        size_t count = 0;
        while ((de = readdir(dr)) != NULL)
        {
            filenames = realloc(filenames, sizeof(char*) * (count + 1));
            filenames[count] = strdup(de->d_name);
            count++;
        }
        closedir(dr);
        qsort(filenames, count, sizeof(char*), compare);
        for (size_t i = 0; i < count; i++) 
        {
            struct stat fileStat;
            char full_path[4097];
            snprintf(full_path, sizeof(full_path), "%s/%s", directory, filenames[i]);
            if (stat(full_path, &fileStat) < 0) 
            {
                perror("\033[1;31mstat\033[0m");
                continue;
            }
            printf((S_ISDIR(fileStat.st_mode)) ? "d" : "-");
            printf((fileStat.st_mode & S_IRUSR) ? "r" : "-");
            printf((fileStat.st_mode & S_IWUSR) ? "w" : "-");
            printf((fileStat.st_mode & S_IXUSR) ? "x" : "-");
            printf((fileStat.st_mode & S_IRGRP) ? "r" : "-");
            printf((fileStat.st_mode & S_IWGRP) ? "w" : "-");
            printf((fileStat.st_mode & S_IXGRP) ? "x" : "-");
            printf((fileStat.st_mode & S_IROTH) ? "r" : "-");
            printf((fileStat.st_mode & S_IWOTH) ? "w" : "-");
            printf((fileStat.st_mode & S_IXOTH) ? "x" : "-");
            printf(" ");
            
            printf("%4ld ", fileStat.st_nlink);
            struct passwd *pwd1 = getpwuid(fileStat.st_uid);
            struct group *grp = getgrgid(fileStat.st_gid);
            printf("%-8s %-8s ", pwd1->pw_name, grp->gr_name);
            printf("%8ld ", fileStat.st_size);
            
            char timeBuf[80];
            struct tm *tm_info = localtime(&fileStat.st_mtime);
            strftime(timeBuf, sizeof(timeBuf), "%b %d %H:%M", tm_info);
            printf("%s ", timeBuf);
            if (S_ISDIR(fileStat.st_mode)) 
            {
                printf("\033[34m%s\033[0m\n", filenames[i]); 
            }
            else if (fileStat.st_mode & S_IXUSR)
            {
                printf("\033[32m%s\033[0m\n", filenames[i]); 
            }
            else
            {
                printf("\033[37m%s\033[0m\n", filenames[i]); 
            }

            free(filenames[i]);
        }
        free(filenames);
        return;
    }
    else if(find_l)
    {
        struct dirent *de;
        DIR *dr;
        char directory[4096];
        if(strcmp(token1, ".")==0)
        {
            char cwd1[1024];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[0m");
                return;
            }
            dr = opendir(cwd1);
            strcpy(directory, cwd1);
        }
        else if(strcmp(token1, "-")==0)
        {
            dr = opendir(pwd);
            strcpy(directory, pwd);
        }
        else if(strcmp(token1, "..")==0)
        {
            char cwd1[1024];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[0m");
                return;
            }
            char *parent_dir = strdup(cwd1);
            parent_dir = dirname(parent_dir);
            dr = opendir(parent_dir);
            strcpy(directory, parent_dir);
            printf("%s\n", parent_dir);
        }
        else if(strcmp(token1, "~")==0)
        {
            dr = opendir(home);
            strcpy(directory, home);
        }
        else if(token1[0]=='~' && token1[1]=='/')
        {
            char full_path[4096];
            snprintf(full_path, sizeof(full_path), "%s%s",home , token1 + 1);
            dr = opendir(full_path);
            strcpy(directory, full_path);
        }
        else if(token1[0]=='/')
        {
            dr = opendir(token1);
            strcpy(directory, token1);
        }
        else if(strcmp(token1, "../..")==0)
        {
            char cwd1[4096];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[0m");
                return;
            }
            char *parent_dir = strdup(cwd1);
            parent_dir = dirname(parent_dir);
            char* parent_to_parent = strdup(parent_dir);
            parent_to_parent = dirname(parent_to_parent);
            dr = opendir(parent_to_parent);
            strcpy(directory, parent_to_parent);
        }
        else
        {
            char current_dir[4096];
            getcwd(current_dir, sizeof(current_dir));
            struct stat path_stat;
            char full_path[4097];
            snprintf(full_path, sizeof(full_path), "%s/%s", current_dir, token1);
            if (stat(full_path, &path_stat) == 0)
            {
               if (S_ISREG(path_stat.st_mode)) 
               {
                    struct stat fileStat;
                    if (stat(full_path, &fileStat) < 0) 
                    {
                        perror("\033[1;31mstat\033[0m");
                        return; 
                    }
                    printf((S_ISDIR(fileStat.st_mode)) ? "d" : "-");
                    printf((fileStat.st_mode & S_IRUSR) ? "r" : "-");
                    printf((fileStat.st_mode & S_IWUSR) ? "w" : "-");
                    printf((fileStat.st_mode & S_IXUSR) ? "x" : "-");
                    printf((fileStat.st_mode & S_IRGRP) ? "r" : "-");
                    printf((fileStat.st_mode & S_IWGRP) ? "w" : "-");
                    printf((fileStat.st_mode & S_IXGRP) ? "x" : "-");
                    printf((fileStat.st_mode & S_IROTH) ? "r" : "-");
                    printf((fileStat.st_mode & S_IWOTH) ? "w" : "-");
                    printf((fileStat.st_mode & S_IXOTH) ? "x" : "-");
                    printf(" ");
                    
                    printf("%4ld ", fileStat.st_nlink);
                    struct passwd *pwd1 = getpwuid(fileStat.st_uid);
                    struct group *grp = getgrgid(fileStat.st_gid);
                    printf("%-8s %-8s ", pwd1->pw_name, grp->gr_name);
                    printf("%8ld ", fileStat.st_size);
                    
                    char timeBuf[80];
                    struct tm *tm_info = localtime(&fileStat.st_mtime);
                    strftime(timeBuf, sizeof(timeBuf), "%b %d %H:%M", tm_info);
                    printf("%s ", timeBuf);
                    if (S_ISDIR(fileStat.st_mode)) 
                    {
                        printf("\033[34m%s\033[0m\n", token1); 
                    }
                    else if (fileStat.st_mode & S_IXUSR)
                    {
                        printf("\033[32m%s\033[0m\n", token1); 
                    }
                    else
                    {
                        printf("\033[37m%s\033[0m\n", token1); 
                    }

                    return;
               } 
               else if (S_ISDIR(path_stat.st_mode)) 
               {
                    strcpy(directory, full_path);
                    dr = opendir(directory);
               } 
            } 
            else 
            {
                printf("\033[1;31merror in cheking status of the file\033[0m\n");
            }
        }
        if (dr == NULL)  
        {
            printf("\033[1;31merror opening direcotry\033[0m");
            return;
        }
        char **filenames = NULL;
        size_t count = 0;
        while ((de = readdir(dr)) != NULL)
        {
            if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0 && de->d_name[0]!='.')
            {
                filenames = realloc(filenames, sizeof(char*) * (count + 1));
                filenames[count] = strdup(de->d_name);
                count++;
            }
        }
        closedir(dr);
        qsort(filenames, count, sizeof(char*), compare);
        for (size_t i = 0; i < count; i++) 
        {
            struct stat fileStat;
            char full_path[4097];
            snprintf(full_path, sizeof(full_path), "%s/%s", directory, filenames[i]);

            if (stat(full_path, &fileStat) < 0) 
            {
                perror("\033[1;31mstat\033[0m");
                continue; 
            }
            printf((S_ISDIR(fileStat.st_mode)) ? "d" : "-");
            printf((fileStat.st_mode & S_IRUSR) ? "r" : "-");
            printf((fileStat.st_mode & S_IWUSR) ? "w" : "-");
            printf((fileStat.st_mode & S_IXUSR) ? "x" : "-");
            printf((fileStat.st_mode & S_IRGRP) ? "r" : "-");
            printf((fileStat.st_mode & S_IWGRP) ? "w" : "-");
            printf((fileStat.st_mode & S_IXGRP) ? "x" : "-");
            printf((fileStat.st_mode & S_IROTH) ? "r" : "-");
            printf((fileStat.st_mode & S_IWOTH) ? "w" : "-");
            printf((fileStat.st_mode & S_IXOTH) ? "x" : "-");
            printf(" ");
            
            printf("%4ld ", fileStat.st_nlink);
            struct passwd *pwd1 = getpwuid(fileStat.st_uid);
            struct group *grp = getgrgid(fileStat.st_gid);
            printf("%-8s %-8s ", pwd1->pw_name, grp->gr_name);
            printf("%8ld ", fileStat.st_size);
            
            char timeBuf[80];
            struct tm *tm_info = localtime(&fileStat.st_mtime);
            strftime(timeBuf, sizeof(timeBuf), "%b %d %H:%M", tm_info);
            printf("%s ", timeBuf);
            if (S_ISDIR(fileStat.st_mode)) 
            {
                printf("\033[34m%s\033[0m\n", filenames[i]); 
            }
            else if (fileStat.st_mode & S_IXUSR)
            {
                printf("\033[32m%s\033[0m\n", filenames[i]); 
            }
            else
            {
                printf("\033[37m%s\033[0m\n", filenames[i]); 
            }

            free(filenames[i]);
        }
        free(filenames);
    }
    else if(find_a)
    {
        struct dirent *de;
        DIR *dr;
        char directory[4096];
        if(strcmp(token1, ".")==0)
        {
            char cwd1[1024];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[0m");
                return;
            }
            dr = opendir(cwd1);
            strcpy(directory, cwd1);
        }
        else if(strcmp(token1, "-")==0)
        {
            dr = opendir(pwd);
            strcpy(directory, pwd);
        }
        else if(strcmp(token1, "..")==0)
        {
            char cwd1[1024];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[0m");
                return;
            }
            char *parent_dir = strdup(cwd1);
            parent_dir = dirname(parent_dir);
            dr = opendir(parent_dir);
            strcpy(directory, parent_dir);
            printf("%s\n", parent_dir);
        }
        else if(strcmp(token1, "~")==0)
        {
            dr = opendir(home);
            strcpy(directory, home);
        }
        else if(token1[0]=='~' && token1[1]=='/')
        {
            char full_path[4096];
            snprintf(full_path, sizeof(full_path), "%s%s",home , token1 + 1);
            dr = opendir(full_path);
            strcpy(directory, full_path);
        }
        else if(token1[0]=='/')
        {
            dr = opendir(token1);
            strcpy(directory, token1);
        }
        else if(strcmp(token1, "../..")==0)
        {
            char cwd1[4096];
            if (getcwd(cwd1, sizeof(cwd1)) == NULL) 
            {
                perror("\033[1;31mgetcwd\033[0m");
                return;
            }
            char *parent_dir = strdup(cwd1);
            parent_dir = dirname(parent_dir);
            char* parent_to_parent = strdup(parent_dir);
            parent_to_parent = dirname(parent_to_parent);
            dr = opendir(parent_to_parent);
            strcpy(directory, parent_to_parent);
        }
        else
        {
            char current_dir[4096];
            getcwd(current_dir, sizeof(current_dir));
            struct stat path_stat;
            char full_path[4097];
            snprintf(full_path, sizeof(full_path), "%s/%s", current_dir, token1);
            if (stat(full_path, &path_stat) == 0)
            {
               if (S_ISREG(path_stat.st_mode)) 
               {
                    printf("%s\n", token1);
                    return;
               } 
               else if (S_ISDIR(path_stat.st_mode)) 
               {
                    strcpy(directory, full_path);
                    dr = opendir(directory);
               } 
               else 
               {
                   printf("\033[1;31mits is neither a regular file nor a directory.\033[0m\n");
               }
            } 
            else 
            {
                printf("\033[1;31merror in cheking status of the file\033[0m\n");
            }
        }
        if (dr == NULL)  
        {
            printf("\033[1;31merror opening direcotry\033[0m");
            return;
        }
        char **filenames = NULL;
        size_t count = 0;
        while ((de = readdir(dr)) != NULL)
        {
            filenames = realloc(filenames, sizeof(char*) * (count + 1));
            filenames[count] = strdup(de->d_name);
            count++;
        }
        closedir(dr);
        qsort(filenames, count, sizeof(char*), compare);
        for (size_t i = 0; i < count; i++) 
        {
            struct stat st;
            char full_path[4097];
            snprintf(full_path, sizeof(full_path), "%s/%s", directory, filenames[i]);
            if (stat(full_path, &st) == 0)
            {
                if (S_ISDIR(st.st_mode)) 
                {
                    printf("\033[34m%s\033[0m\n", filenames[i]); 
                }
                else if (st.st_mode & S_IXUSR)
                {
                    printf("\033[32m%s\033[0m\n", filenames[i]); 
                }
                else
                {
                    printf("\033[37m%s\033[0m\n", filenames[i]); 
                }
            } 
            else 
            {
                printf("\033[1;31merror in cheking status of the file\033[0m\n");
            }
            free(filenames[i]);
        }
        free(filenames); 
    }
}