#include <windows.h>

#include "compiler.h"
#include "../utils/files/file-utils.h"
// ==---------------------------------- Core --------------------------------------== \\

FILE* compile_module(void* module, char** args, const char* path)
{
	char* buff = malloc(FILE_READ_BUFFER_SIZE);
	const int error = read_file(buff, FILE_READ_BUFFER_SIZE, path);

	if (error)
	{
		return NULL;
	}

	return NULL;
}

// ==--------------------------------- Utils --------------------------------------== \\



// ==---------------------------- Memory Management -------------------------------== \\

