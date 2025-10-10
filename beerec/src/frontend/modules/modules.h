#ifndef MODULE_H
#define MODULE_H

#include "../../../data/data.h"
#include "../../utils/list/list.h"

typedef struct ModuleConfig ModuleConfig;
typedef struct ModuleToken ModuleToken;
typedef struct Module Module;

// Tamanho do buffer em que o conteúdo do .mod vai ser escrito.
#define MODULE_FILE_READ_BUFFER_SIZE 4096

// Tamanho do buffer em que os tokens são guardados após o dotmod ser tokenizado.
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
	
	char*           start;
	char*           end;

	u32             length;
};

// A structure de um modulo (guarda informaçoes importantes sobre um).
struct Module
{
	str           path; // path real, não o relativo.

	ModuleConfig* cfg;
	DList*        imports;
};

// Lida com o dotmod localizado no path, caso tenha algum problema, retorna 'NULL'.
ModuleConfig* handle_dotmod(const str path, const u32 dump_tokens);

// Da free na struct da config do dotmod 'cfg' e seus atributos.
void          free_module_config(ModuleConfig* cfg);

// Da setup na structure do module (alocado na heap).
Module*       setup_module(ModuleConfig* cfg, str path);

#endif