#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "modules.h"
#include "../../utils/file/file.h"
#include "../../utils/logger/logger.h"
#include "../../utils/string/string.h"

static void get_module_config_atrib(ModuleConfig* cfg, str content, const u32 dump_tokens);

// ==---------------------------------- Core --------------------------------------== \\

ModuleConfig* handle_dotmod(const str path, const u32 dump_tokens)
{
	str extension = "";
	
	if (!has_extension(path, &extension, ".mod"))
	{
		log_error("Invalid extension from file: \"%s\"...", extension);
		exit(1);
	}
	
	str buff = calloc(MODULE_FILE_READ_BUFFER_SIZE, sizeof(char));
	const i32 error = read_file(buff, MODULE_FILE_READ_BUFFER_SIZE, path);

	if (error)
	{
		log_error("Failed to read module config from path: \"%s\"...", path);
		exit(1);
	}

	log_warning("Handling module config...");

	ModuleConfig* cfg = malloc(sizeof(ModuleConfig));
	get_module_config_atrib(cfg, buff, dump_tokens);

	return cfg;
}

// ==--------------------------------- Utils --------------------------------------== \\

// Tokeniza todo o conteúdo (texto plano) em tokens, que dão
// um signicado aos pedaços do conteúdo do dotmod.
static ModuleToken* tokenize_dotmod(ModuleConfig* cfg, str content, u32* length)
{
	ModuleToken* buff = calloc(MODULE_TOKENS_BUFFER_SIZE, sizeof(ModuleToken));
	u32 buff_length = 0;

	u32 i = 0;

	u32 row = 0;
	u32 column = 1;
	
	while (1)
	{
		if (i > MODULE_FILE_READ_BUFFER_SIZE)
		{
			log_error("Failed to tokenize dotmod, reading outside buffer memory space...");
			exit(1);
		}

		char ch = content[i];

		if (ch == '\0')
		{
			break;
		}
		
		if (ch == ' ')
		{
			row++;
			i++;

			continue;
		}

		if (ch == '\r' || ch == '\t')
		{
			i++;
			continue;
		}

		if (ch == '\n')
		{
			ModuleToken token = { 0 };

			token.type = MODULE_TOKEN_END_LINE;

			token.start = &content[i];
			token.end = &token.start[1];
			token.length = 1;
			
			++row;
			column = 1;
			++i;

			buff[buff_length] = token;
			++buff_length;

			continue;
		}

		if (ch == '=')
		{
			ModuleToken token = { 0 };

			token.type = MODULE_TOKEN_EQUALS;

			token.start = &content[i];
			token.end = &token.start[1];
			token.length = 1;

			column++;
			i++;

			buff[buff_length] = token;
			++buff_length;

			continue;
		}

		if (ch == '\"')
		{
			ModuleToken token = { 0 };

			token.type = MODULE_TOKEN_STRING;
			token.start = &content[++i];

			++column;
			char* curr = token.start;

			u32 j = 0;

			while (*curr != '\"')
			{
				if (*curr == '\n' || *curr == '\0')
				{
					log_error("Failed to tokenize dotmod, found a incomplete string at row: %d, column: %d", row, column);
					exit(1);
				}

				++j;
				curr = &token.start[j];
			}

			token.end = curr;
			token.length = j;

			i += token.length + 1;
			column += token.length + 1;

			buff[buff_length] = token;
			++buff_length;

			continue;
		}

		if (isalpha(ch))
		{
			ModuleToken token = { 0 };

			token.type = MODULE_TOKEN_IDENTIFIER;
			token.start = &content[i];

			u32 j = 0;

			char* curr = token.start;

			while (isalpha(*curr) || isalnum(*curr))
			{
				++j;
				curr = &token.start[j];
			}

			token.end = curr;
			token.length = j;

			i += token.length;
			column += token.length;

			buff[buff_length] = token;
			++buff_length;

			continue;
		}
		
		log_error("Not expected char found at row: \"%d\", column: %d...", row, column);
		exit(1);
	}

	ModuleToken token = { 0 };
	token.type = MODULE_TOKEN_END_SOURCE;

	buff[buff_length] = token;
	++buff_length;

	*length = buff_length;

	return buff;
}

// Lida com todo o conteúdo do dotmod e atribui cada elemento na struct da config 'cfg'.
static void get_module_config_atrib(ModuleConfig* cfg, str content, const u32 dump_tokens)
{
	u32 tokens_length = 0;
	ModuleToken* tokens = tokenize_dotmod(cfg, content, &tokens_length);
	
	if (dump_tokens)
	{	
		for (u32 i = 0; i < tokens_length; i++)
		{
			ModuleToken* token = &tokens[i];
			println("DotMod Token Dump: %d", token->type);
		}
	}

	ModuleToken* curr = &tokens[0];

	while (curr->type != MODULE_TOKEN_END_SOURCE)
	{
		while (curr->type == MODULE_TOKEN_END_LINE)
		{
			curr++;
		}
		
		if (curr->type != MODULE_TOKEN_IDENTIFIER)
		{
			log_error("Expected a attribute identifier...");
			exit(1);
		}

		str identifier = strndup(curr->start, curr->length);

		if (strcmp(identifier, "root") == 0)
		{
			curr++;

			if (curr->type != MODULE_TOKEN_EQUALS)
			{
				log_error("Expected a assign...", identifier);
				exit(1);
			}

			curr++;

			if (curr->type != MODULE_TOKEN_STRING)
			{
				log_error("Expected a string literal...", identifier);
				exit(1);
			}

			str path = strndup(curr->start, curr->length);
			cfg->root_path = path;

			curr++;
			log_success("Found root path: \"%s\"", cfg->root_path);

			continue;
		}

		log_error("Invalid attribute identifier found: \"%s\"...", identifier);
		exit(1);
	}
}

Module* setup_module(ModuleConfig* cfg, str path)
{
	Module* module = calloc(1, sizeof(Module));
	
	module->cfg = cfg;
	module->imports = create_list(8);

	str full_path = get_full_path(cfg->root_path, path);

	if (full_path == NULL)
	{
		log_error("Failed to get full path from module...");
		exit(1);
	}

	module->path = full_path;

	return module;
}

// ==--------------------------- Memory Management ---------------------------------== \\

void free_module_config(ModuleConfig* cfg)
{
	if (cfg->root_path != NULL)
	{
		free(cfg->root_path);
	}

	free(cfg);
}

/**
 * TODO: adicionar memory management pra 'Module' structures.
 */