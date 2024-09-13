#ifndef log_h
#define log_h
#include <stdio.h>

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

void store_log(char* command, char* home);
void execute_log(char* token1, char* token2, char* home, char* pwd,int* ptr, char* dummy1) ;
void purge_log(char* home);
void print_log(char* home);

#endif