#ifndef MODULE_H
#define MODULE_H

#include "../../../data/data.h"

typedef struct ModuleConfig ModuleConfig;

// Tamanho do buffer em que o conteúdo do .mod vai ser escrito.
#define MODULE_FILE_READ_BUFFER_SIZE 4096

struct ModuleConfig
{
	const str root_path;
};

// Lida com o dotmod localizado no path, caso tenha algum problema, retorna 'NULL'.
ModuleConfig* handle_module_config(const str path);

#endif