#include <stdlib.h>

#include "../../utils/logger/logger.h"
#include "../../utils/string/string.h"
#include "parser.h"

static Token*  get_token(Parser* parser);
static Parser* setup_parser(Lexer* lexer);
static Type*   handle_type(Parser* parser);
static Token*  advance_token(Parser* parser);
static Token*  consume_token(Parser* parser);
static void	   add_node(ASTNode** nodes, u32* nodes_length, ASTNode* node);
static void	   expect_token(Parser* parser, TokenType* types, const u32 types_length);

// ==------------------------------ Global Fields ---------------------------------== \\

TypeTable* type_table = NULL;

// ==---------------------------------- Core --------------------------------------== \\

// Da parse em funções, ja adicionando a node final no output do parser.
static void parse_func(Parser* parser)
{
	i32 has_static	 = 0;
	i32 has_virtual	 = 0;
	i32 has_override	 = 0;
	i32 has_visibility = 0;

	TokenType type = get_token(parser)->type;

	Visibility visibility = VISIBILITY_PRIVATE;

	i32 is_static   = 0;
	i32 is_override = 0;
	i32 is_virtual  = 0;

	i32 is_constructor = 1;

	ASTNode* node = create_node(AST_NODE_FUNC);

	while (IS_FUNC_ATTRIBUTE(type))
	{
		if (has_static > 1 || has_virtual > 1 || has_override > 1 || has_visibility > 1)
		{
			log_error("Duplicated function attribute...");
			exit(1);
		}

		if (has_override && has_virtual)
		{
			log_error("A function cant be virtual and override at the same time...");
			exit(1);
		}

		if (has_static && (has_override || has_virtual))
		{
			log_error("A function cant be static and virtual / override at the same time...");
			exit(1);
		}

		switch (type)
		{
			case TOKEN_KEYWORD_PUBLIC:
			{
				visibility = VISIBILITY_PUBLIC;
				has_visibility++;

				break;
			}

			case TOKEN_KEYWORD_PRIVATE:
			{
				visibility = VISIBILITY_PRIVATE;
				has_visibility++;

				break;
			}

			case TOKEN_KEYWORD_STATIC:
			{
				is_static = 1;
				has_static++;

				break;
			}

			case TOKEN_KEYWORD_VIRTUAL:
			{
				is_virtual = 1;
				has_virtual++;

				break;
			}

			case TOKEN_KEYWORD_OVERRIDE:
			{
				is_override = 1;
				has_override++;

				break;
			}

			default:
			{
				log_error("Invalid function attribute...");
				exit(1);
			}
		}

		Token* token = advance_token(parser);
		type		 = token->type;
	}

	if (get_token(parser)->type == TOKEN_KEYWORD_FN)
	{
		is_constructor = 0;
		advance_token(parser);
	}

	expect_token(parser, (TokenType[]){ TOKEN_IDENTIFIER }, 1);

	Token* identifier_token = consume_token(parser);
	str	 identifier		= strndup(identifier_token->start, identifier_token->length);

	// TODO: dar handle nos parameters

	Type* func_type = NULL;

	if (get_token(parser)->type == TOKEN_CHAR_COLON)
	{
		advance_token(parser);
		func_type = handle_type(parser);
	}
	else
	{
		func_type = create_type(TYPE_VOID, NULL); // deixado aqui pra não precisar de free.
	}

	// TODO: dar handle no block

	node->func.identifier  = identifier;
	node->func.type	     = func_type;
	node->func.visibility  = visibility;
	node->func.is_virtual  = is_virtual;
	node->func.is_override = is_override;
	node->func.is_static   = is_static;

	add_node(parser->nodes, &parser->nodes_length, node);
}

// Funciona como uma especie de hub pros tokens, onde, dependendo do token,
// ele chama sua função correspondente.
static void handle_token(Parser* parser)
{
	TokenType type = get_token(parser)->type;

	if (IS_FUNC_ATTRIBUTE(type))
	{
		parse_func(parser);
	}
}

Parser* parse_tokens(Lexer* lexer)
{
	Parser* parser = setup_parser(lexer);
	type_table	   = setup_type_table();

	while (parser->current != TOKEN_END_SOURCE)
	{
		handle_token(parser);
	}

	return parser;
}

// ==--------------------------------- Utils --------------------------------------== \\

// Pula tokens de end line até chegar em um token que não seja end line.
static void jump_end_lines(Parser* parser)
{
	TokenType type = get_token(parser)->type;

	while (type == TOKEN_END_LINE)
	{
		type = advance_token(parser)->type;
	}
}

// Lida com o possivel type que está no token atual,
// caso não seja um type, da exit.
static Type* handle_type(Parser* parser) { return NULL; }

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

// Adiciona a node 'node' a array de nodes 'nodes' e incrementa o valor
// que o pointer 'nodes_length' aponta.
static void add_node(ASTNode** nodes, u32* nodes_length, ASTNode* node) { nodes[(*nodes_length)++] = node; }

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
