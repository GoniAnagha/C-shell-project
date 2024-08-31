#ifndef log_h
#define log_h
#include <stdio.h>

extern int store_background[4096];
extern int idx;
extern char dummy2[4096];

void store_log(char* command, char* home);
void execute_log(char* token1, char* token2, char* home, char* pwd,int* ptr, char* dummy1) ;
void purge_log(char* home);
void print_log(char* home);

#endif