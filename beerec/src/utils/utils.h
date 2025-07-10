#ifndef UTILS_H
#define UTILS_H

char* get_path_from_relative(char* dir, char* path);
char* get_absolute_path(const char* relative_path);
char* strndup(const char* src, const size_t len);
static int has_extension(const char* filename);
char* read_file(const char* file_name);
char* get_directory(char* path);

#endif //UTILS_H
