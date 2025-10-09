#include <stdlib.h>

#include "compiler.h"
#include "../utils/files/file-utils.h"
#include "../utils/logger/logger.h"

// ==---------------------------------- Core --------------------------------------== \\

FILE* compile_module(void* module, str* args, const str path)
{
	str buff = malloc(FILE_READ_BUFFER_SIZE);
	const i32 error = read_file(buff, FILE_READ_BUFFER_SIZE, path);

	if (error)
	{
		log_error("Failed to read module source code from path: %s", path);
		return NULL;
	}

	return NULL;
}

// ==--------------------------------- Utils --------------------------------------== \\



// ==---------------------------- Memory Management -------------------------------== \\

