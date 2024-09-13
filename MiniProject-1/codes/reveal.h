#ifndef reveal_h
#define reveal_h

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

void execute_reveal(char* token1, char* pwd, char* home);
void execute_only_reveal();
int compare(const void *a, const void *b);
void execute_reveal_flags(char* token, int find_a, int find_l, char* pwd, char* home);
#endif