#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* strndup(const char* src, const size_t len)
{
	char* dest = malloc(len + 1);

	if (dest == NULL) 
	{
		exit(1);
	}

	memcpy(dest, src, len);
	dest[len] = '\0';

	return dest;
}