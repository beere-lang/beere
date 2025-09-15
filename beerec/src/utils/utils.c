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

char* get_directory(char* path)
{
	char* copy = strdup(path);

	if (copy == NULL)
	{
		return NULL;
	}

	char* last_slash = strrchr(copy, '/');

	if (last_slash != NULL)
	{
		*(last_slash + 1) = '\0';
	}
	else
	{
		copy[0] = '\0';
	}

	return copy;
}

char* resolve_path(char* abs_path, char* relative)
{
	char* buffer = malloc(2048);

	snprintf(buffer, 2048, "%s/%s", abs_path, relative);

	if (GetFullPathNameA(buffer, 2048, buffer, NULL) == 0) 
	{
		return NULL;
	}

	return buffer;
}

char* get_dot_mod_relative_path(char* mod_path, const char* relative)
{
	char* buffer = malloc(2048);

	char temp[2048];
	strcpy(temp, mod_path);

	char* directory = get_directory(temp);

	char* format = "%s%s";

	if (directory[strlen(directory) - 1] != '/')
	{
		format = "%s/%s";
	}

	snprintf(buffer, 2048, format, directory, relative);

	if (GetFullPathNameA(buffer, 2048, buffer, NULL) == 0) 
	{
		return NULL;
	}

	return buffer;
}

char* get_absolute_path(const char* relative_path)
{
	char* abs_path = malloc(_MAX_PATH);
	
	if (!_fullpath(abs_path, relative_path, _MAX_PATH))
	{
		free(abs_path);
		return NULL;
	}

	return abs_path;
}

static int has_extension(const char* filename, const char* required) 
{
	const char* extension = strrchr(filename, '.');

	if (extension == NULL || extension == filename)
	{
		return 0;
	}

	return strcmp(extension, required) == 0;
}

char* read_file(const char* file_path, int mod)
{
	if (!has_extension(file_path, (mod) ? ".mod" : ".beere")) 
	{
		printf("[FileReader] [Error] Invalid file extension...\n");

		return NULL;
	}

	FILE* file = fopen(file_path, "rb");

	if (!file)
	{
		printf("[FileReader] Failed to get input file: %s\n", file_path);

		return NULL;
	}

	fseek(file, 0, SEEK_END);

	const long size = ftell(file);

	rewind(file);

	char* buffer = malloc(size + 1);

	if (!buffer)
	{
		printf("[FileReader] Failed to allocate memory.\n");
		fclose(file);

		return NULL;
	}

	const size_t bytes_read = fread(buffer, 1, size, file);

	if (bytes_read != size)
	{
		printf("[FileReader] Failed to read all bytes from file.\n");
		fclose(file);

		return NULL;
	}

	buffer[size] = '\0';

	return buffer;
}