#ifndef PARSER_H
#define PARSER_H

#include "../structure/tokens/tokens.h"
#include "../structure/nodes/nodes.h"
#include "../lexer/lexer.h"

#define PARSER_NODES_BUFFER_SIZE 1024

typedef struct Parser Parser;

// Structure do parser, usado mais pra organizar
// o output e o gerenciamento dos tokens.
struct Parser
{
	Token*    tokens;

	Token*    current;
	u32       index;
	
	ASTNode** nodes; // output
};

// Da parse nos tokens do 'lexer', gerando as ASTNodes,
// saindo como output na structure de 'Parser' retornada.
Parser* parse_tokens(Lexer* lexer);

// Da free na structure do parser 'parser' e seu conteúdo.
void    free_parser (Parser* parser);

#endif