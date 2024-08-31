#ifndef reveal_h
#define reveal_h

extern int store_background[4096];
extern int idx;
extern char dummy2[4096];

void execute_reveal(char* token1, char* pwd, char* home);
void execute_only_reveal();
int compare(const void *a, const void *b);
void execute_reveal_flags(char* token, int find_a, int find_l, char* pwd, char* home);
#endif