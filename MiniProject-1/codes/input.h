#ifndef input_h
#define input_h
#include<stdio.h>

extern int store_background[4096];
extern char background_commands[256][4096];
extern int store_foreground[4096];
extern char foreground_commands[256][4096];
extern int status_foreground[256];
extern int status_background[256];
extern int foreground_running;
extern int idx;
extern int idx2;
extern int idx3;
extern int idx4;
extern char dummy2[4096];
extern int back;

void execute_command(char* tok1, char* cwd, char* pwd, int* ptr, char* dummy1);
int check_token_path(char* token);
void search_a(char* token, int* ptr);
void search_l(char* token, int* ptr);
int check_semicolon(char* comm);
int check_and(char* comm);
int count_ampersands(const char* str);

#endif