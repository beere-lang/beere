#include <stdlib.h>
#include <string.h>

#include "modules.h"
#include "../utils/utils.h"
#include "../../compiler/compiler.h"

ModuleStack* setup_module_stack()
{
	ModuleStack* stack = malloc(sizeof(ModuleStack));

	stack->modules = malloc(sizeof(Module*) * 4);
	stack->modules_capacity = 4;
	stack->modules_count = 0;

	stack->modules[0] = NULL;

	return stack;
}

static void add_module_to_stack(ModuleStack* stack, Module* module)
{
	if (stack->modules_capacity <= (stack->modules_count + 1))
	{
		stack->modules_capacity *= 2;

		stack->modules = realloc(stack->modules, sizeof(Module*) * stack->modules_capacity);

		if (stack->modules == NULL)
		{
			exit(1);
		}
	}

	stack->modules[stack->modules_count] = module;
	stack->modules_count++;
	stack->modules[stack->modules_count] = NULL;
}

static void shift_left(ModuleStack* stack, int index)
{
	Module** arr = stack->modules;

	for (int i = index; i < (stack->modules_count - 1); i++)
	{
		arr[i] = arr[i + 1];
	}

	arr[stack->modules_count - 1] = NULL;
	stack->modules_count--;
}

static int contains_module(ModuleStack* stack, Module* module)
{
	for (int i = 0; i < stack->modules_count; i++)
	{
		Module* current = stack->modules[i];
		
		if (strcmp(module->module_path, current->module_path) == 0)
		{
			return 1;
		}
	}

	return 0;
}

int push_stack_module(ModuleStack* stack, Module* module)
{
	if (contains_module(stack, module))
	{
		printf("[Module Manager] [Debug] Invalid import, module will be imported recursively: \"%s\"...", module->module_path);
		return 0;
	}

	add_module_to_stack(stack, module);

	return 1;
}

/**
 * Importante: Função checa pelo endereço.
 */
int pop_stack_module(ModuleStack* stack, Module* module)
{
	for (int i = 0; i < stack->modules_count; i++)
	{
		Module* current = stack->modules[i];

		if (current == module)
		{
			shift_left(stack, i);

			return 1;
		}
	}

	return 0;
}



Module* setup_module(char* path, ModuleStack* stack)
{
	Module* module = malloc(sizeof(Module));

	path = strdup(path);

	module->module_path = path;

	module->stack = stack;
	
	module->exports = malloc(sizeof(Symbol*) * 4);
	module->exports_count = 0;
	module->exports_capacity = 4;

	module->global_scope = NULL;

	module->nodes = malloc(sizeof(Node*) * 4);
	module->nodes_count = 0;
	module->nodes_capacity = 4;

	module->imported_modules = malloc(sizeof(Node*) * 4);
	module->modules_count = 0;
	module->modules_capacity = 4;

	return module;
}

Module* compile_module(ModuleStack* stack, char* file_path)
{
	if (file_path == NULL)
	{
		printf("[Compiler] [Debug] File Path not found...\n");
		exit(1);
	}

	char* content = read_file(file_path);

	if (content == NULL)
	{
		printf("[Compiler] [Debug] Failed to read input file: %s...\n", file_path);
		exit(1);
	}

	//printf("+-----------------------------------------------+\n");
	//printf("Content:\n%s\n", content);
	//printf("+-----------------------------------------------+\n");

	Module* module = setup_module(file_path, stack);

	Token* tokens = tokenize_code(content, 0);

	Node** node_list = parse_tokens(tokens);

	if (!push_stack_module(stack, module))
	{
		exit(1);
	}

	analyze_nodes(module, node_list);

	pop_stack_module(stack, module);

	return module;
}