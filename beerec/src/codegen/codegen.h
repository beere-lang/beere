#ifndef CODEGEN_H
#define CODEGEN_H

#include "../symbols/symbols.h"
#include "../modules/modules.h"

#define asm_return_size sizeof(AsmReturn)
#define asm_area_size sizeof(AsmArea)
#define lines_default_size sizeof(char*) * 10

typedef struct CodeGen CodeGen;
typedef struct Constant Constant;
typedef struct ExternTable ExternTable;
typedef struct ConstantTable ConstantTable;
typedef struct ExternEntry ExternEntry;
typedef struct AsmReturn AsmReturn;
typedef struct AsmArea AsmArea;

extern AsmArea* externs_section;
extern AsmArea* text_section;
extern AsmArea* data_section;
extern AsmArea* rodata_section;
extern AsmArea* bss_section;
extern ConstantTable* constant_table;

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

	int is_reg;
};

struct ExternTable
{
	ExternEntry** externs;

	int externs_capacity;
	int externs_length;
};

struct ExternEntry
{
	char* label;
};

void generate_node(CodeGen* codegen, Node* node, AsmArea* area);
AsmReturn* create_asm_return(char* value, Type* type);
void setup_codegen(Module* module, CodeGen* codegen);
void add_line_to_area(AsmArea* area, char* line);
AsmArea* create_area_with_label(char* label);
void print_code_generated(CodeGen* codegen);
Constant* generate_constant(Node* literal);
AsmArea* create_area();

#endif