#ifndef hop_h
#define hop_h

extern int store_background[4096];
extern int idx;
extern char dummy2[4096];

void execute_hop(char* tok1, char* tok2, char* pwd, char* home);

void execute_only_hop(char* pwd, char* home);
#endif