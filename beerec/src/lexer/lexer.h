#ifndef LEXER_H
#define LEXER_H

#define MAX_TOKEN_LENGTH 1024

#include <stdio.h>
#include "../ast/tokens/tokens.h"

typedef struct
{
	char* start;
	char* current;
	size_t line;
} Lexer;

Token read_next_tkn(Lexer* lexer);
TokenType get_by_keyword_type(Lexer* lexer, const char* start, size_t length);
const char* token_type_to_string(const TokenType type);

#endif //LEXER_H
