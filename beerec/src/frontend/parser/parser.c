#include <stdlib.h>

#include "../../utils/logger/logger.h"
#include "parser.h"

static Token*  get_token(Parser* parser);
static Parser* setup_parser(Lexer* lexer);
static Token*  advance_token(Parser* parser);
static Token*  consume_token(Parser* parser);
static void	   expect_token(Parser* parser, TokenType* types, const u32 types_length);

// ==---------------------------------- Core --------------------------------------== \\

/** TODO: terminar isso. */
static void handle_token(Parser* parser) { Token* token = get_token(parser); }

Parser* parse_tokens(Lexer* lexer)
{
	Parser* parser = setup_parser(lexer);

	while (parser->current != TOKEN_END_SOURCE)
	{
		handle_token(parser);
	}

	return parser;
}

// ==--------------------------------- Utils --------------------------------------== \\

// Avança pro proximo token dos tokens em 'parser', ja retornando o proximo
// token.
static Token* advance_token(Parser* parser)
{
	++parser->index;
	return ++parser->current;
}

// Retorna o token atual dos tokens em 'parser' e avança um token.
static Token* consume_token(Parser* parser)
{
	++parser->index;
	return parser->current++;
}

// Retorna o token atual em que o pointer 'current' do 'parser' aponta.
static Token* get_token(Parser* parser) { return parser->current; }

// Checa se o tipo do token atual que o 'parser' esta, está nos 'types'.
// Caso não esteja nos 'types', da log e depois da exit.
static void expect_token(Parser* parser, TokenType* types, const u32 types_length)
{
	Token* token = parser->current;

	for (i32 i = 0; i < types_length; i++)
	{
		const TokenType type = types[i];

		if (type == token->type)
		{
			return;
		}
	}

	log_error("Unexpected token found...");
	exit(1);
}

// Da setup na structure do parser (alocado na heap).
static Parser* setup_parser(Lexer* lexer)
{
	Parser* parser = calloc(1, sizeof(Parser));

	parser->tokens  = lexer->tokens; // não é uma copia
	parser->current = parser->tokens;

	parser->index = 0;

	parser->nodes = calloc(PARSER_NODES_BUFFER_SIZE, sizeof(ASTNode));

	return parser;
}

// ==--------------------------- Memory Management ---------------------------------== \\

void free_parser(Parser* parser)
{
	if (parser->tokens != NULL)
	{
		free(parser->tokens);
	}

	free(parser->nodes);
	free(parser);
}
