#include <stdlib.h>

#include "lexer.h"
#include "../../utils/logger/logger.h"

static char get_char(Lexer* lexer);
static i32 is_unknown(Token* token);
static char* advance_char(Lexer* lexer);
static char* consume_char(Lexer* lexer);
static Lexer* setup_lexer(u32 max_index, str content);
static void add_token(Token* token, u32* tokens_length, Token* tokens);

// ==---------------------------------- Core --------------------------------------== \\

// Lida com um pedaço que pode ser considerado uma palavra com chars ("a-Z", "_", "0-9").
// Retorna desconhecido 'TOKEN_UNKNOWN' caso não seja.
static str handle_identifier(Lexer* lexer)
{
	return NULL;
}

// Verifica se a palavra 'identifier' se é igual ao nome de um tipo.
// Retorna desconhecido 'TOKEN_UNKNOWN' caso não seja.
static Token handle_types(Lexer* lexer, str identifier)
{
	
	
	Token unknown = { 0 };
	unknown.type = TOKEN_UNKNOWN;

	return unknown;
}

// Verifica se a palavra 'identifier' se é igual ao nome de uma keyword.
// Retorna desconhecido 'TOKEN_UNKNOWN' caso não seja.
static Token handle_keywords(Lexer* lexer, str identifier)
{
	
	
	Token unknown = { 0 };
	unknown.type = TOKEN_UNKNOWN;

	return unknown;
}

// Verifica se o char atual é algum char especifico ("()", "*", "-", "+", etc).
// Retorna desconhecido 'TOKEN_UNKNOWN' caso não seja.
static Token handle_chars(Lexer* lexer)
{
	
	
	Token unknown = { 0 };
	unknown.type = TOKEN_UNKNOWN;

	return unknown;
}

// Verifica se vai vir uma constant (strings, numeros, etc).
// Retorna desconhecido 'TOKEN_UNKNOWN' caso não seja.
static Token handle_constants(Lexer* lexer)
{
	
	
	Token unknown = { 0 };
	unknown.type = TOKEN_UNKNOWN;

	return unknown;
}

Lexer* tokenize_module(Module* module, u32 max_index, str content)
{
	Lexer* lexer = setup_lexer(max_index, content);
	u32 tokens_length = 0;
	
	while (1)
	{
		const char ch = get_char(lexer);

		if (ch == '\r' || ch == ' ' || ch == '\t')
		{
			advance_char(lexer);
			continue;
		}

		if (ch == '\n')
		{
			Token token = { 0 };
			
			token.start = lexer->current;
			token.end = lexer->current + 1;
			token.type = TOKEN_END_LINE;

			add_token(&token, &tokens_length, lexer->tokens);
			
			advance_char(lexer);
			continue;
		}

		if (ch == '\0')
		{
			Token token = { 0 };
			
			token.start = lexer->current;
			token.end = lexer->current + 1;
			token.type = TOKEN_END_SOURCE;

			add_token(&token, &tokens_length, lexer->tokens);

			advance_char(lexer);
			break;
		}

		str identifier = handle_identifier(lexer);

		if (identifier != NULL)
		{
			Token token_type = handle_types(lexer, identifier);
			
			if (!is_unknown(&token_type))
			{
				add_token(&token_type, &tokens_length, lexer->tokens);
				continue;
			}
	
			Token token_keyword = handle_keywords(lexer, identifier);
	
			if (!is_unknown(&token_keyword))
			{
				add_token(&token_keyword, &tokens_length, lexer->tokens);
				continue;
			}
		}

		Token token_char = handle_chars(lexer);
	
		if (!is_unknown(&token_char))
		{
			add_token(&token_char, &tokens_length, lexer->tokens);
			continue;
		}

		Token constant = handle_constants(lexer);

		if (!is_unknown(&constant))
		{
			add_token(&constant, &tokens_length, lexer->tokens);
			continue;
		}
	}

	return lexer;
}

// ==--------------------------------- Utils --------------------------------------== \\

// Retorna se um token é do tipo desconhecido 'TOKEN_UNKNOWN'.
static i32 is_unknown(Token* token)
{
	return token->type != TOKEN_UNKNOWN;
}

// Adiciona o token 'token' a array de tokens 'tokens' e incrementa o valor
// que o pointer 'tokens_length' aponta.
static void add_token(Token* token, u32* tokens_length, Token* tokens)
{
	tokens[(*tokens_length)++] = *token;
}

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

// Retorna o char em que o ponteiro 'current' aponta.
static char get_char(Lexer* lexer)
{
	return *lexer->current;
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