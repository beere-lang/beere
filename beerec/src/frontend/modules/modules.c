#include <windows.h>

#include "modules.h"
#include "../../utils/files/file-utils.h"

// ==---------------------------------- Core --------------------------------------== \\

ModuleConfig* handle_module_config(const char* path)
{
	char* buff = malloc(MODULE_FILE_READ_BUFFER_SIZE);
	const int error = read_file(buff, MODULE_FILE_READ_BUFFER_SIZE, path);

	if (error)
	{
		return NULL;
	}

	return NULL;
}

// ==--------------------------------- Utils --------------------------------------== \\