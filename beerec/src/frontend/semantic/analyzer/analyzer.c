#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "analyzer.h"
#include "../../modules/modules.h"
#include "../../structure/parser/parser.h"
#include "../../modules/modules.h" 
#include "../symbols/symbols.h"
#include "../../../utils/utils.h"

int prototype_flag = -1;
int class_function_index = -1;

Symbol* analyzer_find_symbol_from_scope(const char* identifier, SymbolTable* scope, int is_variable, int is_function, int is_class, int is_module);
Type* analyzer_return_type_of_expression(Module* module, ASTNode* expression, SymbolTable* scope, ASTNodeList* args, int member_access, int* direct);
static Symbol* analyzer_add_symbol_to_scope(Module* module, ASTNode* node, SymbolTable* scope, int* offset, int prototype);
static ASTNode* analyzer_implictly_cast_all(Module* module, Type* preffered, ASTNode* expression, SymbolTable* scope);
static void analyzer_check_arguments(Module* module, ASTNode* params_head, ASTNode* args_head, SymbolTable* scope);
static void analyzer_analyze_node(Module* module, ASTNode* node, SymbolTable* scope, int* offset);
static int check_module_has_symbol(Module* module, char* identifier, SymbolType type);
static int analyzer_is_type_identic(Type* first, Type* second, SymbolTable* scope);
static ASTNode* analyzer_get_function_from_class(Symbol* class, char* func_name);
static ASTNode* analyzer_get_member_from_class(Symbol* class, char* member_name);
static int analyzer_is_class_assignable(Symbol* from, Symbol* to);
static ASTNode* _analyzer_create_cast(ASTNode** node, Type* preferred);
static void analyzer_create_cast(ASTNode** node, Type* preferred);
int analyzer_get_type_size(Type* type, SymbolTable* scope);
static int analyzer_is_inside_method(SymbolTable* scope);
int analyzer_get_list_size(ASTNode* list_head);

int class_count;

typedef struct
{
	const char* method_name;

	Type* access_type;
	Type* return_type;

	int type_acurracy;

	ASTNodeList* params;
}
PrototypeMethod;

int prototype_method_size = 0;

PrototypeMethod* protype_methods[100]; // tamanho pra 100 metodos prototype, trocar caso necessario.

static ASTNodeList* chain_nodes_to_list(ASTNode** nodes)
{
	ASTNodeList* node_list = malloc(sizeof(ASTNodeList));
	node_list->head = NULL;

	if (nodes == NULL)
	{
		return node_list;
	}

	ASTNode** current = &node_list->head;
	int i = 0;

	while (nodes[i] != NULL)
	{
		*current = nodes[i];
		current = &nodes[i]->next;
		i++;
	}

	*current = NULL;

	return node_list;
}

static ASTNode* analyzer_create_prototype_method_node(char* method_name, ASTNode** params, Type* return_type)
{
	ASTNode* node = malloc(sizeof(ASTNode));

	if (node == NULL)
	{
		printf("[Analyzer] [Debug] Failed to alloc memory for prototype method ASTNode...\n");
		exit(1);
	}

	node->type = NODE_FUNC;

	node->function.params = params == NULL ? NULL : chain_nodes_to_list(params);

	node->function.identifier = method_name;
	node->function.return_type = return_type;
	node->function.is_prototype = 1;
	node->function.is_constructor = 0;
	node->function.only_declaration = 0;
	node->function.is_static = 0;

	node->function.visibility = VISIBILITY_PUBLIC;
	
	return node;
}

static PrototypeMethod* analyzer_create_prototype_method(const char* method_name, Type* access_type, int type_acurracy, ASTNode* method_ref, Type* return_type, ASTNodeList* params)
{
	PrototypeMethod* prototype_method = malloc(sizeof(PrototypeMethod));

	if (prototype_method == NULL)
	{
		printf("[Analyzer] [Debug] Failed to alloc memory for prototype method struct...\n");
		exit(1);
	}

	prototype_method->access_type = access_type;
	prototype_method->method_name = method_name;
	prototype_method->type_acurracy = type_acurracy;
	prototype_method->return_type = return_type;
	prototype_method->params = params;

	return prototype_method;
}

static int check_already_has_symbol(Module* module, char* identifier, SymbolType type)
{
	Module* current = module->imported_modules[0];

	for (int i = 0; i < module->modules_count; i++)
	{
		if (check_module_has_symbol(current, identifier, type))
		{
			return 1;
		}
	}

	return 0;
}

static int check_module_has_symbol(Module* module, char* identifier, SymbolType type)
{
	if (module == NULL)
	{
		return 0;
	}
	
	SymbolTable* scope = module->global_scope;
	
	for (int i = 0; i < scope->count; i++)
	{
		Symbol* symbol = scope->symbols[i];

		switch (symbol->type)
		{
			case SYMBOL_FUNCTION:
			{
				if (strcmp(symbol->symbol_function->identifier, identifier) == 0 && type == SYMBOL_FUNCTION)
				{
					return 1;
				}

				break;
			}

			case SYMBOL_VARIABLE:
			{
				if (strcmp(symbol->symbol_variable->identifier, identifier) == 0 && type == SYMBOL_VARIABLE)
				{
					return 1;
				}

				break;
			}

			case SYMBOL_CLASS:
			{
				if (strcmp(symbol->symbol_class->identifier, identifier) == 0 && type == SYMBOL_CLASS)
				{
					return 1;
				}

				break;
			}

			default:
			{
				break;
			}
		}
	}

	return check_already_has_symbol(module, identifier, type);
}

static void analyzer_setup_prototype_methods(Module* module)
{
	{
		ASTNode* size_method = analyzer_create_prototype_method_node("size", NULL, create_type(TYPE_INT, NULL));
		
		Type* type = create_type(TYPE_ARRAY, NULL);
		Type* return_type = create_type(TYPE_INT, NULL);
		
		protype_methods[0] = analyzer_create_prototype_method("size", type, 1, size_method, return_type, NULL);
		prototype_method_size++;

		analyzer_add_symbol_to_scope(module, size_method, module->global_scope, NULL, 1);
	}
	{
		ASTNode* pop_method = analyzer_create_prototype_method_node("pop", NULL, create_type(TYPE_INT, NULL));
		
		Type* type = create_type(TYPE_ARRAY, NULL);
		Type* return_type = create_type(TYPE_INT, NULL);
		
		protype_methods[0] = analyzer_create_prototype_method("pop", type, 1, pop_method, return_type, NULL);
		prototype_method_size++;

		analyzer_add_symbol_to_scope(module, pop_method, module->global_scope, NULL, 1);
	}
}

static int analyzer_type_is_equals_acurracy(Type* first, Type* second, int acurracy)
{
	int i = 0;
	
	Type* next = first;
	Type* _next = second;

	while (i < acurracy)
	{
		if (next->type != _next->type)
		{
			return 0;
		}

		next = next->base;
		_next = _next->base;

		i++;
	}

	if (i != acurracy)
	{
		return 0;
	}

	return 1;
}

static int analyzer_matches_prototype(Type* from_type, PrototypeMethod* prototype_method, char* method_name)
{
	if (strcmp(prototype_method->method_name, method_name) != 0)
	{
		printf("[Analyzer] [Debug] Method names: %s, %s\n", prototype_method->method_name, method_name);
		
		return 0;
	}

	if (!analyzer_type_is_equals_acurracy(from_type, prototype_method->access_type, 1))
	{
		printf("[Analyzer] [Debug] Types doesn't match...\n");

		return 0;
	}

	return 1;
}

static PrototypeMethod* analyzer_is_prototype_call(Type* from_type, char* method_name)
{
	for (int i = 0; i < prototype_method_size; i++)
	{
		PrototypeMethod* method = protype_methods[i];

		if (analyzer_matches_prototype(from_type, method, method_name))
		{
			printf("[Analyzer] [Debug] Found a matching prototype method...\n");

			return method;
		}
	}

	return NULL;
}

static int align_offset(int offset)
{
	if (offset % 8 == 0)
	{
		return offset;
	}

	return ((offset / 8) + 1) * 8;
}

static SymbolTable* analyzer_setup_scope(SymbolType scope_kind, SymbolTable* parent, Symbol* owner_statement)
{
	SymbolTable* scope = malloc(sizeof(SymbolTable));
	
	scope->scope_kind = scope_kind;
	scope->count = 0;
	scope->parent = parent;
	scope->owner_statement = owner_statement;

	scope->symbols = malloc(sizeof(Symbol*) * 4);
	
	if (scope->symbols == NULL)
	{
		printf("[Analyzer] [Debug] Failed to allocate memory for symbols global scope array...\n");
		exit(1);
	}

	scope->capacity = 4;
	scope->symbols[0] = NULL;

	return scope;
}

void analyzer_init(Module* module, ASTNode** node_list)
{
	SymbolTable* scope = analyzer_setup_scope(GLOBAL_SCOPE, NULL, NULL);

	module->global_scope = scope;
	module->nodes = node_list;
	
	analyzer_setup_prototype_methods(module);
}

void analyzer_global_analyze(Module* module, ASTNode* ASTNode)
{
	analyzer_analyze_node(module, ASTNode, module->global_scope, NULL);
}

static int is_importing(Module* module, char* path)
{
	if (module == NULL)
	{
		return 0;
	}
	
	for (int i = 0; i < module->modules_count; i++)
	{
		Module* current = module->imported_modules[i];

		if (current == NULL)
		{
			exit(1);
		}

		if (strcmp(current->module_path, path) == 0)
		{
			return 1;
		}

		int is_import = is_importing(current, path);

		if (is_import)
		{
			return 1;
		}
	}

	return 0;
}

static void add_module_to_list(Module* module, Module* _module)
{
	if (module->modules_capacity <= (module->modules_count + 1)) // o '+ 1' é pro ultimo elemento 'NULL'.
	{
		module->modules_capacity *= 2;

		module->imported_modules = realloc(module->imported_modules, sizeof(Symbol*) * module->modules_capacity);

		if (module->imported_modules == NULL)
		{
			exit(1);
		}
	}

	module->imported_modules[module->modules_count] = _module;
	module->modules_count++;
	module->imported_modules[module->modules_count] = NULL; // ultimo elemento sempre é 'NULL'.
}

static void analyzer_add_export_symbol(Module* module, Symbol* symbol)
{
	if (module->exports_capacity <= (module->exports_count + 1)) // o '+ 1' é pro ultimo elemento 'NULL'.
	{
		module->exports_capacity *= 2;

		module->exports = realloc(module->exports, sizeof(Symbol*) * module->exports_capacity);

		if (module->exports == NULL)
		{
			exit(1);
		}
	}

	module->exports[module->exports_count] = symbol;
	module->exports_count++;
	module->exports[module->exports_count] = NULL; // ultimo elemento sempre é 'NULL'.
}

static int analyzer_get_class_size(Type* type, SymbolTable* scope)
{
	Symbol* class_symbol = analyzer_find_symbol_from_scope(type->class_name, scope, NULL, 0, 1, 0);

	if (class_symbol == NULL)
	{
		printf("[Analyzer] [Debug] Class not found in 'get_type_size()'...");
		exit(1);
	}

	int total_size = 0;

	Symbol* curr = class_symbol;

	while (curr != NULL)
	{
		for (int i = 0; i < curr->symbol_class->field_count; i++)
		{
			ASTNode* field = curr->symbol_class->fields[i];

			if (field->declare.is_static)
			{
				continue;
			}
			
			total_size += analyzer_get_type_size(field->declare.var_type, scope);
		}

		curr = curr->symbol_class->super;
	}

	return total_size;
}

int analyzer_get_type_size(Type* type, SymbolTable* scope)
{
	switch (type->type) 
	{
		case TYPE_BOOL:
		{
			return 1;
		}

		case TYPE_INT:
		{
			return 4;
		}

		case TYPE_FLOAT:
		{
			return 4;
		}

		case TYPE_DOUBLE:
		{
			return 8;
		}

		case TYPE_STRING:
		{
			return 8;
		}

		case TYPE_PTR:
		{
			return 8;
		}

		case TYPE_ANY_PTR:
		{
			return 8;
		}

		case TYPE_CHAR:
		{
			return 1;
		}

		case TYPE_ARRAY:
		{
			return 8; // 8 bytes pro ponteiro que vai apontar pra array na heap.
		}

		case TYPE_CLASS:
		{
			return 8; //analyzer_get_class_size(type, scope);
		}

		default:
		{
			printf("[Analyzer] [Debug] Invalid type...\n");
			exit(1);
		}
	}

	exit(1);
}

static int analyzer_is_numeric(Type* type)
{
	if (type->type == TYPE_DOUBLE)
	{
		return 1;
	}

	if (type->type == TYPE_FLOAT)
	{
		return 1;
	}

	if (type->type == TYPE_INT)
	{
		return 1;
	}

	return 0;
}

static SymbolTable* analyzer_create_scope(SymbolType scope_kind, SymbolTable* parent, Symbol* owner_statement)
{
	SymbolTable* scope = analyzer_setup_scope(scope_kind, parent, owner_statement);

	return scope;
}

static void _analyzer_add_symbol_to_scope(Symbol* symbol, SymbolTable* scope)
{
	if (symbol == NULL)
	{
		printf("[Analyzer] [Debug] Symbol param is null...\n");
		exit(1);
	}

	if (scope == NULL)
	{
		printf("[Analyzer] [Debug] Scope param is null...\n");
		exit(1);
	}
	
	if (scope->capacity <= scope->count + 1)
	{
		scope->capacity *= 2;
		
		scope->symbols = realloc(scope->symbols, sizeof(Symbol*) * scope->capacity);

		if (scope->symbols == NULL)
		{
			printf("[Analyzer] [Debug] Failed to realloc memory for scope's symbols array...\n");
			exit(1);
		}
	}

	scope->symbols[scope->count] = symbol;
	scope->count++;
	scope->symbols[scope->count] = NULL;
}

static Symbol* analyzer_create_variable_symbol(Module* module, ASTNode* ASTNode, SymbolTable* scope, int* offset)
{
	Symbol* symbol = malloc(sizeof(Symbol));

	if (symbol == NULL)
	{
		printf("[Analyzer] [Debug] Failed to alloc memory for symbol...\n");
		exit(1);
	}

	symbol->type = SYMBOL_VARIABLE;

	symbol->symbol_variable = malloc(sizeof(SymbolVariable));

	if (symbol->symbol_variable == NULL)
	{
		printf("[Analyzer] [Debug] Failed to alloc memory for symbol variable struct...\n");
		exit(1);
	}

	symbol->symbol_variable->is_global = (scope->scope_kind == GLOBAL_SCOPE);

	symbol->symbol_variable->type = ASTNode->declare.var_type;
	symbol->symbol_variable->identifier = ASTNode->declare.identifier;
	symbol->symbol_variable->is_const = ASTNode->declare.is_const;
	symbol->symbol_variable->is_static = ASTNode->declare.is_static;
	symbol->symbol_variable->is_class_global = 0;
	
	symbol->is_export = ASTNode->declare.is_export;

	if (symbol->is_export)
	{
		analyzer_add_export_symbol(module, symbol);
	}

	if (offset != NULL)
	{
		if (symbol->symbol_variable->is_global)
		{
			printf("[Analyzer] [Debug] Found a global variable with a valid offset: %d...\n", *offset);
			exit(1);
		}

		printf("[Analyzer] [Debug] Added a local variable with offset: %d...\n", *offset);

		symbol->symbol_variable->offset = *offset;
	}
	else if (offset == NULL && !symbol->symbol_variable->is_global && (scope->scope_kind != SYMBOL_CLASS))
	{
		printf("[Analyzer] [Debug] Found a local variable with a invalid offset...\n");
		exit(1);
	}

	_analyzer_add_symbol_to_scope(symbol, scope);

	return symbol;
}

static Symbol* analyzer_create_parameter_symbol(ASTNode* ASTNode, SymbolTable* scope, int* offset)
{
	Symbol* symbol = malloc(sizeof(Symbol));
	symbol->type = SYMBOL_VARIABLE;

	if (symbol == NULL)
	{
		printf("[Analyzer] [Debug] Failed to alloc memory for symbol struct (parameter)...\n");
		exit(1);
	}

	symbol->symbol_variable = malloc(sizeof(SymbolVariable));

	if (symbol->symbol_variable == NULL)
	{
		printf("[Analyzer] [Debug] Failed to alloc memory for symbol variable struct (parameter)...\n");
		exit(1);
	}

	symbol->symbol_variable->type = ASTNode->param.argument_type;
	symbol->symbol_variable->identifier = ASTNode->param.identifier;
	
	symbol->symbol_variable->is_const = 0;

	symbol->is_export = 0;
		
	if (offset != NULL)
	{
		printf("[Analyzer] [Debug] Added a param: \"%s\" with offset: %d...\n", symbol->symbol_variable->identifier, *offset);
	}
	else
	{
		printf("[Analyzer] [Debug] Found a param with a invalid offset...\n");
		exit(1);
	}
		
	symbol->symbol_variable->offset = *offset;
		
	symbol->symbol_variable->is_global = 0;
	symbol->symbol_variable->is_static = 0;
	symbol->symbol_variable->is_class_global = 0;
		
	_analyzer_add_symbol_to_scope(symbol, scope);

	return symbol;
}

static Symbol* analyzer_create_function_symbol(Module* module, ASTNode* ASTNode, SymbolTable* scope, int is_prototype)
{
	Symbol* symbol = malloc(sizeof(Symbol));
	symbol->type = SYMBOL_FUNCTION;

	if (symbol == NULL)
	{
		printf("[Analyzer] [Debug] Failed to alloc memory for symbol struct (function)...\n");
		exit(1);
	}

	symbol->symbol_function = malloc(sizeof(SymbolFunction));

	if (symbol->symbol_function == NULL)
	{
		printf("[Analyzer] [Debug] Failed to alloc memory for symbol function struct (function)...\n");
		exit(1);
	}

	symbol->symbol_function->return_type = ASTNode->function.return_type;
	symbol->symbol_function->identifier = ASTNode->function.identifier;
	symbol->symbol_function->is_virtual = ASTNode->function.is_virtual;
	symbol->symbol_function->is_override = ASTNode->function.is_override;
	symbol->symbol_function->is_static = ASTNode->function.is_static;

	symbol->is_export = ASTNode->function.is_export;

	if (symbol->is_export)
	{
		analyzer_add_export_symbol(module, symbol);
	}

	symbol->is_prototype = is_prototype;

	if (ASTNode->function.params != NULL)
	{
		symbol->symbol_function->params_head = ASTNode->function.params->head;
	}
	else
	{
		symbol->symbol_function->params_head = NULL;
	}
		
	_analyzer_add_symbol_to_scope(symbol, scope);

	return symbol;
}

static Symbol* analyzer_create_class_symbol(Module* module, ASTNode* ASTNode, SymbolTable* scope)
{
	Symbol* symbol = malloc(sizeof(Symbol));
	symbol->type = SYMBOL_CLASS;

	if (symbol == NULL)
	{
		printf("[Analyzer] [Debug] Failed to alloc memory for symbol struct (class)...\n");
		exit(1);
	}

	symbol->symbol_class = malloc(sizeof(SymbolClass));

	if (symbol->symbol_class == NULL)
	{
		printf("[Analyzer] [Debug] Failed to alloc memory for symbol class struct (class)...\n");
		exit(1);
	}

	symbol->symbol_class->class_scope = NULL;

	symbol->symbol_class->identifier = ASTNode->class_node.identifer;

	symbol->symbol_class->functions = ASTNode->class_node.funcs;
	symbol->symbol_class->fields = ASTNode->class_node.fields;

	symbol->symbol_class->func_count = ASTNode->class_node.funcs_count;
	symbol->symbol_class->field_count = ASTNode->class_node.fields_count;

	symbol->symbol_class->constructor_node = ASTNode->class_node.constructor;
	
	int init_size = 0;

	if (ASTNode->class_node.super_identifer != NULL)
	{
		Type* type = create_type(TYPE_CLASS, ASTNode->class_node.super_identifer);
		init_size += analyzer_get_type_size(type, scope);
	}

	symbol->symbol_class->total_offset = init_size;
	symbol->symbol_class->offset = init_size;

	symbol->symbol_class->class_id = class_count;

	class_count++;

	symbol->is_export = ASTNode->class_node.is_export;

	if (symbol->is_export)
	{
		analyzer_add_export_symbol(module, symbol);
	}

	symbol->symbol_class->super = NULL;
		
	_analyzer_add_symbol_to_scope(symbol, scope);

	return symbol;
}

static Symbol* analyzer_create_module_symbol(Module* module, ASTNode* ASTNode, SymbolTable* scope)
{
	Symbol* symbol = malloc(sizeof(Symbol));
	symbol->type = SYMBOL_MODULE;

	if (symbol == NULL)
	{
		printf("[Analyzer] [Debug] Failed to alloc memory for symbol struct (module)...\n");
		exit(1);
	}

	symbol->symbol_module = malloc(sizeof(SymbolModule));

	if (symbol->symbol_module == NULL)
	{
		printf("[Analyzer] [Debug] Failed to alloc memory for symbol module struct (module)...\n");
		exit(1);
	}

	symbol->symbol_module->identifier = ASTNode->class_node.identifer;

	symbol->symbol_module->module = NULL;
		
	_analyzer_add_symbol_to_scope(symbol, scope);

	return symbol;
}

static Symbol* analyzer_create_symbol(Module* module, SymbolType type, ASTNode* ASTNode, SymbolTable* scope, int is_param, int* offset, int is_prototype)
{
	if (type == SYMBOL_VARIABLE && !is_param)
	{
		return analyzer_create_variable_symbol(module, ASTNode, scope, offset);
	}

	if (type == SYMBOL_VARIABLE && is_param)
	{
		return analyzer_create_parameter_symbol(ASTNode, scope, offset);
	}

	if (type == SYMBOL_FUNCTION)
	{
		return analyzer_create_function_symbol(module, ASTNode, scope, is_prototype);
	}

	if (type == SYMBOL_CLASS)
	{
		return analyzer_create_class_symbol(module, ASTNode, scope);
	}

	if (type == SYMBOL_MODULE)
	{
		return analyzer_create_module_symbol(module, ASTNode, scope);
	}

	return NULL;
}

static Symbol* analyzer_add_symbol_to_scope(Module* module, ASTNode* ASTNode, SymbolTable* scope, int* offset, int prototype)
{
	switch (ASTNode->type) 
	{
		case NODE_FIELD:
		{
			return analyzer_create_symbol(module, SYMBOL_VARIABLE, ASTNode, scope, 0, offset, prototype);
		}

		case NODE_FUNC:
		{
			return analyzer_create_symbol(module, SYMBOL_FUNCTION, ASTNode, scope, 0, offset, prototype);
		}

		case NODE_PARAM:
		{
			return analyzer_create_symbol(module, SYMBOL_VARIABLE, ASTNode, scope, 1, offset, prototype);
		}

		case NODE_IMPORT:
		{
			return analyzer_create_symbol(module, SYMBOL_MODULE, ASTNode, scope, 0, offset, prototype);
		}

		default:
		{
			printf("[Analyzer] [Debug] ASTNode type not valid for a symbol...\n");
			exit(1);
		}
	}

	return NULL;
}

Symbol* analyzer_find_symbol_from_scope(const char* identifier, SymbolTable* scope, int is_variable, int is_function, int is_class, int is_module)
{
	if (scope == NULL) 
	{
		return NULL;
	}
	
	for (int i = 0; i < scope->count; i++) 
	{
		Symbol* next = scope->symbols[i];

		if (next->type == SYMBOL_FUNCTION && is_function) 
		{
			if (strcmp(identifier, next->symbol_function->identifier) == 0)
			{
				return next;
			}
		}

		if (next->type == SYMBOL_VARIABLE && is_variable) 
		{
			if (strcmp(identifier, next->symbol_variable->identifier) == 0)
			{
				return next;
			}
		}

		if (next->type == SYMBOL_CLASS && is_class) 
		{
			if (strcmp(identifier, next->symbol_class->identifier) == 0)
			{
				return next;
			}
		}
		
		if (next->type == SYMBOL_MODULE && is_module) 
		{
			if (strcmp(identifier, next->symbol_module->identifier) == 0)
			{
				return next;
			}
		}
	}

	return analyzer_find_symbol_from_scope(identifier, scope->parent, is_variable, is_function, is_class, is_module);
}

static Symbol* analyzer_find_symbol_only_from_scope(const char* identifier, SymbolTable* scope, int is_function, int is_class, int is_module)
{
	int i = 0;

	for (int i = 0; i < scope->count; i++) 
	{
		Symbol* next = scope->symbols[i];

		if (next->type == SYMBOL_FUNCTION && is_function) 
		{
			if (strcmp(identifier, next->symbol_function->identifier) == 0)
			{
				return next;
			}
		}
		if (next->type == SYMBOL_VARIABLE && !is_function) 
		{
			if (strcmp(identifier, next->symbol_variable->identifier) == 0)
			{
				return next;
			}
		}
		if (next->type == SYMBOL_CLASS && is_class) 
		{
			if (strcmp(identifier, next->symbol_class->identifier) == 0)
			{
				return next;
			}
		}
		if (next->type == SYMBOL_MODULE && is_module) 
		{
			if (strcmp(identifier, next->symbol_module->identifier) == 0)
			{
				return next;
			}
		}
	}

	return NULL;
}

static int analyzer_get_type_chain_size(Type* head)
{
	Type* next = head;

	int count = 0;

	while (next !=  NULL)
	{
		next = next->base;
		count++;
	}

	return count;
}

/**
 * Checa se o field 'type' de um struct 'Type' é igual a um 'VarType' específico.
 */
static int analyzer_is_equals(Type* type, VarType enum_type)
{
	if (type->type == enum_type)
	{
		return 1;
	}

	return 0;
}

/**
 * Checa se dois tipos são compativeis
 * 
 * Exemplo:
 *   INT e FLOAT --> compativeis, são numericos.
 *   INT e BOOL --> não são compativeis.
 */
static int analyzer_is_compatible(Type* first, Type* second, SymbolTable* scope)
{
	if (analyzer_is_numeric(first) && analyzer_is_numeric(second))
	{
		return 1;
	}

	if (analyzer_is_equals(first, TYPE_BOOL) && analyzer_is_equals(second, TYPE_BOOL))
	{
		return 1;
	}

	if (analyzer_is_equals(first, TYPE_STRING) && analyzer_is_equals(second, TYPE_STRING))
	{
		return 1;
	}

	if (analyzer_is_equals(first, TYPE_CHAR) && analyzer_is_equals(second, TYPE_CHAR))
	{
		return 1;
	}

	if (first->type == TYPE_CLASS && second->type == TYPE_CLASS) 
	{
		Symbol* class = analyzer_find_symbol_from_scope(first->class_name, scope, 0, 0, 1, 0);
		Symbol* _class = analyzer_find_symbol_from_scope(second->class_name, scope, 0, 0, 1, 0);

		if (class == NULL || _class == NULL)
		{
			printf("[Analyzer] [Debug] Failed to find class symbols...");
			exit(1);
		}
		
		return analyzer_is_class_assignable(class, _class);
	}

	if (analyzer_is_equals(first, TYPE_PTR) || analyzer_is_equals(second, TYPE_PTR))
	{
		if (analyzer_is_type_identic(first, second, scope))
		{
			return 1;
		}
		else
		{
			printf("[Analyzer] [Debug] Pointers type aren't the same...\n");
		}
	}

	if (analyzer_is_equals(first, TYPE_ARRAY) || analyzer_is_equals(second, TYPE_ARRAY))
	{
		if (analyzer_is_type_identic(first, second, scope))
		{
			return 1;
		}
		else
		{
			printf("[Analyzer] [Debug] Arrays type aren't the same...\n");
		}
	}

	return 0;
}

/**
 * Checa se dois tipos são iguais (se o tipo for uma classe e as duas classes forem assignable, então é valido)
 */
static int analyzer_is_type_identic(Type* first, Type* second, SymbolTable* scope)
{
	if (analyzer_get_type_chain_size(first) != analyzer_get_type_chain_size(second))
	{
		return 0;
	}

	Type* type = first;
	Type* _type = second;

	while (type != NULL)
	{
		if (type->type != _type->type)
		{
			return 0;
		}

		if (type->type == TYPE_CLASS)
		{
			if (!analyzer_is_compatible(type, _type, scope))
			{
				return 0;
			}
		}

		type = type->base;
		_type = _type->base;
	}

	return 1;
}

/**
 * Checa se dois tipos são exatamente iguais (até mesmo se as classes forem assignable mas forem diferentes)
 */
static int analyzer_is_the_same(Type* first, Type* second)
{
	Type* base = first;
	Type* _base = second;

	while (base != NULL)
	{
		if (base->type != _base->type)
		{
			return 0;
		}

		if (base->type == TYPE_CLASS)
		{
			if (strcmp(base->class_name, _base->class_name) != 0)
			{
				return 0;
			}
		}

		base = base->base;
		_base = _base->base;
	}

	return (base == NULL && _base == NULL);
}

/**
 * Pega o tipo de maior precedencia numa expressão.
 */
static Type* analyzer_get_higher(Type* first, Type* second)
{
	if (analyzer_is_equals(first, TYPE_DOUBLE) || analyzer_is_equals(second, TYPE_DOUBLE)) 
	{
		return create_type(TYPE_DOUBLE, NULL);
	}

	if (analyzer_is_equals(first, TYPE_FLOAT) || analyzer_is_equals(second, TYPE_FLOAT)) 
	{
		return create_type(TYPE_FLOAT, NULL);
	}

	if (analyzer_is_equals(first, TYPE_INT) || analyzer_is_equals(second, TYPE_INT)) 
	{
		return create_type(TYPE_INT, NULL);
	}

	return first;
}

/**
 * Pega o escopo de classe mais proximo do escopo atual.
 */
static SymbolTable* analyzer_get_class_scope(SymbolTable* scope)
{
	if (scope == NULL)
	{
		return NULL;
	}

	if (scope->scope_kind == SYMBOL_CLASS)
	{
		return scope;
	}

	return analyzer_get_class_scope(scope->parent);
}

static Type* analyzer_handle_prototype(Type* object_type, char* function_name, PrototypeMethod** ref)
{
	PrototypeMethod* prototype = analyzer_is_prototype_call(object_type, function_name);

	if (prototype == NULL)
	{
		return NULL;
	}

	*ref = prototype;

	return prototype->return_type;
}

static int is_class_object(Type* type, int ptr_access)
{
	Type* ref = ptr_access ? type->base : type;
	
	if (ref == NULL)
	{
		return 0;
	}
	
	if (ref->type != TYPE_CLASS)
	{
		return 0;
	}

	return 1;
}

/**
 * Checa se é possivel acessar um membro static de uma class.
 */
static void analyzer_check_directly(int direct, ASTNode* member, ASTNodeType type)
{
	if (type == NODE_FIELD)
	{
		if (direct && !member->declare.is_static)
		{
			printf("[Analyzer] [Debug] Can't access a non static field directly: %s...\n", member->declare.identifier);
			exit(1);
		}

		if (!direct && member->declare.is_static)
		{
			printf("[Analyzer] [Debug] Can't access a static field from a instance: %s...\n", member->declare.identifier);
			exit(1);
		}
	}
	else if (type == NODE_FUNC)
	{
		if (direct && !member->function.is_static)
		{
			printf("[Analyzer] [Debug] Can't access a non static function directly...\n");
			exit(1);
		}

		if (!direct && member->function.is_static)
		{
			printf("[Analyzer] [Debug] Can't access a static function from a instance...\n");
			exit(1);
		}
	}
}

static Type* handle_function(Symbol* class_symbol, char* field_name, ASTNode* node, Module* module, SymbolTable* scope, int directly, ASTNodeList* args)
{
	ASTNode* member = analyzer_get_function_from_class(class_symbol, field_name);
		
	if (member == NULL)
	{
		return NULL;
	}

	if (member->type == NODE_FUNC)
	{
		if (member->function.visibility != VISIBILITY_PUBLIC)
		{
			printf("[Analyzer] [Debug] Trying to access a private function: \"%s\"...\n", field_name);
			return NULL;
		}

		if (directly)
		{
			ASTNode* direct_node = malloc(sizeof(ASTNode));
			
			direct_node->type = NODE_STATIC_ACCESS;
			direct_node->static_member_access.class_symbol = class_symbol;

			node->member_access.object = direct_node;
		}

		analyzer_check_directly(directly, member, NODE_FUNC);
	
		Type* field_type = analyzer_return_type_of_expression(module, member, class_symbol->symbol_class->class_scope, args, 1, NULL);

		ASTNode* params_head = member->function.params == NULL ? NULL : member->function.params->head;
		ASTNode* args_head = (args == NULL) ? NULL : args->head;
			
		analyzer_check_arguments(module, params_head, args_head, scope);

		if (field_type == NULL)
		{
			return NULL;
		}
		
		return field_type;
	}
}

static int get_prototype_id(const char* name)
{
	if (strcmp(name, "pop") == 0)
	{
		return 0;
	}

	return -1;
}

static Type* handle_prototype_method(ASTNode** node, Type* type, char* field_name, Module* module, SymbolTable* scope, ASTNodeList* args)
{
	printf("[Analyzer] [Debug] Maybe found a prototype function call...\n");

	PrototypeMethod* ref = NULL;

	Type* function_type = analyzer_handle_prototype(type, field_name, &ref);

	ASTNode* params_head = ref->params == NULL ? NULL : ref->params->head;
	ASTNode* args_head = (args == NULL) ? NULL : args->head;
		
	analyzer_check_arguments(module, params_head, args_head, scope);

	if (function_type == NULL)
	{
		return NULL;
	}

	prototype_flag = get_prototype_id(ref->method_name);

	return function_type;
}

static Type* handle_module_class_directly_access(Symbol* module_symbol, SymbolTable* scope, const char* module_identifier, char* field_name)
{
	SymbolTable* module_scope = module_symbol->symbol_module->module->global_scope;
	Symbol* class_symbol = analyzer_find_symbol_from_scope(field_name, module_scope, 0, 0, 1, 0);

	if (class_symbol == NULL)
	{
		return NULL;
	}
	
	/**
	 * TODO: Checar se o modulo ta exportando a class.
	 */

	Type* type = create_type(TYPE_CLASS, (char*) class_symbol->symbol_class->identifier);
	type->class_symbol = class_symbol;

	return type;
}

static Type* handle_module_field_access(Module* module, SymbolTable* scope, const char* module_identifier, char* field_name)
{
	Symbol* module_symbol = analyzer_find_symbol_from_scope(module_identifier, scope, 0, 0, 0, 1);
	
	if (module_symbol == NULL)
	{
		return NULL;
	}

	SymbolTable* module_scope = module_symbol->symbol_module->module->global_scope;
	Symbol* field_symbol = analyzer_find_symbol_from_scope(field_name, module_scope, 1, 0, 0, 0);

	if (field_symbol == NULL)
	{
		return handle_module_class_directly_access(module_symbol, scope, module_identifier, field_name);
	}
	
	/**
	 * TODO: hecar se o modulo ta exportando a field.
	 */

	return field_symbol->symbol_variable->type;
}

static Type* handle_module_method_access(Module* module, SymbolTable* scope, const char* module_identifier, char* method_name)
{
	Symbol* module_symbol = analyzer_find_symbol_from_scope(module_identifier, scope, 0, 0, 0, 1);
	
	if (module_symbol == NULL)
	{
		return NULL;
	}

	SymbolTable* module_scope = module_symbol->symbol_module->module->global_scope;
	Symbol* field_symbol = analyzer_find_symbol_from_scope(method_name, module_scope, 0, 1, 0, 0);

	if (field_symbol == NULL)
	{
		return NULL;
	}
	
	/**
	 * TODO: Checar se o modulo ta exportando o metodo.
	 */

	return field_symbol->symbol_function->return_type;
}

/**
 * Pega o tipo de retorno de um acesso de membro de uma class.
 */
Type* analyzer_get_member_access_type(Module* module, ASTNode* node, SymbolTable* scope, ASTNodeList* args)
{
	int directly = 0;
	
	Type* type = analyzer_return_type_of_expression(module, node->member_access.object, scope, args, 1, &directly);
	char* field_name = node->member_access.member_name;

	int is_ptr = node->member_access.ptr_acess;
	
	int is_module = (type->type == TYPE_MODULE);

	const char* class_name = type->class_name;
	
	if (type->type == TYPE_PTR && !is_ptr)
	{
		printf("[Analyzer] [Debug] Use '->' to access pointers pointing to a class...\n");

		return NULL;
	}

	if (is_module && is_ptr)
	{
		printf("[Analyzer] [Debug] Use '.' to access modules...\n");

		return NULL;
	}

	if (type->type == TYPE_CLASS && is_ptr)
	{
		printf("[Analyzer] [Debug] Use '.' to access normal classes...\n");

		return NULL;
	}

	if (type->type != TYPE_CLASS && type->type != TYPE_PTR && is_ptr)
	{
		printf("[Analyzer] [Debug] Use '.' to access prototype methods...\n");

		return NULL;
	}

	if (node->member_access.ptr_acess)
	{
		class_name = type->base->class_name;
	}

	if (node->member_access.object->type == NODE_THIS)
	{
		printf("[Analyzer [Debug] Using 'this' pointer...\n");
	}

	if (node->member_access.object->type == NODE_SUPER)
	{
		printf("[Analyzer [Debug] Using 'super'...\n");
	}

	if (is_module)
	{
		if (node->member_access.is_function)
		{
			return handle_module_method_access(module, scope, class_name, field_name);
		}

		return handle_module_field_access(module, scope, class_name, field_name);
	}

	int is_class = is_class_object(type, is_ptr);		

	if (type == NULL)
	{
		return NULL;
	}

	if (node->member_access.is_function && !is_class)
	{
		return handle_prototype_method(&node, type, field_name, module, scope, args);
	}

	if (!is_class)
	{
		return NULL;
	}
			
	Symbol* class_symbol = analyzer_find_symbol_from_scope(class_name, scope, 0, 0, 1, 0);
	
	if (class_symbol == NULL)
	{
		class_symbol = type->class_symbol;
		directly = 1;
	}

	if (class_symbol == NULL)
	{
		return NULL;
	}

	if (node->member_access.object->type == NODE_STATIC_ACCESS)
	{
		directly = 1;
	}

	if (node->member_access.is_function)
	{
		return handle_function(class_symbol, field_name, node, module, scope, directly, args);
	}

	ASTNode* member = analyzer_get_member_from_class(class_symbol, field_name);
	
	if (member == NULL)
	{
		return NULL;
	}

	if (member->type == NODE_FIELD)
	{
		analyzer_check_directly(directly, member, NODE_FIELD);

		if (directly)
		{
			ASTNode* direct_node = malloc(sizeof(ASTNode));
			direct_node->type = NODE_STATIC_ACCESS;
			direct_node->static_member_access.class_symbol = class_symbol;

			node->member_access.object = direct_node;
		}
		
		if (member->declare.visibility != VISIBILITY_PUBLIC)
		{
			printf("[Analyzer] [Debug] Trying to access a private field: \"%s\"...\n", field_name);
			
			return NULL;
		}
	}

	Type* field_type = analyzer_return_type_of_expression(module, member, class_symbol->symbol_class->class_scope, args, 1, NULL);

	if (field_type == NULL)
	{
		return NULL;
	}

	return field_type;
}

/**
 * Retorna o tipo retornado por uma operação
 */
static Type* analyzer_get_operation_type(Module* module, ASTNode* node, SymbolTable* scope)
{
	Type* left = analyzer_return_type_of_expression(module, node->operation.left, scope, NULL, 0, NULL);
	Type* right = analyzer_return_type_of_expression(module, node->operation.right, scope, NULL, 0, NULL);

	switch (node->operation.op)
	{		
		case TOKEN_OPERATOR_DIVIDED_EQUALS: // /=
		case TOKEN_OPERATOR_TIMES_EQUALS: // *=
		case TOKEN_OPERATOR_MINUS_EQUALS: // -=
		case TOKEN_OPERATOR_PLUS_EQUALS: // +=
		case TOKEN_OPERATOR_DIVIDED: // /
		case TOKEN_CHAR_STAR: // "*" multiply
		case TOKEN_OPERATOR_MINUS: // -
		case TOKEN_OPERATOR_PLUS: // +
		case TOKEN_OPERATOR_DECREMENT: // --
		case TOKEN_OPERATOR_INCREMENT: // ++
		{
			return analyzer_get_higher(left, right);
		}
				
		case TOKEN_OPERATOR_LESS: // <
		case TOKEN_OPERATOR_LESS_EQUALS: // <=
		case TOKEN_OPERATOR_GREATER_EQUALS: // >=
		case TOKEN_OPERATOR_GREATER: // >
		case TOKEN_OPERATOR_EQUALS: // ==
		case TOKEN_OPERATOR_NOT_EQUALS:
		case TOKEN_OPERATOR_OR: // ||
		case TOKEN_OPERATOR_AND: // &&
		{
			return create_type(TYPE_BOOL, NULL);
		}

		default:
		{
			printf("[Analyzer] [Debug] Invalid operation operator (type_of_expression)...\n");
			exit(1);
		}
	}
}

/**
 * Retorna a função dona do escopo de função mais proximo do 'scope'.
 */
static Symbol* analyzer_get_owner_function(SymbolTable* scope)
{
	if (scope == NULL)
	{
		return NULL;
	}

	if (scope->scope_kind == SYMBOL_FUNCTION)
	{
		return scope->owner_statement;
	}

	return analyzer_get_owner_function(scope->parent);
}

/**
 * Retorna se o 'scope' ta dentro do escopo de uma class.
 */
static int analyzer_is_inside_class(SymbolTable* scope)
{
	if (scope == NULL)
	{
		return 0;
	}

	if (scope->scope_kind == SYMBOL_CLASS)
	{
		return 1;
	}

	return analyzer_is_inside_class(scope->parent);
}

/**
 * Retorna o escopo da class que o scope ta dentro (NULL se não ta dentro do escopo de uma class).
 */
static Symbol* analyzer_is_inside_class_alt(SymbolTable* scope)
{
	if (scope == NULL)
	{
		return NULL;
	}

	if (scope->scope_kind == SYMBOL_CLASS)
	{
		return scope->owner_statement;
	}

	return analyzer_is_inside_class_alt(scope->parent);
}

/**
 * Lida com call do constructor da super class.
 */
static Type* handle_super_flat_call(Module* module, SymbolTable* scope, ASTNode* callee, ASTNodeList* args)
{
	Symbol* class = analyzer_is_inside_class_alt(scope);
	
	if (class == NULL)
	{
		exit(1);
	}

	Symbol* super = class->symbol_class->super;

	if (super == NULL)
	{
		exit(1);
	}

	if (super->symbol_class->constructor == NULL)
	{
		exit(1);
	}
	
	ASTNode* params_head = super->symbol_class->constructor->symbol_function->params_head;

	if (params_head == NULL && args != NULL)
	{
		printf("Super constructor dont have params...\n");
		exit(1);
	}
	else if (params_head != NULL)
	{
		if (args == NULL)
		{
			printf("Super needs arguments...\n");
			exit(1);
		}

		ASTNode* args_head = args->head;
	
		analyzer_check_arguments(module, params_head, args_head, scope);
	}
	
	return create_type(TYPE_VOID, NULL);
}

static Type* handle_flat_method(Module* module, SymbolTable* scope, ASTNode* callee, ASTNodeList* args)
{
	if (callee->type == NODE_SUPER)
	{
		return handle_super_flat_call(module, scope, callee, args);
	}
	
	Symbol* symbol = analyzer_find_symbol_from_scope(callee->variable.identifier, scope, 0, 1, 0, 0);
		
	if (symbol == NULL)
	{
		return NULL;
	}

	Symbol* owner_function = analyzer_get_owner_function(scope);

	if (analyzer_is_inside_class(scope))
	{
		if (owner_function->symbol_function->is_static && !symbol->symbol_function->is_static)
		{
			printf("[Analyzer] [Debug] Calling a function from a static scope...\n");
			exit(1);
		}
	}

	ASTNode* params_head = symbol->symbol_function->params_head;
	ASTNode* args_head = (args == NULL) ? NULL : args->head;

	analyzer_check_arguments(module, params_head, args_head, scope);

	return symbol->symbol_function->return_type;
}

static void check_built_in_args(Module* module, SymbolTable* scope, int method_id, ASTNodeList* args)
{
	switch (method_id)
	{
		case 0: // print
		{
			int size = analyzer_get_list_size((args == NULL) ? NULL : args->head);

			if (size != 1)
			{
				exit(1);
			}

			Type* type = analyzer_return_type_of_expression(module, args->head, scope, NULL, 0, 0);

			if (type->type != TYPE_STRING)
			{
				exit(1);
			}

			return;
		}
	}
}

static Type* handle_built_in_methods(Module* module, SymbolTable* scope, ASTNode* node, ASTNodeList* args)
{
	ASTNode* callee = node->function_call.callee;
	
	if (strcmp(callee->variable.identifier, "print") == 0)
	{
		check_built_in_args(module, scope, 0, args);

		node->function_call.built_in_id = 0;
		node->function_call.is_built_in = 1;

		return create_type(TYPE_VOID, NULL);
	}

	return NULL;
}

/**
 * Pega o retorno da chamada de uma função.
 */
static Type* analyzer_get_function_call_type(Module* module, ASTNode* node, SymbolTable* scope, ASTNodeList* args)
{
	args = node->function_call.arguments;
	ASTNode* callee = node->function_call.callee;

	node->function_call.built_in_id = -1;
	node->function_call.is_built_in = 0;

	if (callee->type == NODE_IDENT || callee->type == NODE_SUPER)
	{
		if (callee->type != NODE_SUPER)
		{
			Type* built_in = handle_built_in_methods(module, scope, node, args);
	
			if (built_in != NULL)
			{
				return built_in;
			}
		}

		return handle_flat_method(module, scope, callee, args); // flat method porque não é de class ou modulo, e sim uma função global normal.
	}

	Type* type = analyzer_return_type_of_expression(module, callee, scope, args, 0, NULL);
			
	if (type == NULL)
	{
		return NULL;
	}

	if (prototype_flag != -1)
	{
		node->function_call.is_prototype = 1;
		node->function_call.prototype_id = prototype_flag;
		prototype_flag = -1;
	}
	
	return type;
}

static Type* analyzer_get_dereference_type(Module* module, ASTNode* node, SymbolTable* scope)
{
	Type* type = NULL;

	ASTNode* ref = node;
	ASTNode* _ref = node;
			
	while (_ref->type == NODE_DEREFERENCE)
	{
		_ref = _ref->dereference.expr;
	}
			
	if (_ref->type == NODE_IDENT)
	{
		type = analyzer_return_type_of_expression(module, _ref, scope, NULL, 0, NULL);
	}
			
	int depth = 0;
			
	while (ref->type == NODE_DEREFERENCE)
	{
		ref = ref->dereference.expr;
		depth++;
	}
			
	for (int i = 0; i < depth; i++)
	{
		if (type->base == NULL)
		{
			printf("[Analyzer] [Debug] Deferencing more than the pointer's depth...\n");
			exit(1);
		}
				
		type = type->base;
	}
			
	return type;
}

/**
 * Retorna o tipo de retorno do acesso de uma array.
 */
static Type* analyzer_get_array_access_type(Module* module, ASTNode* node, SymbolTable* scope)
{
	ASTNode* index = node->acess_array.index_expr;

	Type* type = analyzer_return_type_of_expression(module, node->acess_array.array, scope, NULL, 0, NULL);

	if (type == NULL)
	{
		return NULL;
	}

	Type* index_type = analyzer_return_type_of_expression(module, index, scope, NULL, 0, NULL);

	if (type->type != TYPE_ARRAY)
	{
		return NULL;
	}

	if (type->base == NULL)
	{
		return NULL;
	}

	analyzer_analyze_node(module, index, scope, NULL);

	if (index_type == NULL)
	{
		return NULL;
	}

	if (!analyzer_is_numeric(index_type))
	{
		return NULL;
	}

	analyzer_implictly_cast_all(module, create_type(TYPE_INT, NULL), index, scope);

	return type->base;
}

/**
 * Retorna o tipo de retorno do ponteiro implicito 'this'.
 */
static Type* analyzer_get_this_type(Module* module, ASTNode* ASTNode, SymbolTable* scope)
{
	analyzer_analyze_node(module, ASTNode, scope, NULL);
			
	// cast fudido (só pra não dar warning)
	char* identifier = (char*) (analyzer_get_class_scope(scope)->owner_statement->symbol_class->identifier);
	Type* type = create_type(TYPE_CLASS, identifier);

	return type;
}

static Type* analyzer_super_type(Module* module, ASTNode* ASTNode, SymbolTable* scope)
{
	analyzer_analyze_node(module, ASTNode, scope, NULL);
			
	// cast fudido (só pra não dar warning)
	char* identifier = (char*) (analyzer_get_class_scope(scope)->owner_statement->symbol_class->super->symbol_class->identifier);
	Type* type = create_type(TYPE_CLASS, identifier);

	return type;
}

static Type* analyzer_get_create_instance_type(ASTNode* node, SymbolTable* scope)
{
	Type* type = create_type(TYPE_CLASS, node->create_instance.class_name);

	return type;
}

static Type* analyzer_get_adress_of_type(Module* module, ASTNode* node, SymbolTable* scope)
{
	Type* inner = analyzer_return_type_of_expression(module, node->adress_of.expr, scope, NULL, 0, NULL);
	
	Type* ptr = create_type(TYPE_PTR, NULL);
	ptr->base = inner;
			
	return ptr;
}

static Symbol* get_class_owner(SymbolTable* scope)
{
	if (scope == NULL)
	{
		return NULL;
	}

	if (scope->scope_kind == SYMBOL_CLASS)
	{
		return scope->owner_statement;
	}

	return get_class_owner(scope->parent);
}

static Type* analyzer_get_identifier_type(ASTNode* node, SymbolTable* scope, int member_access, int* direct)
{
	Symbol* symbol = analyzer_find_symbol_from_scope(node->variable.identifier, scope, 1, 0, 1, 1);

	SymbolTable* curr_scope = scope;
	
	while (symbol == NULL)
	{
		if (curr_scope == NULL)
		{
			break;
		}
		
		Symbol* class_owner = get_class_owner(curr_scope);

		if (class_owner == NULL)
		{
			break;
		}
		
		symbol = analyzer_find_symbol_from_scope(node->variable.identifier, class_owner->symbol_class->class_scope, 1, 0, 0, 0);

		if (class_owner->symbol_class->super == NULL)
		{
			break;
		}

		curr_scope = class_owner->symbol_class->super->symbol_class->class_scope;
	}
	
	if (symbol == NULL)
	{
		printf("[Analyzer] [Debug] Variable not declared: %s...\n", node->variable.identifier);
		exit(1);
	}
	
	if (symbol->type == SYMBOL_CLASS)
	{	
		printf("[Analyzer] [Debug] Accessing directly: \"%s\"...\n", node->variable.identifier);

		*direct = 1;

		return create_type(TYPE_CLASS, (char*) symbol->symbol_class->identifier);
	}

	if (symbol->type == SYMBOL_MODULE)
	{
		printf("[Analyzer] [Debug] Accessing a module: \"%s\"...\n", node->variable.identifier);
		
		Type* module_type = create_type(TYPE_MODULE, symbol->symbol_module->identifier);

		return module_type;
	}

	if (!member_access)
	{
		Symbol* owner_function = analyzer_get_owner_function(scope);

		if (analyzer_is_inside_class(scope))
		{
			if (owner_function->symbol_function->is_static && !symbol->symbol_variable->is_static)
			{
				printf("[Analyzer] [Debug] Using a static field from a static scope...\n");
				exit(1);
			}
		}
	}

	return symbol->symbol_variable->type;
}

static Type* analyzer_get_argument_type(Module* module, ASTNode* node, SymbolTable* scope)
{
	return analyzer_return_type_of_expression(module, node->argument.value, scope, NULL, 0, NULL);
}

static Type* analyzer_get_parameter_type(ASTNode* node, SymbolTable* scope)
{
	return node->param.argument_type;
}

static int analyzer_is_inside_method(SymbolTable* scope)
{
	if (scope == NULL)
	{
		return 0;
	}

	if (scope->scope_kind == SYMBOL_FUNCTION)
	{
		return 1;
	}

	return analyzer_is_inside_method(scope);
}

static Type* analyzer_get_super_type(ASTNode* ASTNode, SymbolTable* scope)
{
	Symbol* class = analyzer_is_inside_class_alt(scope);

	if (class == NULL)
	{
		exit(1);
	}

	if (!analyzer_is_inside_method(scope))
	{
		exit(1);
	}

	Symbol* super = class->symbol_class->super;
	
	if (super == NULL)
	{
		exit(1);
	}

	// cast podre, só pra não dar warning
	return create_type(TYPE_CLASS, (char*) super->symbol_class->identifier);
}

Type* analyzer_return_type_of_expression(Module* module, ASTNode* expression, SymbolTable* scope, ASTNodeList* args, int member_access, int* direct)
{
	if (expression == NULL)
	{
		printf("[Analyzer] [Debug] Expression is null...\n");
		exit(1);
	}

	switch (expression->type) 
	{
		case NODE_STATIC_ACCESS:
		{
			return create_type(TYPE_CLASS, (char*) expression->static_member_access.class_symbol->symbol_class->identifier);
		}
		
		case NODE_OPERATION:
		{
			return analyzer_get_operation_type(module, expression, scope);
		}

		case NODE_LITERAL:
		{
			return expression->literal.literal_type;
		}

		case NODE_IDENT:
		{
			return analyzer_get_identifier_type(expression, scope, member_access, direct);
		}

		case NODE_CAST:
		{
			return expression->cast_node.cast_type;
		}
		
		case NODE_REFERENCE:
		{
			return analyzer_get_adress_of_type(module, expression, scope);
		}
		
		case NODE_DEREFERENCE:
		{
			return analyzer_get_dereference_type(module, expression, scope);
		}
		
		case NODE_FIELD:
		{
			return expression->declare.var_type;
		}
		
		case NODE_CREATE_INST:
		{
			return analyzer_get_create_instance_type(expression, scope);
		}
		
		case NODE_THIS:
		{
			return analyzer_get_this_type(module, expression, scope);
		}

		case NODE_FUNC:
		{
			return expression->function.return_type;
		}
		
		case NODE_CALL:
		{
			return analyzer_get_function_call_type(module, expression, scope, args);
		}
		
		case NODE_MEMBER_ACCESS:
		{
			return analyzer_get_member_access_type(module, expression, scope, args);
		}

		case NODE_ARR_ACCESS:
		{
			return analyzer_get_array_access_type(module, expression, scope);
		}

		case NODE_ARGUMENT:
		{
			return analyzer_get_argument_type(module, expression, scope);
		}

		case NODE_PARAM:
		{
			return analyzer_get_parameter_type(expression, scope);
		}

		case NODE_SUPER:
		{
			return analyzer_get_super_type(expression, scope);
		}

		case NODE_ARR_LITERAL:
		{
			return expression->array_literal.array_type;
		}
		
		default:
		{
			printf("[Analyzer] [Debug] Invalid expression type: %d...\n", expression->type);
			exit(1);
		}
	}

	return NULL;
}

static int analyzer_is_operation_compatible(Module* module, ASTNode* node, SymbolTable* scope)
{
	ASTNodeOperation* operation = &node->operation;

	Type* left = analyzer_return_type_of_expression(module, operation->left, scope, NULL, 0, NULL);
	Type* right = analyzer_return_type_of_expression(module, operation->right, scope, NULL, 0, NULL);

	switch (operation->op) 
	{
		case TOKEN_OPERATOR_OR: // ||
		case TOKEN_OPERATOR_AND: // &&
		{
			if (analyzer_is_equals(left, TYPE_BOOL) && analyzer_is_equals(right, TYPE_BOOL))
			{
				return 1;
			}

			break;
		}
		
		case TOKEN_OPERATOR_DIVIDED_EQUALS: // /=
		case TOKEN_OPERATOR_TIMES_EQUALS: // *=
		case TOKEN_OPERATOR_MINUS_EQUALS: // -=
		case TOKEN_OPERATOR_PLUS_EQUALS: // +=
		case TOKEN_OPERATOR_DIVIDED: // /
		case TOKEN_CHAR_STAR: // "*" multiply
		case TOKEN_OPERATOR_MINUS: // -
		case TOKEN_OPERATOR_PLUS: // +
		case TOKEN_OPERATOR_INCREMENT: // ++
		case TOKEN_OPERATOR_DECREMENT: // --
		{
			if (analyzer_is_compatible(left, right, scope))
			{
				return 1;
			}
		}
		
		case TOKEN_OPERATOR_LESS: // <
		case TOKEN_OPERATOR_LESS_EQUALS: // <=
		case TOKEN_OPERATOR_GREATER_EQUALS: // >=
		case TOKEN_OPERATOR_GREATER: // >
		case TOKEN_OPERATOR_EQUALS: // ==
		case TOKEN_OPERATOR_NOT_EQUALS:
		{
			if (analyzer_is_compatible(left, right, scope))
			{
				return 1;
			}

			break;
		}

		default:
		{
			printf("[Analyzer] [Debug] Invalid operation operator...\n");
			exit(1);
		}
	}

	return 0;
}

static ASTNode* analyzer_implictly_cast_all(Module* module, Type* preffered, ASTNode* expression, SymbolTable* scope)
{
	Type* type = analyzer_return_type_of_expression(module, expression, scope, NULL, 0, NULL);

	if ((type->type != TYPE_PTR && preffered->type != TYPE_PTR) && (type->type != TYPE_ARRAY && preffered->type != TYPE_ARRAY))
	{
		if (type->type == preffered->type)
		{
			printf("[Analyzer] [Debug] Not casting, types are the same...\n");

			return expression;
		}
	}

	if (!analyzer_is_compatible(type, preffered, scope))
	{
		printf("[Analyzer] [Debug] Expression type is incompatible...\n");
		exit(1);
	}

	if (!analyzer_is_type_identic(type, preffered, scope))
	{
		return _analyzer_create_cast(&expression, preffered);
	}
	else
	{
		printf("[Analyzer] [Debug] Not casting, types are the same...\n");

		return expression;
	}
}

static void analyzer_analyze_type(Type* type, SymbolTable* scope)
{
	if (type == NULL)
	{
		return;
	}
	
	if (type->type == TYPE_CLASS)
	{
		if (type->class_name == NULL)
		{
			printf("[Analyzer] [Debug] Class name is null...");
			exit(1);
		}
		
		if (analyzer_find_symbol_from_scope(type->class_name, scope, 0, 0, 1, 0) == NULL)
		{
			printf("[Analyzer] [Debug] Class of type: \"%s\" not found...\n", type->class_name);
			exit(1);
		}
	}

	analyzer_analyze_type(type->base, scope);
}

static void analyzer_handle_variable_declaration(Module* module, ASTNode* node, SymbolTable* scope, int* offset)
{ 
	if (analyzer_find_symbol_only_from_scope(node->declare.identifier, scope, 0, 0, 1) != NULL)
	{
		printf("[Analyzer] [Debug] Statement with identifier already declared in scope...\n");
		exit(1);
	}

	analyzer_analyze_type(node->declare.var_type, scope);

	Symbol* owner_function = analyzer_get_owner_function(scope);

	Symbol* symbol = analyzer_add_symbol_to_scope(module, node, scope, offset, 0);

	if (!node->declare.is_static && owner_function != NULL)
	{
		printf("[Analyzer] [Debug] Local variable \"%s\", adding offset...\n", node->declare.identifier);

		int size = analyzer_get_type_size(node->declare.var_type, scope);
		size = align_offset(size);

		symbol->symbol_variable->offset = *offset;

		*offset -= size;
	}
	
	if (!node->declare.is_static && scope->scope_kind == SYMBOL_CLASS)
	{
		int size = analyzer_get_type_size(node->declare.var_type, scope);
		
		if (size % 8 != 0)
		{
			size = ((size / 8) + 1) * 8;
		}
		
		Symbol* class_symbol = scope->owner_statement;
		class_symbol->symbol_class->total_offset += size;

		symbol->symbol_variable->offset = class_symbol->symbol_class->offset;
		symbol->symbol_variable->is_class_global = 1;
		class_symbol->symbol_class->offset += size;
	}
	
	ASTNode* expression = node->declare.default_value;

	if (expression != NULL)
	{
		analyzer_analyze_node(module, expression, scope, NULL);

		node->declare.default_value = analyzer_implictly_cast_all(module, node->declare.var_type, expression, scope);
	}
}

static void analyzer_handle_parameters(Module* module, ASTNode* head, SymbolTable* scope, int* offset)
{
	ASTNode* next = head;

	int count = 0;

	while (next != NULL)
	{
		int size = analyzer_get_type_size(next->param.argument_type, scope);
		
		if (analyzer_find_symbol_only_from_scope(next->param.identifier, scope, 0, 0, 0) != NULL)
		{
			printf("[Analyzer] [Debug] Parameter with name: \"%s\" already declared...\n", next->param.identifier);
			exit(1);
		}

		Symbol* symbol = analyzer_add_symbol_to_scope(module, next, scope, offset, 0);

		*offset += align_offset(size);

		next = next->next;
		count++;
	}
}

int analyzer_check_parameters(ASTNode* params_head, ASTNode* _params_head)
{
	int function_size = (params_head == NULL) ? 0 : analyzer_get_list_size(params_head);
	int _function_size = (_params_head == NULL) ? 0 : analyzer_get_list_size(_params_head);
	
	if (function_size != _function_size)
	{
		return 0;
	}

	int i = 0;

	ASTNode* parameter = params_head;
	ASTNode* _parameter = _params_head;
	
	while (i < function_size)
	{
		if (parameter == NULL || _parameter == NULL)
		{
			return 0;
		}

		if (strcmp(parameter->param.identifier, _parameter->param.identifier) != 0)
		{
			return 0;
		}

		if (!analyzer_is_the_same(parameter->param.argument_type, _parameter->param.argument_type))
		{
			return 0;
		}

		parameter = parameter->next;
		_parameter = _parameter->next;

		i++;
	}

	return 1;
}

static void analyzer_check_function(Symbol* symbol, Symbol* super_class)
{
	if (super_class == NULL)
	{
		if (symbol->symbol_function->is_override)
		{
			printf("[Analyzer] [Debug] Overriding a non declared function...\n");
			exit(1);
		}

		return;
	}

	ASTNode* member = analyzer_get_function_from_class(super_class, (char*) symbol->symbol_function->identifier);

	if (member == NULL)
	{
		if (symbol->symbol_function->is_override)
		{
			printf("[Analyzer] [Debug] Overriding a non declared function...\n");
			exit(1);
		}

		return;
	}

	int flag = analyzer_is_the_same(symbol->symbol_function->return_type, member->function.return_type);
	int _flag = analyzer_check_parameters(symbol->symbol_function->params_head, (member->function.params == NULL) ? NULL : member->function.params->head);
	
	if (!flag || !_flag)
	{
		printf("[Analyzer] [Debug] Types / Parameters aren't the same...\n");
		exit(1);
	}
	
	if (member->function.is_virtual)
	{
		if (!symbol->symbol_function->is_override)
		{
			printf("[Analyzer] [Debug] Function needs the 'override' modifier...\n");
			exit(1);
		}

		return;
	}
	
	printf("[Analyzer] [Debug] Can't apply shadowing on functions...\n");
	exit(1);
}

static void analyzer_add_method_to_table(MethodEntry* entry, ClassVTable* table)
{
	if (table->entries_capacity <= table->entries_count)
	{
		table->entries_capacity *= 2;
		table->entries = realloc(table->entries, sizeof(MethodEntry*) * table->entries_capacity);
	}

	table->entries[table->entries_count] = entry;
	table->entries_count++;
}

static void analyzer_copy_method_to_table(MethodEntry* entry, ClassVTable* table)
{
	if (table->entries_capacity <= table->entries_count)
	{
		table->entries_capacity *= 2;
		table->entries = realloc(table->entries, sizeof(MethodEntry*) * table->entries_capacity);
	}

	MethodEntry* temp = malloc(sizeof(MethodEntry));
	*temp = *entry;

	table->entries[table->entries_count] = temp;
	table->entries_count++;
}

static void analyzer_find_and_replace_method_to_table(MethodEntry* entry, ClassVTable* table)
{
	for (int i = 0; i < table->entries_count; i++)
	{
		MethodEntry* curr = table->entries[i];

		if (strcmp(entry->method_name, curr->method_name) == 0)
		{
			entry->method_index = curr->method_index;
			table->entries[i] = entry;
		}
	}
}

static MethodEntry* analyzer_create_method_entry(ASTNode* method, Symbol* class, int index)
{
	MethodEntry* entry = malloc(sizeof(MethodEntry));

	entry->method_name = strdup(method->function.identifier);
	entry->class_name = strdup(class->symbol_class->identifier);

	entry->method_index = index;

	return entry;
}

static void analyzer_setup_class_vtable(Symbol* class)
{
	ClassVTable* class_v_table = malloc(sizeof(ClassVTable));
	
	class_v_table->entries = malloc(sizeof(MethodEntry*) * 4);
	class_v_table->entries_capacity = 4;
	class_v_table->entries_count = 0;

	class->symbol_class->class_v_table = class_v_table;
}

static void analyzer_copy_class_vtable(Symbol* class, ClassVTable* super_v_table)
{
	ClassVTable* class_v_table = malloc(sizeof(ClassVTable));
	class->symbol_class->class_v_table = class_v_table;

	class_v_table->entries = malloc(sizeof(MethodEntry*) * super_v_table->entries_capacity);
	class_v_table->entries_capacity = super_v_table->entries_capacity;
	class_v_table->entries_count = 0;

	for (int i = 0; i < super_v_table->entries_count; i++)
	{
		MethodEntry* entry = super_v_table->entries[i];
		analyzer_copy_method_to_table(entry, class_v_table);
	}
}

static void analyzer_handle_class_v_table(Symbol* class)
{
	analyzer_setup_class_vtable(class);
}

static void analyzer_handle_class_methods(Symbol* class)
{
	for (int i = 0; i < class->symbol_class->func_count; i++)
	{
		ASTNode* curr = class->symbol_class->functions[i];

		if (curr->function.is_virtual)
		{
			int index = class->symbol_class->class_v_table->entries_count;

			MethodEntry* entry = analyzer_create_method_entry(curr, class, index);
			analyzer_add_method_to_table(entry, class->symbol_class->class_v_table);
		}
		else if (curr->function.is_override)
		{
			MethodEntry* entry = analyzer_create_method_entry(curr, class, -1);
			analyzer_find_and_replace_method_to_table(entry, class->symbol_class->class_v_table);
		}
	}
}

static void analyzer_handle_function_declaration(Module* module, ASTNode* ASTNode, SymbolTable* scope)
{
	const ASTNodeFunc* function_node = &ASTNode->function;
	
	if (analyzer_find_symbol_from_scope(function_node->identifier, scope, 0, 1, 0, 0) != NULL)
	{
		printf("[Analyzer] [Debug] Function with name: \"%s\" already declared...\n", function_node->identifier);
		exit(1);
	}

	if (check_already_has_symbol(module, function_node->identifier, SYMBOL_FUNCTION))
	{
		printf("[Analyzer] [Debug] Function with name: %s, already declared in imported modules...\n", function_node->identifier);
		exit(1);
	}

	if (function_node->is_constructor)
	{
		if (strcmp(scope->owner_statement->symbol_class->identifier, function_node->identifier) != 0)
		{
			printf("[Analyzer] [Debug] Invalid constructor name, use the same name from class...\n");
			exit(1);
		}
	}

	if (function_node->is_static && function_node->is_virtual)
	{
		printf("[Analyzer] [Debug] Static functions can't be virtual...\n");
		exit(1);
	}

	int local_offset = -8;

	int param_offset = 8;

	analyzer_analyze_type(ASTNode->function.return_type, scope);

	Symbol* func_symbol = analyzer_add_symbol_to_scope(module, ASTNode, scope, NULL, 0);

	func_symbol->is_prototype = 0;
	
	SymbolTable* block_scope = analyzer_create_scope(SYMBOL_FUNCTION, scope, func_symbol);

	func_symbol->symbol_function->scope = block_scope;

	if (scope->scope_kind == SYMBOL_CLASS)
	{
		Symbol* super = scope->owner_statement->symbol_class->super;

		analyzer_check_function(func_symbol, super);
	}

	if (function_node->params != NULL)
	{
		analyzer_handle_parameters(module, function_node->params->head, block_scope, &param_offset);
	}

	analyzer_analyze_node(module, function_node->block, block_scope, &local_offset);

	func_symbol->symbol_function->total_offset = local_offset;
	func_symbol->symbol_function->total_param_offset = param_offset - 8;
}

static void analyzer_create_cast(ASTNode** node, Type* preferred)
{
	printf("[Analyzer] [Debug] Creating a cast ASTNode, casting type: %d\n", preferred->type);
	
	ASTNode* cast = malloc(sizeof(ASTNode));

	if (cast == NULL)
	{
		printf("[Analyzer] [Debug] Failed to alllocate memory for cast ASTNode...\n");
		exit(1);
	}

	cast->type = NODE_CAST;
	cast->cast_node.cast_type = preferred;
	cast->cast_node.expr = *node;

	*node = cast;
}

static Type* cpy_type(Type* type)
{
	Type* ret = NULL;
	
	Type* t = type;
	Type** curr = &ret;

	while (t != NULL)
	{
		*curr = malloc(sizeof(Type));
		
		**curr = *t;
		t = t->base;

		curr = &ret->base;
	}

	return ret;
}

static ASTNode* _analyzer_create_cast(ASTNode** node, Type* preferred)
{
	printf("[Analyzer] [Debug] Creating a cast ASTNode, casting type: %d\n", preferred->type);
	
	ASTNode* cast = malloc(sizeof(ASTNode));

	if (cast == NULL)
	{
		printf("[Analyzer] [Debug] Failed to alllocate memory for cast ASTNode...\n");
		exit(1);
	}

	cast->type = NODE_CAST;
	cast->cast_node.cast_type = cpy_type(preferred);
	cast->cast_node.expr = *node;

	*node = cast;

	return cast;
}

static void analyzer_implictly_cast_operation(Module* module, ASTNode* node, SymbolTable* scope)
{
	ASTNodeOperation* operation = &node->operation;

	Type* left = analyzer_return_type_of_expression(module, operation->left, scope, NULL, 0, NULL);
	Type* right = analyzer_return_type_of_expression(module, operation->right, scope, NULL, 0, NULL);

	Type* higher = analyzer_get_higher(left, right);

	if (!analyzer_is_the_same(left, higher))
	{
		analyzer_create_cast(&operation->left, higher);
	}

	if (!analyzer_is_the_same(right, higher))
	{
		analyzer_create_cast(&operation->right, higher);
	}
}

static void analyzer_handle_operation(Module* module, ASTNode* node, SymbolTable* scope)
{
	ASTNodeOperation* operation = &node->operation;
	
	analyzer_analyze_node(module, operation->left, scope, NULL);
	analyzer_analyze_node(module, operation->right, scope, NULL);
	
	if (!analyzer_is_operation_compatible(module, node, scope))
	{
		printf("[Analyzer] [Debug] Incompatible operation...\n");
		exit(1);
	}

	analyzer_implictly_cast_operation(module, node, scope);
}

static void analyzer_check_global_scope(ASTNode* ASTNode, SymbolTable* scope)
{
	const ASTNodeType type = ASTNode->type;
	const SymbolType kind = scope->scope_kind;
	
	if (type == NODE_FUNC && kind != GLOBAL_SCOPE && kind != SYMBOL_CLASS)
	{
		printf("[Analyzer] [Debug] Function declaration statement isn't on global scope...\n");
		exit(1);
	}

	if (type == NODE_IMPORT && kind != GLOBAL_SCOPE)
	{
		printf("[Analyzer] [Debug] Function declaration statement isn't on global scope...\n");
		exit(1);
	}

	if (type != NODE_FUNC && type != NODE_LITERAL && type != NODE_CLASS && type != NODE_FIELD && type != NODE_IMPORT && kind == GLOBAL_SCOPE)
	{
		printf("[Analyzer] [Debug] Statement can't be in a global scope: %d...\n", type);
		exit(1);
	}
}

static void analyzer_handle_block(Module* module, ASTNode* node, SymbolTable* scope, int* local_offset)
{
	ASTNode* next = node->block.statements->head;
	
	while (next != NULL)
	{
		if (next->type == NODE_FIELD || next->type == NODE_IF || next->type == NODE_WHILE_LOOP || next->type == NODE_FOR_LOOP || next->type == NODE_SWITCH)
		{
			analyzer_analyze_node(module, next, scope, local_offset);
		}
		else
		{
			analyzer_analyze_node(module, next, scope, NULL);
		}
		
		next = next->next;
	}
}

static Type* analyzer_get_function_return_type(SymbolTable* scope)
{
	SymbolTable* current = scope;

	while (current != NULL)
	{
		if (current->scope_kind == SYMBOL_FUNCTION && current->owner_statement != NULL)
		{
			return current->owner_statement->symbol_function->return_type;
		}

		current = current->parent;
	}

	return NULL;
}

static void analyzer_handle_return(Module* module, ASTNode* node, SymbolTable* scope)
{
	ASTNode* return_value = node->return_statement.return_value;
	
	if (return_value == NULL)
	{
		printf("[Analyzer] [Debug] Returning void...\n");
	}

	if (return_value != NULL)
	{
		analyzer_analyze_node(module, return_value, scope, NULL);
	}

	Type* return_type = analyzer_get_owner_function(scope)->symbol_function->return_type;

	if (return_type == NULL)
	{
		printf("[Analyzer] [Debug] Function return type is null...\n");
		exit(1);
	}

	if ((return_type->type != TYPE_VOID && return_value == NULL) || (return_type->type == TYPE_VOID && return_value != NULL))
	{
		printf("[Analyzer] [Debug] Invalid return type, return type expected: %d...\n", return_type->type);
		exit(1);
	}

	if (return_value != NULL)
	{
		analyzer_implictly_cast_all(module, return_type, return_value, scope);
	}
}

int analyzer_get_list_size(ASTNode* list_head)
{
	ASTNode* next = list_head;

	int count = 0;

	while (next !=  NULL)
	{
		next = next->next;
		count++;
	}

	return count;
}

static void analyzer_check_arguments(Module* module, ASTNode* params_head, ASTNode* args_head, SymbolTable* scope)
{
	int arguments_size = (args_head == NULL) ? 0 : analyzer_get_list_size(args_head);
	int params_size = (params_head == NULL) ? 0 : analyzer_get_list_size(params_head);
	
	if (arguments_size != params_size)
	{
		printf("[Analyzer] [Debug] Expected argument size is: %d...\n", params_size);
		printf("[Analyzer] [Debug] Params size: %d, Args size: %d...\n", params_size, arguments_size);
		exit(1);
	}

	ASTNode* current = args_head;
	ASTNode* current_ = params_head;

	while (current != NULL)
	{
		Type* type = analyzer_return_type_of_expression(module, current->argument.value, scope, NULL, 0, NULL);
		Type* type_ = current_->param.argument_type;

		if (!analyzer_is_type_identic(type, type_, scope))
		{
			analyzer_implictly_cast_all(module, type_, current, scope);
		}
		
		current = current->next;
		current_ = current_->next;
	}
}

static char* get_function_call_name(ASTNode* callee)
{
	if (callee->type == NODE_MEMBER_ACCESS)
	{
		return callee->member_access.member_name;
	}

	if (callee->type == NODE_IDENT)
	{
		return callee->variable.identifier;
	}

	return "";
}

static void analyzer_handle_function_call(Module* module, ASTNode* node, SymbolTable* scope)
{
	printf("[Analyzer] [Debug] Trying to call a function...\n");
	
	Type* type = analyzer_return_type_of_expression(module, node, scope, NULL, 0, NULL);

	if (type == NULL)
	{
		printf("[Analyzer] [Debug] Function not found: \"%s\"...\n", get_function_call_name(node->function_call.callee));
		exit(1);
	}
}

static void analyzer_handle_if(Module* module, ASTNode* node, SymbolTable* scope, int* offset)
{
	analyzer_analyze_node(module, node->if_statement.condition_top, scope, NULL);

	if (analyzer_return_type_of_expression(module, node->if_statement.condition_top, scope, NULL, 0, NULL)->type != TYPE_BOOL)
	{
		printf("[Analyzer] [Debug] Invalid if condition, expression return type needs to be 'boolean'...\n");
		exit(1);
	}

	SymbolTable* then_scope = analyzer_create_scope(SYMBOL_IF, scope, NULL);
	node->if_statement.then_scope = then_scope; // pro codegen
	
	analyzer_analyze_node(module, node->if_statement.then_branch, then_scope, offset);

	if (node->if_statement.else_branch != NULL)
	{
		SymbolTable* else_scope = analyzer_create_scope(SYMBOL_ELSE, scope, NULL);
		analyzer_analyze_node(module, node->if_statement.else_branch, else_scope, offset);

		node->if_statement.else_scope = else_scope; // pro codegen
	}
}

static void analyzer_handle_adress_of(ASTNode* ASTNode, SymbolTable* scope)
{
	const ASTNodeReference* adress_of = &ASTNode->adress_of;
	
	if (adress_of->expr->type != NODE_IDENT)
	{
		printf("[Analyzer] [Debug] Adressing requires a pointer variable...\n");
		exit(1);
	}
	
	Symbol* var = analyzer_find_symbol_from_scope(adress_of->expr->variable.identifier, scope, 1, 0, 0, 0);
	
	if (var == NULL)
	{
		printf("[Analyzer] [Debug] Referencing a not declared variable...\n");
		exit(1);
	}
}

static void analyzer_handle_dereference(ASTNode* node, SymbolTable* scope)
{
	ASTNode* ref = node;

	while (ref->type == NODE_DEREFERENCE)
	{
		ref = ref->dereference.expr;
	}

	if (ref->type != NODE_IDENT)
	{
		printf("[Analyzer] [Debug] Dereferencing requires a pointer variable...\n");
		exit(1);
	}
	
	Symbol* field = analyzer_find_symbol_from_scope(ref->variable.identifier, scope, 1, 0, 0, 0);
	
	if (field == NULL)
	{
		printf("[Analyzer] [Debug] Dereferencing a not declared variable...\n");
		exit(1);
	}
}

static void analyzer_handle_while_loop(Module* module, ASTNode* node, SymbolTable* scope, int* offset)
{
	analyzer_analyze_node(module, node->while_loop.condition, scope, NULL);

	if (analyzer_return_type_of_expression(module, node->while_loop.condition, scope, NULL, 0, NULL)->type != TYPE_BOOL)
	{
		printf("[Analyzer] [Debug] Invalid while condition, expression return type needs to be 'boolean'...\n");
		exit(1);
	}

	SymbolTable* block_scope = analyzer_create_scope(SYMBOL_WHILE, scope, NULL);
	node->while_loop.then_scope = block_scope;
	block_scope->owner_statement = malloc(sizeof(Symbol));
	block_scope->owner_statement->type = SYMBOL_WHILE;

	analyzer_analyze_node(module, node->while_loop.then_block, block_scope, offset);
}

static void analyzer_handle_for_loop(Module* module, ASTNode* node, SymbolTable* scope, int* offset)
{
	SymbolTable* block_scope = analyzer_create_scope(SYMBOL_FOR, scope, NULL);
	node->for_loop.then_scope = block_scope;
	
	analyzer_analyze_node(module, node->for_loop.init, scope, offset);

	analyzer_analyze_node(module, node->for_loop.condition, scope, offset);
	analyzer_analyze_node(module, node->for_loop.then_statement, scope, offset);

	if (analyzer_return_type_of_expression(module, node->for_loop.condition, scope, NULL, 0, NULL)->type != TYPE_BOOL)
	{
		printf("[Analyzer] [Debug] Invalid for condition, expression return type needs to be 'boolean'...\n");
		exit(1);
	}

	analyzer_analyze_node(module, node->for_loop.then_block, block_scope, offset);
}

static int analyzer_is_inside(SymbolTable* scope, SymbolType type)
{
	if (scope == NULL)
	{
		return 0;
	}

	if (scope->scope_kind == type)
	{
		return 1;
	}

	return analyzer_is_inside(scope->parent, type);
}

static void analyzer_handle_break(ASTNode* ASTNode, SymbolTable* scope)
{
	if (!analyzer_is_inside(scope, SYMBOL_FOR) && !analyzer_is_inside(scope, SYMBOL_WHILE) && !analyzer_is_inside(scope, SYMBOL_SWITCH))
	{
		printf("[Analyzer] [Debug] Break statement outside a loop or switch statement...\n");
		exit(1);
	}
}

static void analyzer_handle_var_assign(Module* module, ASTNode* node, SymbolTable* scope)
{
	ASTNode* expression = node->variable_assign.expr;
	ASTNode* left = node->variable_assign.left;
	
	Type* type = analyzer_return_type_of_expression(module, left, scope, NULL, 0, NULL);

	if (type == NULL)
	{
		printf("[Analyzer] [Debug] Member not found...\n");
		exit(1);
	}

	analyzer_analyze_node(module, expression, scope, NULL);
	
	if (left->type == NODE_CAST)
	{
		printf("[Analyzer] [Debug] Can't assign value to a cast...\n");
		exit(1);
	}

	analyzer_implictly_cast_all(module, type, expression, scope);
}

static ASTNode* analyzer_get_function_from_class(Symbol* class, char* func_name)
{
	if (class == NULL)
	{
		return NULL;
	}
	
	for (int i = 0; i < class->symbol_class->func_count; i++)
	{
		ASTNode* member = class->symbol_class->functions[i];
	
		if (strcmp(member->function.identifier, func_name) == 0)
		{
			return member;
		}
	}

	return analyzer_get_function_from_class(class->symbol_class->super, func_name);
}

static int analyzer_class_extends(Symbol* class, char* super_name)
{
	if (class->symbol_class->super == NULL)
	{
		return 0;
	}

	if (strcmp(class->symbol_class->super->symbol_class->identifier, super_name) == 0)
	{
		return 1;
	}

	return analyzer_class_extends(class->symbol_class->super, super_name);
}

static int analyzer_is_class_assignable(Symbol* from, Symbol* to)
{
	if (strcmp(from->symbol_class->identifier, to->symbol_class->identifier) == 0)
	{
		return 1;
	}
	
	return analyzer_class_extends(from, (char*) to->symbol_class->identifier);
}

static ASTNode* analyzer_get_member_from_class(Symbol* class, char* member_name)
{
	if (class == NULL)
	{
		return NULL;
	}
	
	for (int i = 0; i < class->symbol_class->field_count; i++)
	{
		ASTNode* member = class->symbol_class->fields[i];

		if (strcmp(member->declare.identifier, member_name) == 0)
		{
			return member;
		}
	}

	return analyzer_get_member_from_class(class->symbol_class->super, member_name);
}

static void analyzer_handle_class_funcs(Module* module, ASTNode* ASTNode, SymbolTable* scope)
{
	const ASTNodeClass* class_node = &ASTNode->class_node;
	
	for (int i = 0; i < class_node->funcs_count; i++)
	{
		analyzer_analyze_node(module, class_node->funcs[i], scope, NULL);
	}
}

static void analyzer_handle_class_vars(Module* module, ASTNode* ASTNode, SymbolTable* scope)
{
	const ASTNodeClass* class_node = &ASTNode->class_node;
	
	for (int i = 0; i < class_node->fields_count; i++)
	{
		analyzer_analyze_node(module, class_node->fields[i], scope, NULL);
	}
}

static int analyzer_check_if_call_super_constructor(Symbol* class, int add_call_implictly)
{
	ASTNode* constructor = class->symbol_class->constructor_node;

	if (constructor == NULL)
	{
		return 0;
	}

	int super_constructor_call = 0;
	ASTNode** head = &(constructor->function.block->block.statements->head);
	ASTNode* next = *head;

	while (next != NULL)
	{
		if (next->type != NODE_CALL)
		{
			next = next->next;

			continue;
		}

		if (next->function_call.callee->type != NODE_SUPER)
		{
			next = next->next;

			continue;
		}

		super_constructor_call = 1;

		break;
	}

	if (!super_constructor_call && add_call_implictly) // só usar caso o constructor da super não tenha parametros
	{
		ASTNode* node = malloc(sizeof(ASTNode));
		node->type = NODE_CALL;

		if (node == NULL)
		{
			exit(1);
		}
		
		ASTNode* super_node = malloc(sizeof(ASTNode));

		if (super_node == NULL)
		{
			exit(1);
		}

		super_node->type = NODE_SUPER;

		node->function_call.callee = super_node;
		node->function_call.arguments = NULL;

		node->next = *head;

		*head = node;

		printf("Added 'super' call implictly...\n");
	}

	return super_constructor_call;
}

static void analyzer_check_super_constructor(Module* module, Symbol* symbol, SymbolTable* scope)
{
	Symbol* super = symbol->symbol_class->super;
	
	if (super == NULL)
	{
		return;
	}
	
	if (super->symbol_class->constructor != NULL && super->symbol_class->constructor->symbol_function->params_head != NULL)
	{
		if (symbol->symbol_class->constructor == NULL || !analyzer_check_if_call_super_constructor(symbol, 0))
		{
			exit(1);
		}
	}
	else if (super->symbol_class->constructor != NULL)
	{
		analyzer_check_if_call_super_constructor(symbol, 1);
	}
}

static void analyzer_handle_class_declaration(Module* module, ASTNode* node, SymbolTable* scope)
{
	const ASTNodeClass* class_node = &node->class_node;
	
	if (analyzer_find_symbol_from_scope(class_node->identifer, scope, 0, 0, 1, 0) != NULL)
	{
		printf("[Analyzer] [Debug] Class already declared: %s...", class_node->identifer);
		exit(1);
	}
	
	Symbol* class_symbol = analyzer_create_symbol(module, SYMBOL_CLASS, node, scope, 0, NULL, 0);

	SymbolTable* class_scope = analyzer_create_scope(SYMBOL_CLASS, scope, class_symbol);
	
	if (class_node->super_identifer != NULL)
	{
		Symbol* super_symbol = analyzer_find_symbol_from_scope(class_node->super_identifer, scope, 0, 0, 1, 0);
		
		if (super_symbol == NULL)
		{
			printf("[Analyzer] [Debug] Super class not found...\n");
			exit(1);
		}
		
		class_symbol->symbol_class->super = super_symbol;

		analyzer_copy_class_vtable(class_symbol, super_symbol->symbol_class->class_v_table);
	}
	else 
	{
		class_symbol->symbol_class->super = NULL;
		analyzer_handle_class_v_table(class_symbol);
		
		printf("[Analyzer] [Debug] Class dont have a super class...\n");
	}

	class_symbol->symbol_class->class_scope = class_scope;
	class_symbol->symbol_class->constructor = NULL;
	
	analyzer_handle_class_vars(module, node, class_scope);
	
	analyzer_handle_class_funcs(module, node, class_scope);
	
	analyzer_handle_class_methods(class_symbol);

	if (class_node->constructor != NULL)
	{
		analyzer_analyze_node(module, class_node->constructor, class_scope, NULL);
		class_symbol->symbol_class->constructor = analyzer_find_symbol_from_scope(class_node->constructor->function.identifier, class_scope, 0, 1, 0, 0);
	}

	analyzer_check_super_constructor(module, class_symbol, class_scope);
}

static void analyzer_handle_create_instance(Module* module, ASTNode* node, SymbolTable* scope)
{
	const ASTNodeCreateInst* instance_node = &node->create_instance;
	
	Symbol* class = analyzer_find_symbol_from_scope(instance_node->class_name, scope, 0, 0, 1, 0);
	
	if (class == NULL)
	{
		printf("[Analyzer] [Debug] Failed to create a instance, class not found: %s...", instance_node->class_name);
		exit(1);
	}
	
	if (class->symbol_class->constructor != NULL)
	{
		ASTNode* params_head = class->symbol_class->constructor->symbol_function->params_head;
		ASTNode* args_head = (instance_node->constructor_args == NULL) ? NULL : instance_node->constructor_args->head;

		analyzer_check_arguments(module, params_head, args_head, scope);
	}
}

static int analyzer_is_able_to_this(SymbolTable* scope)
{
	if (scope == NULL)
	{
		return 0;
	}

	if (scope->scope_kind == SYMBOL_CLASS)
	{
		return 1;
	}

	return analyzer_is_able_to_this(scope->parent);
}

static int analyzer_is_able_to_super(SymbolTable* scope)
{
	if (scope == NULL)
	{
		return 0;
	}

	if (scope->scope_kind == SYMBOL_CLASS)
	{
		return scope->owner_statement->symbol_class->super != NULL;
	}

	return analyzer_is_able_to_this(scope->parent);
}

static void analyzer_handle_this(ASTNode* ASTNode, SymbolTable* scope)
{
	if (!analyzer_is_able_to_this(scope))
	{
		exit(1);
	}
}

static void analyzer_handle_super(ASTNode* ASTNode, SymbolTable* scope)
{
	if (!analyzer_is_able_to_super(scope))
	{
		exit(1);
	}
}

static void analyzer_handle_member_access(Module* module, ASTNode* ASTNode, SymbolTable* scope)
{
	printf("[Analyzer] [Debug] Trying to acess a member from class object...\n");
	
	Type* type = analyzer_return_type_of_expression(module, ASTNode, scope, NULL, 0, NULL);

	if (type == NULL)
	{
		printf("[Analyzer] [Debug] Member not found...\n");
		exit(1);
	}
}

static void analyzer_handle_array_access(Module* module, ASTNode* ASTNode, SymbolTable* scope)
{
	Type* type = analyzer_return_type_of_expression(module, ASTNode, scope, NULL, 0, NULL);

	if (type == NULL)
	{
		printf("[Analyzer] [Debug] Failed to access array...\n");

		exit(1);
	}
}

static void analyzer_handle_cast(Module* module, ASTNode* node, SymbolTable* scope)
{
	printf("[Analyzer] [Debug] Found a cast ASTNode...\n");
	
	Type* expression_type = analyzer_return_type_of_expression(module, node->cast_node.expr, scope, NULL, 0, NULL);

	if (expression_type == NULL)
	{
		printf("[Analyzer] [Debug] Invalid expression in cast ASTNode...\n");
		exit(1);
	}

	if (!analyzer_is_compatible(node->cast_node.cast_type, expression_type, scope))
	{
		printf("[Analyzer] [Debug] Incompatible cast types...\n");
		exit(1);
	}
}

static void analyzer_handle_identifier(ASTNode* node, SymbolTable* scope)
{
	Symbol* symbol = analyzer_find_symbol_from_scope(node->variable.identifier, scope, 1, 0, 0, 0);

	if (symbol == NULL)
	{
		printf("[Analyzer] [Debug] Field with name: \"%s\" not found...\n", node->variable.identifier);
		exit(1);
	}
}

static void analyzer_check_array_literal_values(Module* module, ASTNode* head, Type* type, SymbolTable* scope)
{
	ASTNode* current = head;
	
	// pula o primeiro tipo (array), ja que a array precisa de elementos daquele tipo.
	Type* required = type->base;

	int index = 0;

	while (current != NULL)
	{
		analyzer_analyze_node(module, current, scope, NULL);

		Type* expr_type = analyzer_return_type_of_expression(module, current, scope, NULL, 0, NULL);
		
		if (!analyzer_is_the_same(expr_type, required))
		{
			printf("[Analyzer] [Debug] Array element #%d expected type '%d' but got '%d'...\n", index, required->type, expr_type->type);
			exit(1);
		}

		current = current->next;

		index++;
	}
}

static void analyzer_handle_array_literal(Module* module, ASTNode* node, SymbolTable* scope)
{
	ASTNodeList* values = node->array_literal.values;
	Type* type = node->array_literal.array_type;
	
	analyzer_analyze_type(type, scope);

	if (values != NULL)
	{
		analyzer_check_array_literal_values(module, values->head, type, scope);
	}
}

static void analyzer_handle_switch_case(Module* module, ASTNode* node, SymbolTable* scope, Type* required_type, int* offset)
{
	ASTNode* condition = node->switch_case_block.condition;
	ASTNode* block = node->switch_case_block.block;

	Type* expr_type = analyzer_return_type_of_expression(module, condition, scope, NULL, 0, NULL);

	if (!analyzer_is_type_identic(expr_type, required_type, scope))
	{
		printf("[Analyzer] [Debug] Invalid expression return type...\n");
		exit(1);
	} 

	analyzer_analyze_node(module, block, scope, offset);
}

static void analyzer_handle_switch_statement(Module* module, ASTNode* node, SymbolTable* scope, int* offset)
{
	ASTNodeList* case_list = node->switch_statement.case_list;
	
	analyzer_analyze_node(module, node->switch_statement.value, scope, offset);

	ASTNode* next = case_list->head;

	Type* required_type = analyzer_return_type_of_expression(module, node->switch_statement.value, scope, NULL, 0, NULL);

	SymbolTable* switch_scope = analyzer_create_scope(SYMBOL_SWITCH, scope, NULL);

	while (next != NULL)
	{
		SymbolTable* case_scope = NULL;

		if (next->switch_case_block.new_scope)
		{
			case_scope = analyzer_create_scope(SYMBOL_SWITCH_CASE, switch_scope, NULL);
			next->switch_case_block.scope = case_scope;
		}
		else
		{
			next->switch_case_block.scope = NULL;
		}

		analyzer_handle_switch_case(module, next, case_scope != NULL ? case_scope : switch_scope, required_type, offset);

		next = next->next;
	}
}

static char* handle_relative_path(char* abs_path, char* relative)
{
	return resolve_path(abs_path, relative);
}

static void analyzer_handle_local_import(Module* module, ASTNode* node, SymbolTable* scope)
{
	ModuleStack* stack = module->stack;

	if (stack == NULL)
	{
		stack = setup_module_stack();
	}
	
	char* path = handle_relative_path(module->handler->root_path, node->import_node.import_path);

	printf("\n%s: ---------------------------------------------------------------+\n\n", node->import_node.import_path);

	Module* import_module = compile_module(module->handler, stack, path);

	printf("\n+---------------------------------------------------------------------------------+\n\n");

	Symbol* symbol = analyzer_add_symbol_to_scope(module, node, scope, NULL, 0);
	symbol->symbol_module->module = import_module;

	add_module_to_list(module, import_module);

	// a variavel path é usada só agora, depois é copiada.
	free(path);
}

static void analyzer_handle_import(Module* module, ASTNode* node, SymbolTable* scope)
{
	if (analyzer_find_symbol_from_scope(node->import_node.identifier, scope, 1, 1, 1, 1) != NULL)
	{
		printf("[Analyzer] [Debug] Statement with identifier: \"%s\" already declared...\n", node->import_node.identifier);
		exit(1);
	}
	
	if (node->import_node.is_local)
	{
		analyzer_handle_local_import(module, node, scope);
	}
	else
	{
		/**
		 * TODO: dar handle em imports de libs imbutidas.
		 */
	}
}

static void analyzer_analyze_node(Module* module, ASTNode* node, SymbolTable* scope, int* offset)
{
	analyzer_check_global_scope(node, scope);
	
	switch (node->type) 
	{
		case NODE_FIELD:
		{
			analyzer_handle_variable_declaration(module, node, scope, offset);
			
			return;
		}
		
		case NODE_FUNC:
		{
			analyzer_handle_function_declaration(module, node, scope);
			
			return;
		}

		case NODE_IMPORT:
		{
			analyzer_handle_import(module, node, scope);
			
			return;
		}
		
		case NODE_LITERAL:
		{
			return;
		}
		
		case NODE_IDENT:
		{
			analyzer_handle_identifier(node, scope);

			return;
		}

		case NODE_IF:
		{
			analyzer_handle_if(module, node, scope, offset);

			return;
		}

		case NODE_WHILE_LOOP:
		{
			analyzer_handle_while_loop(module, node, scope, offset);
			
			return;
		}
		
		case NODE_FOR_LOOP:
		{
			analyzer_handle_for_loop(module, node, scope, offset);
			
			return;
		}

		case NODE_SWITCH:
		{
			analyzer_handle_switch_statement(module, node, scope, offset);

			return;
		}
		
		case NODE_OPERATION:
		{
			analyzer_handle_operation(module, node, scope);
			
			return;
		}
		
		case NODE_BLOCK:
		{
			analyzer_handle_block(module, node, scope, offset);
			
			return;
		}
		
		case NODE_RET:
		{
			analyzer_handle_return(module, node, scope);
			
			return;
		}
		
		case NODE_BREAK:
		{
			analyzer_handle_break(node, scope);
			
			return;
		}
		
		case NODE_REFERENCE:
		{
			analyzer_handle_adress_of(node, scope);
			
			return;
		}
		
		case NODE_DEREFERENCE:
		{
			analyzer_handle_dereference(node, scope);
			
			return;
		}
		
		case NODE_ASSIGN:
		{
			analyzer_handle_var_assign(module, node, scope);
			
			return;
		}
		
		case NODE_MEMBER_ACCESS:
		{
			analyzer_handle_member_access(module, node, scope);
			
			return;
		}

		case NODE_CALL:
		{
			analyzer_handle_function_call(module, node, scope);
			
			return;
		}
		
		case NODE_CLASS:
		{
			analyzer_handle_class_declaration(module, node, scope);

			return;
		}

		case NODE_CREATE_INST:
		{
			analyzer_handle_create_instance(module, node, scope);

			return;
		}

		case NODE_THIS:
		{
			analyzer_handle_this(node, scope);

			return;
		}

		case NODE_ARR_ACCESS:
		{
			analyzer_handle_array_access(module, node, scope);

			return;
		}

		case NODE_CAST:
		{
			analyzer_handle_cast(module, node, scope);

			return;
		}

		case NODE_ARR_LITERAL:
		{
			analyzer_handle_array_literal(module, node, scope);

			return;
		}

		case NODE_SUPER:
		{
			analyzer_handle_super(node, scope);

			return;
		}

		default:
		{
			printf("[Analyzer] [Debug] ASTNode type not implemented in analyzer: \"%d\"...", node->type);
			exit(1);
		}
	}
}
