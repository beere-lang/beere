#include <stdlib.h>

#include "lexer.h"
#include "../../utils/logger/logger.h"

static char* advance_char(Lexer* lexer);
static char* consume_char(Lexer* lexer);
static Lexer* setup_lexer(u32 max_index, str content);

// ==---------------------------------- Core --------------------------------------== \\

Lexer* tokenize_module(Module* module, u32 max_index, str content)
{
	Lexer* lexer = setup_lexer(max_index, content);

	return lexer;
}

// ==--------------------------------- Utils --------------------------------------== \\

// Avança pro proximo char do content em 'lexer', ja retornando o proximo char e
// dando erro caso acesse o conteúdo fora do 'max_index'
static char* advance_char(Lexer* lexer)
{
	if (lexer == NULL)
	{
		log_error("Invalid lexer found is NULL...");
		exit(1);
	}
	
	++lexer->current;
	++lexer->index;

	if (lexer->index > lexer->max_index)
	{
		log_error("Failed to tokenize module, read outside content memory space...");
		exit(1);
	}

	return lexer->current;
}

// Retorna o char atual, avança pro proximo char do content em 'lexer' e
// dando erro caso acesse o conteúdo fora do 'max_index'
static char* consume_char(Lexer* lexer)
{
	if (lexer == NULL)
	{
		log_error("Invalid lexer found is NULL...");
		exit(1);
	}
	
	char* ch = lexer->current;

	++lexer->current;
	++lexer->index;
	
	if (lexer->index > lexer->max_index)
	{
		log_error("Failed to tokenize module, read outside content memory space...");
		exit(1);
	}

	return ch;
}

// Da setup na structure do lexer (alocado na heap).
static Lexer* setup_lexer(u32 max_index, str content)
{
	Lexer* lexer = calloc(1, sizeof(Lexer));

	lexer->content = content;
	lexer->current = content;

	lexer->index = 0;

	lexer->max_index = max_index;
	lexer->tokens = calloc(LEXER_TOKENS_BUFFER_SIZE, sizeof(Token));
	
	return lexer;
}

// ==--------------------------- Memory Management ---------------------------------== \\

void free_lexer(Lexer* lexer)
{
	if (lexer->content != NULL)
	{
		free(lexer->content);
	}

	free(lexer->tokens);
	free(lexer);
}