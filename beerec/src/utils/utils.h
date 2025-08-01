#ifndef UTILS_H
#define UTILS_H

char* get_absolute_path(const char* relative_path);
char* strndup(const char* src, const size_t len);
char* resolve_path(char* abs_path, char* relative);
static int has_extension(const char* filename, const char* required);
char* get_dot_mod_relative_path(char* mod_path, const char* relative);
char* read_file(const char* file_name, int mod);
char* get_directory(char* path);

#endif
