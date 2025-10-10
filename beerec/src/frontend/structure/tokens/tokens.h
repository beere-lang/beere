#ifndef TOKENS_H
#define TOKENS_H

#include "../../../../data/data.h"

typedef struct Token Token;
typedef struct TokenNumber TokenNumber;
typedef struct TokenString TokenString;

// Tipos de token.
typedef enum
{
	TOKEN_END_SOURCE,
	TOKEN_END_LINE,
	TOKEN_STRING,
	TOKEN_NUMBER
}
TokenType;

// Tipos de literais de numero.
typedef enum
{
	NUMBER_TYPE_INT,
	NUMBER_TYPE_FLOAT,
	NUMBER_TYPE_DOUBLE
}
NumberType;

// Structure de literais de numero, contendo seu valor e tipo.
struct TokenNumber
{
	NumberType type;
	
	union
	{
		i32    int_value;
		f32    float_value;
		f64    double_value;
	};
};

// Structure de literais de string, contendo seu valor (copia).
struct TokenString
{
	str value;
};

// Structure de um token, contendo o tipo do token,
// onde começa e onde termina, mais structures adicionais.
// (informação de literais, etc).
struct Token
{
	TokenType       type;

	char*           start;
	char*           end;

	union
	{
		TokenNumber number;
		TokenString string;
	};
};

#endif