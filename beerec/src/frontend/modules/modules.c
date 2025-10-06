#include <stdlib.h>
#include <string.h>

#include "modules.h"
#include "../../utils/utils.h"
#include "../../init/compiler.h"
#include "handler/module-handler.h"

static void free_module_nodes(ModuleNode** node_list);
static void free_module_parser(ModuleParser* parser);
static void free_module_tokens(Token* token_list);

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
		printf("\n[Module Manager] [Debug] Invalid import, module will be imported recursively: \"%s\"...\n", module->module_path);
		return 0;
	}

	add_module_to_stack(stack, module);

	return 1;
}

/**
 * WARNING: checa pelo endereço.
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

	module->nodes = malloc(sizeof(ASTNode*) * 4);
	module->nodes_count = 0;
	module->nodes_capacity = 4;

	module->imported_modules = malloc(sizeof(ASTNode*) * 4);
	module->modules_count = 0;
	module->modules_capacity = 4;

	return module;
}

Module* compile_module(ModuleHandler* handler, ModuleStack* stack, char* file_path)
{
	file_path = strndup(file_path, strlen(file_path) + 1);
	
	if (file_path == NULL)
	{
		printf("[Compiler] [Debug] File Path not found...\n");
		exit(1);
	}

	char* content = read_file(file_path, 0);

	if (content == NULL)
	{
		printf("[Compiler] [Debug] Failed to read input file: %s...\n", file_path);
		exit(1);
	}

	Module* module = setup_module(file_path, stack);
	module->handler = handler;

	Token* tokens = tokenize_code(content, 0);

	ASTNode** node_list = parse_tokens(tokens);

	if (!push_stack_module(stack, module))
	{
		exit(1);
	}

	analyze_nodes(module, node_list);

	pop_stack_module(stack, module);

	return module;
}

ModuleHandler* interpret_module_file(char* path)
{
	if (path == NULL)
	{
		printf("[Modules] [Debug] File Path not found...\n");
		exit(1);
	}

	char* content = read_file(path, 1);

	if (content == NULL)
	{
		printf("[Compiler] [Debug] Failed to read module file: %s...\n", path);
		exit(1);
	}

	ModuleHandler* handler = malloc(sizeof(ModuleHandler));
	handler->root_path = NULL;
	handler->original_path = path;

	Token* tokens = tokenize_code(content, 0);

	ModuleParser* module_parser = malloc(sizeof(ModuleParser));

	module_parser->tokens = tokens;
	module_parser->current = tokens;

	ModuleNode** node_list = parse_statements(module_parser);

	free_module_parser(module_parser); // não vai ser usado mais, só o conteúdo.
	
	handle_nodes(handler, node_list);

	free_module_tokens(tokens); // não vai ser usado mais.
	free_module_nodes(node_list); // não vai ser usado mais.

	printf("\n");

	return handler;
}

static void free_module_node(ModuleNode* ASTNode)
{
	if (ASTNode == NULL)
	{
		return;
	}
	
	switch (ASTNode->type)
	{
		case MODULE_NODE_DECLARATION:
		{
			free(ASTNode->module_node_declaration->identifier);
			free(ASTNode->module_node_declaration->value);

			break;
		}

		default:
		{
			exit(1);
		}
	}

	free(ASTNode);
	ASTNode = NULL;
}

static void free_module_nodes(ModuleNode** node_list)
{
	if (node_list == NULL)
	{
		return;
	}
	
	ModuleNode* next = node_list[0];

	int i = 0;

	while (next != NULL)
	{
		free_module_node(next);

		i++;

		next = node_list[i];
	}

	free(node_list);
	node_list = NULL;
}

static void free_module_tokens(Token* token_list)
{
	if (token_list == NULL)
	{
		return;
	}
	
	free(token_list);
	token_list = NULL;
}

static void free_module_parser(ModuleParser* parser)
{
	if (parser == NULL)
	{
		return;
	}
	
	free(parser);
	parser = NULL;
}

static void free_module_handler(ModuleHandler* handler)
{
	if (handler == NULL)
	{
		return;
	}
	
	free(handler->root_path);

	free(handler);
	handler = NULL;
}