#ifndef MODULES_H
#define MODULES_H

#include "../ast/nodes/nodes.h"

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
	char* module_path;

	Node** nodes;
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

Module* compile_module(ModuleStack* stack, char* file_path);
int push_stack_module(ModuleStack* stack, Module* module);
int pop_stack_module(ModuleStack* stack, Module* module);
ModuleStack* setup_module_stack();
Module* setup_module(char* path, ModuleStack* stack);

#endif