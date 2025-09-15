#ifndef MODULES_H
#define MODULES_H

#include "../structure/ast/nodes/nodes.h"
#include "handler/module-handler.h"

typedef struct Symbol Symbol;
typedef struct SymbolTable SymbolTable;

typedef struct Module Module;

typedef struct
{
	Module** modules;

	int modules_count;
	int modules_capacity;
}
ModuleStack;

struct Module
{
	ModuleHandler* handler;

	char* module_path;

	ASTNode** nodes;
	int nodes_count;
	int nodes_capacity;

	SymbolTable* global_scope;

	Symbol** exports;
	int exports_count;
	int exports_capacity;

	Module** imported_modules;
	int modules_count;
	int modules_capacity;

	ModuleStack* stack;
};

Module* compile_module(ModuleHandler* handler, ModuleStack* stack, char* file_path);
int push_stack_module(ModuleStack* stack, Module* module);
int pop_stack_module(ModuleStack* stack, Module* module);
Module* setup_module(char* path, ModuleStack* stack);
ModuleHandler* interpret_module_file(char* path);
ModuleStack* setup_module_stack();

#endif