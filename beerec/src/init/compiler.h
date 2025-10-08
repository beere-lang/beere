#ifndef COMPILER_H
#define COMPILER_H

#include <stdio.h>

// Compila todo o codigo fonte do module e seus modules importados recursivamente.
// O codigo do module é primeiro, processado pelo Front-End e depois processado
// pelo Back-End, que gera GIMPLE IR (GCC), depois é transformado em binario pela
// API do GCC e por fim, é retornado o arquivo contendo (FILE*) o executavel gerado.
FILE* compile_module(void* module, char** args);

#endif