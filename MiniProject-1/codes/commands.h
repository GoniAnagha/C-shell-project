#ifndef commands_h
#define commands_h

extern int store_background[4096];
extern int idx;
extern char dummy2[4096];

void run_background(char* command, int* store_background, int* idx);
void run_foreground(char* command,char* cwd, char* pwd, int* ptr, char* dummy1);
void check_print_status();

#endif