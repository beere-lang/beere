#ifndef TYPES_H
#define TYPES_H

#include "../tokens/tokens.h"

typedef struct Type Type;
typedef struct Symbol Symbol;

struct Type
{
	VarType type;
	
	struct Type* base;

	char* class_name;

	Symbol* class_symbol;
};

#endif