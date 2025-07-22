#ifndef CODEGEN_H
#define CODEGEN_H

#include "../symbols/symbols.h"

#define asm_return_size sizeof(AsmReturn)
#define asm_area_size sizeof(AsmArea)
#define lines_default_size sizeof(char*) * 10

typedef struct CodeGen CodeGen;
typedef struct Constant Constant;
typedef struct ConstantTable ConstantTable;
typedef struct AsmReturn AsmReturn;
typedef struct AsmArea AsmArea;

AsmArea* externs_section = NULL;
AsmArea* text_section = NULL;
AsmArea* data_section = NULL;
AsmArea* rodata_section = NULL;
AsmArea* bss_section = NULL;

ConstantTable* constant_table = NULL;

typedef enum
{
	FLAG_ASSIGN,
	FLAG_FORCE_REG
}
Flag;

struct CodeGen
{
	int prefer_second;

	Module* module;
	SymbolTable* scope;
};

struct Constant
{
	int id;
	char* value;
};

struct ConstantTable
{
	Constant** constants;

	int constants_capacity;
	int constant_length;
};

struct AsmArea
{
	char** lines;

	int lines_capacity;
	int lines_length;
};

struct AsmReturn
{
	char* result;
	Type* type;
};

AsmReturn* create_asm_return(char* value, Type* type);
void add_line_to_area(AsmArea* area, char* line);
AsmArea* create_area_with_label(char* label);
Constant* generate_constant(Node* literal);
AsmArea* create_area();

#endif