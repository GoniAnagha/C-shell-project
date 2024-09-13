#ifndef PIPES_H
#define PIPES_H

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

void handle_pipes(char* command, char* cwd, char* pwd, int* ptr, char* dummy1);
int check_invalid(char* command);
void trim_spaces(char *str);

#endif