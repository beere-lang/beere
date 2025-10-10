#ifndef MODULE_H
#define MODULE_H

#include "../../../data/data.h"

typedef struct ModuleConfig ModuleConfig;
typedef struct ModuleToken ModuleToken;

// Tamanho do buffer em que o conteúdo do .mod vai ser escrito.
#define MODULE_FILE_READ_BUFFER_SIZE 4096

// Tamanho do buffer em que os tokens são guardados após os dotmod ser tokenizado.
#define MODULE_TOKENS_BUFFER_SIZE 2048

// Guarda todos os atributos do dotmod.
struct ModuleConfig
{
	str root_path;
};

// Tipos de tokens usados na tokenização do dotmod, 
// nada demais, ja que é só uma config basicamente.
typedef enum
{
	MODULE_TOKEN_IDENTIFIER,
	MODULE_TOKEN_STRING,
	MODULE_TOKEN_EQUALS,
	MODULE_TOKEN_END_LINE,
	MODULE_TOKEN_END_SOURCE
}
ModuleTokenType;

// Um token do modulo, usado na tokenização como output.
struct ModuleToken
{
	ModuleTokenType type;
	
	char* start;
	char* end;

	u32 length;
};

// Lida com o dotmod localizado no path, caso tenha algum problema, retorna 'NULL'.
ModuleConfig* handle_module_config(const str path, const u32 dump_tokens);

#endif