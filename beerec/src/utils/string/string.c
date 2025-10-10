#include <stdlib.h>

#include "string.h"

str strndup(char* start, u32 length)
{
	str copy = malloc(length + 1);

	for (u32 i = 0; i < length; i++)
	{
		copy[i] = start[i];
	}

	copy[length] = '\0';

	return copy;
}