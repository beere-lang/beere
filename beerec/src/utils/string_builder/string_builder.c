#include "string_builder.h"
#include <stdlib.h>
#include <string.h>

void init(StringBuilder* string_builder) 
{
	string_builder->memory_size = 64;
	string_builder->length = 0;
	
	string_builder->buffer = malloc(string_builder->memory_size);

	if (string_builder->buffer) 
	{
		string_builder->buffer[0] = '\0';
	}
}

void append(StringBuilder* string_builder, const char* str) 
{
	const int size = (int) strlen(str);

	if (string_builder->length + size + 1 > string_builder->memory_size)
	{
		while (string_builder->length + size + 1 > string_builder->memory_size) 
		{
			string_builder->memory_size *= 2;
		}

		string_builder->buffer = realloc(string_builder->buffer, string_builder->memory_size);
	}

	strcpy(string_builder->buffer + string_builder->length, str);
	string_builder->length += size;
}

void free_string_builder(StringBuilder* string_builder) 
{
	free(string_builder->buffer);
	string_builder->buffer = NULL;
	string_builder->length = 0;
	string_builder->memory_size = 64;
}