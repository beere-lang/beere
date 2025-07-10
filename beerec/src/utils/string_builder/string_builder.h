#ifndef STRING_BUILDER_H
#define STRING_BUILDER_H

#include <stddef.h>

typedef struct
{
	char* buffer;
	size_t memory_size;
	int length;
} StringBuilder;

void init(StringBuilder* string_builder);
void free_string_builder(StringBuilder* string_builder);
void append(StringBuilder* string_builder, const char* str);

#endif