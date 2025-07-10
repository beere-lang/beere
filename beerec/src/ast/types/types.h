#ifndef TYPES_H
#define TYPES_H

#include "../tokens/tokens.h"

typedef struct Type Type;

struct Type
{
	VarType type;
	
	/**
	 * EX: let i_ptr_ptr: int** = &i_ptr --> pointer to pointer --> pointer to int
	 */
	struct Type* base;

	/**
	 * For this pointers
	 */
	char* class_name;
};

#endif