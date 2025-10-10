#include <string.h>
#include <stdio.h>

#include "file.h"

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