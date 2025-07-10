/**
	Created by jer1337 on 13/06/2025.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "./logger/logger.h"

char* strndup(const char* src, const size_t len)
{
    char* dest = malloc(len + 1);

    if (dest == NULL) 
    {
        parser_error("Failed to allocate memory for string duplication");
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

char* get_path_from_relative(const char* directory, const char* relative_path)
{
    size_t len_dir = strlen(directory);
    size_t len_rel = strlen(relative_path);
    int need_sep = 0;

    if (len_dir == 0)
    {
        // Diretório vazio, só retorna uma cópia do relativo
        return strdup(relative_path);
    }

    // Verifica se o directory termina com barra
    char last_char = directory[len_dir - 1];
    if (last_char != '/' && last_char != '\\')
    {
        need_sep = 1;
    }

    // +1 para barra +1 para '\0' se precisar
    char* full_path = malloc(len_dir + len_rel + (need_sep ? 2 : 1));
    if (!full_path)
        return NULL;

    strcpy(full_path, directory);

    if (need_sep)
        strcat(full_path, "\\");

    strcat(full_path, relative_path);

    return full_path;
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

static int has_extension(const char* filename) 
{
	const char* extension = strrchr(filename, '.');

	if (extension == NULL || extension == filename)
	{
		return 0;
	}

	return strcmp(extension, ".beere") == 0;
}

char* read_file(const char* file_name)
{
	if (!has_extension(file_name)) 
	{
		printf("[File Reader] [Error] Invalid file extension...\n");

		return NULL;
	}

	FILE* file = fopen(file_name, "rb");

	if (!file)
	{
		printf("[File Reader] Failed to get input file: %s\n", file_name);

		return NULL;
	}

	fseek(file, 0, SEEK_END);

	const long size = ftell(file);

	rewind(file);

	char* buffer = malloc(size + 1);

	if (!buffer)
	{
		printf("[File Reader] Failed to allocate memory.\n");
		fclose(file);

		return NULL;
	}

	const size_t bytes_read = fread(buffer, 1, size, file);

	if (bytes_read != size)
	{
		printf("[File Reader] Failed to read all bytes from file.\n");
		fclose(file);

		return NULL;
	}

	buffer[size] = '\0';

	return buffer;
}