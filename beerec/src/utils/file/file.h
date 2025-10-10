#ifndef FILE_H
#define FILE_H

#include "../../../data/data.h"

// Modo em que o arquivo vai ser aberto no 'fopen'.
#define FILE_OPEN_MODE "rb"

// Tamanho do buffer em que o full path vai ser escrito.
#define FULL_PATH_BUFFER_SIZE 512

// Le o arquivo localizado no 'path', caso tenha algum erro, retorna 1, caso contrario, retorna 0.
i32 read_file    (str buff, const u32 buff_size, const str path);

// Checa se um path / nome de um arquivo tem a extensão 'extension', e seta o 'ref_extension' pro 
// inicio da extensão do 'name' caso 'ref_extension não seja 'NULL'.
i32 has_extension(str name, str* ref_extension, str extension);

// Linka dois paths, 'a' e 'b', com alguns checks pra deixar o path final "limpo".
// Não é a função mais segura do mundo, mas funciona se usado corretamente.
str link_paths   (str a, str b);

// Retorna o path completo de um path relativo a outro 'root'.
// Não é a função mais segura do mundo, mas funciona se usado corretamente.
str get_full_path(str root, str relative);

#endif