#include <stdlib.h>

#include "compiler.h"
#include "../utils/file/file.h"
#include "../utils/logger/logger.h"
#include "../frontend/lexer/lexer.h"

// ==---------------------------------- Core --------------------------------------== \\

Module* compile_module(ModuleConfig* cfg, str* args, const str path)
{
	str buff = malloc(FILE_READ_BUFFER_SIZE);
	const i32 error = read_file(buff, FILE_READ_BUFFER_SIZE, path);

	if (error)
	{
		log_error("Failed to read module source code from path: %s", path);
		exit(1);
	}

	if (cfg == NULL)
	{
		log_error("Invalid dotmod structure is 'NULL'...");
		exit(1);
	}

	Module* module = calloc(1, sizeof(Module));
	module->cfg = cfg;
	module->imports = create_list(8);

	Lexer* lexer = tokenize_module(module, FILE_READ_BUFFER_SIZE, buff);

	return module;
}

// ==--------------------------------- Utils --------------------------------------== \\


