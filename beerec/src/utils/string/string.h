#ifndef STRING_H
#define STRING_H

#include "../../../data/data.h"

// Retorna uma copia alocada na heap começando a copiar de um endereço 'start' até 'start + length'.
str strndup(char* start, u32 length);

#endif