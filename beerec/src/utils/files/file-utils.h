#ifndef FILE_UTILS_H
#define FILE_UTILS_H

// Le o arquivo localizado no 'path', caso tenha algum erro, retorna 1, caso contrario, retorna 0.
int read_file(char* buff, const unsigned int buff_size, const char* path);

#endif