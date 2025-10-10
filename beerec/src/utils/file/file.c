#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "file.h"
#include "../string/string.h"

i32 has_extension(str name, str* ref_extension, str extension)
{
	if (name == NULL)
	{
		return 0;
	}

	char* start = strrchr(name, '.');

	if (start != NULL)
	{
		if (ref_extension != NULL)
		{
			*ref_extension = start;
		}
		
		if (strcmp(start, extension) == 0)
		{
			return 1;
		}
	}

	return 0;
}

i32 read_file(str buff, const u32 buff_size, const str path)
{
	FILE* file = fopen(path, FILE_OPEN_MODE);
	
	if (file == NULL)
	{
		return 1;
	}

	if (fread(buff, sizeof(char), buff_size - 1, file) == 0)
	{
		return 1;
	}

	fclose(file);
	
	buff[buff_size - 1] = '\0';

	return 0;
}

str link_paths(str a, str b)
{
	str first = NULL;
	const u32 a_length = strlen(a);

	if (a[a_length - 1] == '/' || a[a_length - 1] == '\\')
	{
		first = strndup(a, a_length - 1);
	}
	else
	{
		first = strdup(a);
	}

	str second = NULL;
	const u32 b_length = strlen(b);

	if (b[0] == '/' || b[0] == '\\')
	{
		second = strdup(b + 1);
	}
	else
	{
		second = strdup(b);
	}

	str buff = calloc(a_length + b_length + 1, sizeof(char));
	sprintf(buff, "%s/%s", first, second);

	free(first);
	free(second);

	return buff;
}

str get_full_path(str root, str relative)
{
	str buff = calloc(FULL_PATH_BUFFER_SIZE, sizeof(char));
	str path = link_paths(root, relative);

	if (!GetFullPathNameA(path, FULL_PATH_BUFFER_SIZE, buff, NULL))
	{
		free(path);
		free(buff);

		return NULL;
	}

	free(path);

	return buff;
}