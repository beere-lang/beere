#ifndef PARSER_H
#define PARSER_H

#include "../lexer/lexer.h"
#include "../structure/nodes/nodes.h"
#include "../structure/tokens/tokens.h"

typedef struct Parser Parser;

// Tamanho do buffer em que as nodes são guardadas após os tokens serem parse.
#define PARSER_NODES_BUFFER_SIZE 1024

// Checa se o TokenType 't' é algum atributo de função, tipo, public, private, static, override, virtual, etc
#define IS_FUNC_ATTRIBUTE(t)                                                                                           \
	t == TOKEN_KEYWORD_VIRTUAL || t == TOKEN_KEYWORD_PUBLIC || t == TOKEN_KEYWORD_PRIVATE ||                         \
	    t == TOKEN_KEYWORD_STATIC || t == TOKEN_KEYWORD_OVERRIDE

// Structure do parser, usado mais pra organizar
// o output e o gerenciamento dos tokens.
struct Parser
{
	Token*    tokens;

	Token*    current;
	u32	    index;

	ASTNode** nodes; // output
	u32	    nodes_length;
};

// Da parse nos tokens do 'lexer', gerando as ASTNodes,
// saindo como output na structure de 'Parser' retornada.
Parser* parse_tokens(Lexer* lexer);

// Da free na structure do parser 'parser' e seu conteúdo.
void	  free_parser(Parser* parser);

#endif