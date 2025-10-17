#ifndef TOKENS_H
#define TOKENS_H

#include "../../../../data/data.h"

typedef struct Token		Token;
typedef struct ConstantNumber ConstantNumber;
typedef struct ConstantString ConstantString;
typedef struct ConstantBool	ConstantBool;
typedef struct ConstantChar	ConstantChar;

// Inicializa uma structure de token, setando apenas os campos principais.
#define CREATE_TOKEN(t, s, l) { .type = t, .start = s, .length = l }

// Tipos de token.
typedef enum
{
	TOKEN_END_SOURCE,
	TOKEN_END_LINE,

	TOKEN_UNKNOWN,

	TOKEN_IDENTIFIER,

	TOKEN_KEYWORD_IF,
	TOKEN_KEYWORD_ELSE,
	TOKEN_KEYWORD_CONTINUE,
	TOKEN_KEYWORD_BREAK,
	TOKEN_KEYWORD_STATIC,
	TOKEN_KEYWORD_CLASS,
	TOKEN_KEYWORD_SUPER,
	TOKEN_KEYWORD_IMPORT,
	TOKEN_KEYWORD_AS,
	TOKEN_KEYWORD_RETURN,
	TOKEN_KEYWORD_LET,
	TOKEN_KEYWORD_FN,
	TOKEN_KEYWORD_CONST,
	TOKEN_KEYWORD_VIRTUAL,
	TOKEN_KEYWORD_OVERRIDE,
	TOKEN_KEYWORD_PUBLIC,
	TOKEN_KEYWORD_PRIVATE,
	TOKEN_KEYWORD_FOR,
	TOKEN_KEYWORD_WHILE,
	TOKEN_KEYWORD_VOID,

	TOKEN_TYPE_INT,
	TOKEN_TYPE_FLOAT,
	TOKEN_TYPE_DOUBLE,
	TOKEN_TYPE_CHAR,
	TOKEN_TYPE_BOOL,
	TOKEN_TYPE_STRING,

	TOKEN_CHAR_PLUS,
	TOKEN_CHAR_MINUS,
	TOKEN_CHAR_ASTERISK,
	TOKEN_CHAR_DASH,
	TOKEN_CHAR_EQUALS,
	TOKEN_CHAR_COLON,
	TOKEN_CHAR_DOT,
	TOKEN_CHAR_LEFT_PAREN,
	TOKEN_CHAR_RIGHT_PAREN,
	TOKEN_CHAR_LEFT_BRACE,
	TOKEN_CHAR_RIGHT_BRACE,
	TOKEN_CHAR_LEFT_BRACKET,
	TOKEN_CHAR_RIGHT_BRACKET,

	TOKEN_OPERATOR_INCREMENT,
	TOKEN_OPERATOR_DECREMENT,
	TOKEN_OPERATOR_LOG_OR,
	TOKEN_OPERATOR_LOG_AND,
	TOKEN_OPERATOR_EQUALS,

	TOKEN_OPERATOR_PLUS_EQUALS,
	TOKEN_OPERATOR_MINUS_EQUALS,
	TOKEN_OPERATOR_TIMES_EQUALS,
	TOKEN_OPERATOR_DIVIDED_EQUALS,

	TOKEN_CONSTANT_BOOL,
	TOKEN_CONSTANT_STRING,
	TOKEN_CONSTANT_CHAR,
	TOKEN_CONSTANT_NUMBER
} TokenType;

// Tipos de literais de numero.
typedef enum
{
	NUMBER_TYPE_INT,
	NUMBER_TYPE_FLOAT,
	NUMBER_TYPE_DOUBLE
} NumberType;

// Structure de literais de numero, contendo seu valor e tipo.
struct ConstantNumber
{
	NumberType type;

	union
	{
		i32 int_value;
		f32 float_value;
		f64 double_value;
	};
};

// Structure de literais de string, contendo seu valor (copia).
struct ConstantString
{
	str value;
};

// Structure de literais de char, contendo seu valor.
struct ConstantChar
{
	char value;
};

// Structure de literais de boolean, contendo seu valor.
struct ConstantBool
{
	int value;
};

// Structure de um token, contendo o tipo do token,
// onde começa e onde termina, mais structures adicionais.
// (informação de literais, etc).
struct Token
{
	TokenType type;

	char*	    start;

	u32	    length;

	union
	{
		ConstantNumber constant_number;

		ConstantString constant_string;
		ConstantChar   constant_char;

		ConstantBool   constant_bool;
	};
};

#endif