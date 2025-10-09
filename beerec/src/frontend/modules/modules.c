#include <stdlib.h>

#include "modules.h"
#include "../../utils/files/file-utils.h"
#include "../../utils/logger/logger.h"

// ==---------------------------------- Core --------------------------------------== \\

ModuleConfig* handle_module_config(const str path)
{
	str buff = malloc(MODULE_FILE_READ_BUFFER_SIZE);
	const i32 error = read_file(buff, MODULE_FILE_READ_BUFFER_SIZE, path);

	if (error)
	{
		log_error("Failed to read module config from path: %s", path);
		return NULL;
	}

	ModuleConfig* mod_cfg = malloc(sizeof(ModuleConfig));

	return NULL;
}

// ==--------------------------------- Utils --------------------------------------== \\