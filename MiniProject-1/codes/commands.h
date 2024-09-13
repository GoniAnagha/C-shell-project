#ifndef commands_h
#define commands_h
#include <sys/types.h>

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

void run_background(char* command, int* store_background, int* idx, char* cwd, int* ptr1);
void run_foreground(char* command,char* cwd, char* pwd, int* ptr, char* dummy1);
void check_print_status();
int bring_to_foreground(pid_t pid);
void execute_activities();
void fetch_manpage(char* manPage);

#endif