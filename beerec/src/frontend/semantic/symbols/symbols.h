#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "../../structure/ast/types/types.h"
#include "../../structure/ast/nodes/nodes.h"

typedef struct Symbol Symbol;
typedef struct SymbolTable SymbolTable; // AKA: scope

typedef struct SymbolVariable SymbolVariable;
typedef struct SymbolFunction SymbolFunction;
typedef struct SymbolModule SymbolModule;
typedef struct SymbolClass SymbolClass;

typedef struct ClassVTable ClassVTable;
typedef struct MethodEntry MethodEntry;

typedef struct Module Module;

typedef enum
{
	SYMBOL_VARIABLE,
	SYMBOL_FUNCTION,
	SYMBOL_IF,
	SYMBOL_ELSE,

	SYMBOL_WHILE,
	SYMBOL_FOR,
	SYMBOL_CLASS,
	SYMBOL_SWITCH,
	SYMBOL_SWITCH_CASE,

	GLOBAL_SCOPE,

	SYMBOL_MODULE,
}
SymbolType;

struct SymbolVariable
{
	int is_export;
	
	Type* type;
	const char* identifier;

	int is_const;
	int is_global;

	int offset;
	int is_static;
	int is_class_global;
};

struct SymbolFunction
{
	int is_export;
	
	Type* return_type;
	const char* identifier;

	ASTNode* params_head;

	SymbolTable* scope;

	int total_offset;

	int total_param_offset;

	int is_virtual;
	int is_override;
	
	int is_static;
};

struct MethodEntry
{
	char* method_name;
	char* class_name;

	int method_index;
};

struct ClassVTable
{
	MethodEntry** entries;

	int entries_count;
	int entries_capacity;
};

struct SymbolClass
{
	int class_id;

	int is_export;

	SymbolTable* class_scope;

	Symbol* constructor;

	const char* identifier;
	Symbol* super;

	ASTNode** functions;
	ASTNode** fields;

	ASTNode* constructor_node;

	int field_count;
	int func_count;

	int total_offset;
	int offset;

	ClassVTable* class_v_table;
};

struct SymbolModule
{
	char* identifier;

	Module* module;
};

struct Symbol
{
	SymbolType type;

	int is_prototype;

	int is_export;

	union
	{
		struct SymbolVariable* symbol_variable;
		struct SymbolFunction* symbol_function;
		struct SymbolClass* symbol_class;
		struct SymbolModule* symbol_module;
	};
};

struct SymbolTable
{
	Symbol* owner_statement;

	SymbolTable* parent;

	SymbolType scope_kind;

	Symbol** symbols;

	int capacity;
	int count;
};

#endif