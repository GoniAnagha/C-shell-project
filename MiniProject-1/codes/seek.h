#ifndef seek_h
#define seek_h

extern int store_background[4096];
extern int idx;
extern char dummy2[4096];
int has_file_extension(char *filename);
void execute_seek_cwd(char* token, int* ptr);
void execute_seek_path(char* file, char* target, char* pwd, char* home, int* ptr);
void execute_seek(char* path, char* file, int d, int e, int f, char* pwd, char* home, int* ptr);
void search_file(const char *folder, const char *base_folder, const char *filename, int* ptr);
void search_only_folders(const char *folder, const char *base_folder, const char *filename, int* ptr);
void search_only_files(const char *folder, const char *base_folder, const char *filename, int* ptr);
void search_file_ed(const char *folder, const char *base_folder, const char *filename, int* ptr, int d, int f, char* found);
void search_file_ef(const char *folder, const char *base_folder, const char *filename, int *ptr, int d, int f, char *found);

#endif