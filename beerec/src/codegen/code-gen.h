#ifndef CODE_GEN_H
#define CODE_GEN_H

#include "../symbols/symbols.h"

typedef struct CodeGen CodeGen;
typedef struct AsmLine AsmLine;
typedef struct AsmSlice AsmSlice;
typedef struct AsmArea AsmArea;
typedef struct Constant Constant;
typedef struct ConstantTable ConstantTable;

struct CodeGen
{
	SymbolTable* scope;
};

struct AsmLine
{
	char* line;
};

struct AsmArea
{
	AsmLine** lines;

	int lines_count;
	int lines_capacity;
};

struct Constant
{
	int number;

	AsmArea* area;
};

struct ConstantTable
{
	Constant** constants;

	int constants_count;
	int constants_capacity;
};

#endif
