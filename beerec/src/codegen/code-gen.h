#ifndef CODE_GEN_H
#define CODE_GEN_H

#include "../symbols/symbols.h"

static const char* rax_family[] = {"rax", "eax", "ax", "al", "ah", NULL};
static const char* rbx_family[] = {"rbx", "ebx", "bx", "bl", "bh", NULL};
static const char* rcx_family[] = {"rcx", "ecx", "cx", "cl", "ch", NULL};
static const char* rdx_family[] = {"rdx", "edx", "dx", "dl", "dh", NULL};
static const char* rsi_family[] = {"rsi", "esi", "si", "sil", NULL};
static const char* rdi_family[] = {"rdi", "edi", "di", "dil", NULL};
static const char* rsp_family[] = {"rsp", "esp", "sp", "spl", NULL};
static const char* rbp_family[] = {"rbp", "ebp", "bp", "bpl", NULL};
static const char* r8_family[]  = {"r8", "r8d", "r8w", "r8b", NULL};
static const char* r9_family[]  = {"r9", "r9d", "r9w", "r9b", NULL};

static const char** all_families[] = 
{
    rax_family, rbx_family, rcx_family, rdx_family,
    rsi_family, rdi_family, rsp_family, rbp_family,
    r8_family, r9_family,
    NULL
};

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

typedef struct
{
	char* class_name;

	int has_constructor;
	int has_v_table;
}
AsmClassInfo;

typedef struct
{
	AsmClassInfo** classes;

	int class_count;
	int class_capacity;
}
ClassTable;



typedef struct
{
	char* reg;
	Type* type;
}
AsmReturn;

void code_gen_global(CodeGen* code_gen, Node* node);
void setup_code_gen(CodeGen* code_gen, Module* module);
void print_code_generated();

#endif
