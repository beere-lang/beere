#ifndef FILE_H
#define FILE_H

#include "../../../data/data.h"

// Modo em que o arquivo vai ser aberto no 'fopen'.
#define FILE_OPEN_MODE "rb"

// Le o arquivo localizado no 'path', caso tenha algum erro, retorna 1, caso contrario, retorna 0.
i32 read_file(str buff, const u32 buff_size, const str path);

// Checa se um path / nome de um arquivo tem a extensão 'extension', e seta o 'ref_extension' pro 
// inicio da extensão do 'name' caso 'ref_extension não seja 'NULL'.
i32 has_extension(str name, str* ref_extension, str extension);

#endif