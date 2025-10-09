#include <stdlib.h>

#include "compiler.h"
#include "../utils/files/file-utils.h"
// ==---------------------------------- Core --------------------------------------== \\

FILE* compile_module(void* module, str* args, const str path)
{
	str buff = malloc(FILE_READ_BUFFER_SIZE);
	const i32 error = read_file(buff, FILE_READ_BUFFER_SIZE, path);

	if (error)
	{
		return NULL;
	}

	return NULL;
}

// ==--------------------------------- Utils --------------------------------------== \\



// ==---------------------------- Memory Management -------------------------------== \\

