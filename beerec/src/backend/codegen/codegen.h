#ifndef CODEGEN_H
#define CODEGEN_H

#include "../../frontend/semantic/symbols/symbols.h"
#include "../../frontend/modules/modules.h"


#define asm_return_size sizeof(AsmReturn)
#define asm_area_size sizeof(AsmArea)
#define lines_default_size sizeof(char*) * 10

typedef struct CodeGen CodeGen;
typedef struct Constant Constant;
typedef struct ExternTable ExternTable;
typedef struct ClassOffsetsTable ClassOffsetsTable;
typedef struct MethodRegisterStack MethodRegisterStack;
typedef struct MethodRegisterStackTable MethodRegisterStackTable;
typedef struct RegistersTable RegistersTable;
typedef struct RegistersClass RegistersClass;
typedef struct AsmReturnValue AsmReturnValue;
typedef struct ConstantTable ConstantTable;
typedef struct ExternEntry ExternEntry;
typedef struct AsmReturn AsmReturn;
typedef struct Register Register;
typedef struct AsmArea AsmArea;

extern AsmArea* externs_section;
extern AsmArea* text_section;
extern AsmArea* data_section;
extern AsmArea* rodata_section;
extern AsmArea* bss_section;
extern ConstantTable* constant_table;
extern ClassOffsetsTable* class_offsets_table;

typedef enum
{
	FLAG_ASSIGN,
	FLAG_FORCE_REG
}
Flag;

struct CodeGen
{
	int inner_class;
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

typedef enum
{
	REGISTER_RETURN_TYPE,
	TEXT_RETURN_TYPE
}
AsmReturnValueType;

struct AsmReturnValue
{
	AsmReturnValueType type;
	
	Register* reg;
	char* text;
};

struct AsmReturn
{
	Type* type;
	AsmReturnValue* value;
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

typedef enum
{
	BITS_SIZE_8,
	BITS_SIZE_16,
	BITS_SIZE_32,
	BITS_SIZE_64
}
BitsSize;

typedef enum
{
	CLASS_GENERALS,
	CLASS_FLOATS
}
RegistersClassType;

struct RegistersClass
{
	RegistersClassType class;

	Register** registers;

	int registers_capacity;
	int registers_length;
};

struct RegistersTable
{
	RegistersClass** registers_classes;

	int registers_classes_capacity;
	int registers_classes_length;
};

struct Register
{
	char* reg;
	int in_use;
	BitsSize size;

	Register* parent;
	Register* child;
};

struct MethodRegisterStack
{
	Symbol* method;
	
	Register** registers;

	int registers_capacity;
	int registers_length;
};

struct MethodRegisterStackTable
{
	MethodRegisterStack** stacks;

	int stacks_capacity;
	int stacks_length;
};

AsmReturn* create_asm_return(char* value, Register* reg, Type* type, int is_reg);
Register* find_and_use_register(Type* type, BitsSize size, Symbol* method);
Constant* generate_directly_constant(double value, int is_double);
void generate_node(CodeGen* codegen, Node* node, AsmArea* area);
Register* find_register_by_name(char* name, Type* type);
void setup_codegen(Module* module, CodeGen* codegen);
void add_extern_entry_to_table(ExternEntry* entry);
void add_line_to_area(AsmArea* area, char* line);
Symbol* find_owner_method(SymbolTable* scope);
ExternEntry* create_extern_entry(char* label);
AsmArea* create_area_with_label(char* label);
void print_code_generated(CodeGen* codegen);
Constant* generate_constant(Node* literal);
void unuse_register(Register* reg);
AsmArea* create_area();

#endif