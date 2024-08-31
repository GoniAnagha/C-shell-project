#ifndef input_h
#define input_h
#include<stdio.h>

extern int store_background[4096];
extern int idx;
extern char dummy2[4096];

void execute_command(char* tok1, char* cwd, char* pwd, int* ptr, char* dummy1);
int check_token_path(char* token);
void search_a(char* token, int* ptr);
void search_l(char* token, int* ptr);
int check_semicolon(char* comm);
int check_and(char* comm);
int count_ampersands(const char* str);
void sigchld_handler(int sig);

#endif