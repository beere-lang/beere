/**
 * TODO:
 * - Implementar super, caso ainda não foi implementado.
 * 
 * NEXT:
 * - Implementar o optimizer.
 * - Implementar sistema de logs robusto e funcional.
 * - Implementar o codegen.
 * - Implementar o sistema de extern "C"
 */

/**
 * IMPORTANTE: Adicionar checks se algo (funçoes, variaveis globais, etc) dos modulos importados ja existem no modulo que importa.
 */

/**
 * TODO: Adicionar declaração de statements (funções, classes) antes de definir.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "analyzer.h"
#include "../modules/modules.h"
#include "../parser/parser.h"
#include "../modules/modules.h" 
#include "../symbols/symbols.h"
#include "../utils/utils.h"

static Symbol* analyzer_find_symbol_from_scope(const char* identifier, SymbolTable* scope, int is_variable, int is_function, int is_class, int is_module);
static Type* analyzer_return_type_of_expression(Module* module, Node* expression, SymbolTable* scope, NodeList* args, int member_access, int* direct);
static Symbol* analyzer_add_symbol_to_scope(Module* module, Node* node, SymbolTable* scope, int* offset, int prototype);
static void analyzer_implictly_cast_all(Module* module, Type* preffered, Node* expression, SymbolTable* scope);
static void analyzer_check_arguments(Module* module, Node* params_head, Node* args_head, SymbolTable* scope);
static void analyzer_analyze_node(Module* module, Node* node, SymbolTable* scope, int* offset);
static int check_module_has_symbol(Module* module, char* identifier, SymbolType type);
static int analyzer_is_type_identic(Type* first, Type* second, SymbolTable* scope);
static Node* analyzer_get_function_from_class(Symbol* class, char* func_name);
static Node* analyzer_get_member_from_class(Symbol* class, char* member_name);
static int analyzer_get_type_size(Type* type, SymbolTable* scope);
static int analyzer_is_class_assignable(Symbol* from, Symbol* to);
static void analyzer_create_cast(Node** node, Type* preferred);
static int analyzer_get_list_size(Node* list_head);

// +--------------------------------------------------------------------------------------------------------------+
// | Prototype methods são metodos que vem "imbutidos" na linguagem porém que você acessa por alguma variavel	  |
// | 												       							                              |
// | Exemplo:																		                              |
// |  let arr: int[] = new int[] { 10, 20, 30 }									  								  |
// |  let size: int = arr.size() // metodo prototype retorna 3 (quantidade de elementos que a array contém)       |
// |  arr.push(20) // metodo prototype adiciona um elemento a array (20)                                          |
// +--------------------------------------------------------------------------------------------------------------+

typedef struct
{
	const char* method_name;

	Type* access_type;
	Type* return_type;

	int type_acurracy;

	NodeList* params;
}
PrototypeMethod;

int prototype_method_size = 0;

PrototypeMethod* protype_methods[100]; // tamanho pra 100 metodos prototype, trocar caso necessario.

/**
 * @param nodes:
 * - Sempre terminar a lista em NULL, caso contrario vai dar malfuncionamento.
 */
static NodeList* chain_nodes_to_list(Node** nodes)
{
	NodeList* node_list = malloc(sizeof(NodeList));
	node_list->head = NULL;

	if (nodes == NULL) 
	{
		return node_list;
	}

	Node** current = &node_list->head;
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

static Node* analyzer_create_prototype_method_node(char* method_name, Node** params, Type* return_type)
{
	Node* node = malloc(sizeof(Node));

	if (node == NULL)
	{
		printf("[Analyzer] [Debug] Failed to alloc memory for prototype method node...\n");
		exit(1);
	}

	node->type = NODE_FUNCTION;

	node->function_node.function.params = params == NULL ? NULL : chain_nodes_to_list(params);

	node->function_node.function.identifier = method_name;
	node->function_node.function.return_type = return_type;
	node->function_node.function.is_prototype = 1;
	node->function_node.function.is_constructor = 0;
	node->function_node.function.only_declaration = 0;
	node->function_node.function.is_static = 0;

	node->function_node.function.visibility = VISIBILITY_PUBLIC;
	
	return node;
}

static PrototypeMethod* analyzer_create_prototype_method(const char* method_name, Type* access_type, int type_acurracy, Node* method_ref, Type* return_type, NodeList* params)
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

/**
 * Declaração de funções prototype.
 * - IMPORTANT: Não esquecer de criar a lista de parametros caso necessario.
 */
static void analyzer_setup_prototype_methods(Module* module)
{
	{
		Node* size_method = analyzer_create_prototype_method_node("size", NULL, create_type(TYPE_INT, NULL));
		
		Type* type = create_type(TYPE_ARRAY, NULL);
		Type* return_type = create_type(TYPE_INT, NULL);
		
		protype_methods[0] = analyzer_create_prototype_method("size", type, 1, size_method, return_type, NULL);
		prototype_method_size++;

		analyzer_add_symbol_to_scope(module, size_method, module->global_scope, NULL, 1);
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

// +--------------------------------------+
// | Parte principal do analyzer a seguir |
// +--------------------------------------+

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

void analyzer_init(Module* module, Node** node_list)
{
	SymbolTable* scope = analyzer_setup_scope(GLOBAL_SCOPE, NULL, NULL);

	module->global_scope = scope;
	module->nodes = node_list;
	
	analyzer_setup_prototype_methods(module);
}

void analyzer_global_analyze(Module* module, Node* node)
{
	analyzer_analyze_node(module, node, module->global_scope, NULL);
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

	for (int i = 0; i < class_symbol->symbol_class->field_count; i++)
	{
		total_size += analyzer_get_type_size(class_symbol->symbol_class->fields[i]->declare_node.declare.var_type, scope);
	}

	return total_size;
}

static int analyzer_get_type_size(Type* type, SymbolTable* scope)
{
	switch (type->type) 
	{
		case TYPE_BOOL:
		{
			return 4;
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

		case TYPE_NULL:
		{
			return 8;
		}

		case TYPE_ARRAY:
		{
			return 12; // 8 bytes pro ponteiro que vai apontar pra array na heap e 4 bytes pro length da array em run-time.
		}

		case TYPE_CLASS:
		{
			return analyzer_get_class_size(type, scope);
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

static Symbol* analyzer_create_variable_symbol(Module* module, Node* node, SymbolTable* scope, int* offset)
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

	symbol->symbol_variable->is_global = (scope->scope_kind == GLOBAL_SCOPE || scope->scope_kind == SYMBOL_CLASS);

	symbol->symbol_variable->type = node->declare_node.declare.var_type;
	symbol->symbol_variable->identifier = node->declare_node.declare.identifier;
	symbol->symbol_variable->is_const = node->declare_node.declare.is_const;
	symbol->symbol_variable->is_static = node->declare_node.declare.is_static;

	symbol->is_export = node->declare_node.declare.is_export;

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
	else if (offset == NULL && !symbol->symbol_variable->is_global)
	{
		printf("[Analyzer] [Debug] Found a local variable with a invalid offset...\n");
		exit(1);
	}

	_analyzer_add_symbol_to_scope(symbol, scope);

	return symbol;
}

static Symbol* analyzer_create_parameter_symbol(Node* node, SymbolTable* scope, int* offset)
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

	symbol->symbol_variable->type = node->param_node.param.argument_type;
	symbol->symbol_variable->identifier = node->param_node.param.identifier;

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
		
	_analyzer_add_symbol_to_scope(symbol, scope);

	return symbol;
}

static Symbol* analyzer_create_function_symbol(Module* module, Node* node, SymbolTable* scope, int is_prototype)
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

	symbol->symbol_function->return_type = node->function_node.function.return_type;
	symbol->symbol_function->identifier = node->function_node.function.identifier;
	symbol->symbol_function->is_virtual = node->function_node.function.is_virtual;
	symbol->symbol_function->is_override = node->function_node.function.is_override;
	symbol->symbol_function->is_static = node->function_node.function.is_static;

	symbol->is_export = node->function_node.function.is_export;

	if (symbol->is_export)
	{
		analyzer_add_export_symbol(module, symbol);
	}

	symbol->is_prototype = is_prototype;

	if (node->function_node.function.params != NULL)
	{
		symbol->symbol_function->params_head = node->function_node.function.params->head;
	}
	else
	{
		symbol->symbol_function->params_head = NULL;
	}
		
	_analyzer_add_symbol_to_scope(symbol, scope);

	return symbol;
}

static Symbol* analyzer_create_class_symbol(Module* module, Node* node, SymbolTable* scope)
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

	symbol->symbol_class->identifier = node->class_node.class_node.identifer;

	symbol->symbol_class->functions = node->class_node.class_node.func_declare_list;
	symbol->symbol_class->fields = node->class_node.class_node.var_declare_list;

	symbol->symbol_class->func_count = node->class_node.class_node.func_count;
	symbol->symbol_class->field_count = node->class_node.class_node.var_count;

	symbol->is_export = node->class_node.class_node.is_export;

	if (symbol->is_export)
	{
		analyzer_add_export_symbol(module, symbol);
	}

	symbol->symbol_class->super = NULL;
		
	_analyzer_add_symbol_to_scope(symbol, scope);

	return symbol;
}

static Symbol* analyzer_create_module_symbol(Module* module, Node* node, SymbolTable* scope)
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

	symbol->symbol_module->identifier = node->class_node.class_node.identifer;

	symbol->symbol_module->module = NULL;
		
	_analyzer_add_symbol_to_scope(symbol, scope);

	return symbol;
}

static Symbol* analyzer_create_symbol(Module* module, SymbolType type, Node* node, SymbolTable* scope, int is_param, int* offset, int is_prototype)
{
	if (type == SYMBOL_VARIABLE && !is_param)
	{
		return analyzer_create_variable_symbol(module, node, scope, offset);
	}

	if (type == SYMBOL_VARIABLE && is_param)
	{
		return analyzer_create_parameter_symbol(node, scope, offset);
	}

	if (type == SYMBOL_FUNCTION)
	{
		return analyzer_create_function_symbol(module, node, scope, is_prototype);
	}

	if (type == SYMBOL_CLASS)
	{
		return analyzer_create_class_symbol(module, node, scope);
	}

	if (type == SYMBOL_MODULE)
	{
		return analyzer_create_module_symbol(module, node, scope);
	}

	return NULL;
}

static Symbol* analyzer_add_symbol_to_scope(Module* module, Node* node, SymbolTable* scope, int* offset, int prototype)
{
	switch (node->type) 
	{
		case NODE_DECLARATION:
		{
			return analyzer_create_symbol(module, SYMBOL_VARIABLE, node, scope, 0, offset, prototype);

			break;
		}

		case NODE_FUNCTION:
		{
			return analyzer_create_symbol(module, SYMBOL_FUNCTION, node, scope, 0, offset, prototype);
			
			break;
		}

		case NODE_PARAMETER:
		{
			return analyzer_create_symbol(module, SYMBOL_VARIABLE, node, scope, 1, offset, prototype);
			
			break;
		}

		case NODE_IMPORT:
		{
			return analyzer_create_symbol(module, SYMBOL_MODULE, node, scope, 0, offset, prototype);
			
			break;
		}

		default:
		{
			printf("[Analyzer] [Debug] Node type not valid for a symbol...\n");
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
 * - Usado em casts implicitos.
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

	return scope->parent;
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

static void analyzer_check_directly(int direct, Node* member, NodeType type)
{
	if (type == NODE_DECLARATION)
	{
		if (direct && !member->declare_node.declare.is_static)
		{
			printf("[Analyzer] [Debug] Can't access a non static field directly...\n");
			exit(1);
		}

		if (!direct && member->declare_node.declare.is_static)
		{
			printf("[Analyzer] [Debug] Can't access a static field from a instance...\n");
			exit(1);
		}
	}
	else if (type == NODE_FUNCTION)
	{
		if (direct && !member->function_node.function.is_static)
		{
			printf("[Analyzer] [Debug] Can't access a non static function directly...\n");
			exit(1);
		}

		if (!direct && member->function_node.function.is_static)
		{
			printf("[Analyzer] [Debug] Can't access a static function from a instance...\n");
			exit(1);
		}
	}
}

static Type* handle_function(Symbol* class_symbol, char* field_name, Module* module, SymbolTable* scope, int directly, NodeList* args)
{
	Node* member = analyzer_get_function_from_class(class_symbol, field_name);
		
	if (member == NULL)
	{
		return NULL;
	}

	if (member->type == NODE_FUNCTION)
	{
		if (member->function_node.function.visibility != VISIBILITY_PUBLIC)
		{
			printf("[Analyzer] [Debug] Trying to access a private function: \"%s\"...\n", field_name);
			return NULL;
		}

		analyzer_check_directly(directly, member, NODE_FUNCTION);
	
		Type* field_type = analyzer_return_type_of_expression(module, member, class_symbol->symbol_class->class_scope, args, 1, NULL);

		Node* params_head = member->function_node.function.params == NULL ? NULL : member->function_node.function.params->head;
		Node* args_head = (args == NULL) ? NULL : args->head;
			
		analyzer_check_arguments(module, params_head, args_head, scope);

		if (field_type == NULL)
		{
			return NULL;
		}
		
		return field_type;
	}
}

static Type* handle_prototype_method(Type* type, char* field_name, Module* module, SymbolTable* scope, NodeList* args)
{
	printf("[Analyzer] [Debug] Maybe found a prototype function call...\n");

	PrototypeMethod* ref = NULL;

	Type* function_type = analyzer_handle_prototype(type, field_name, &ref);

	Node* params_head = ref->params == NULL ? NULL : ref->params->head;
	Node* args_head = (args == NULL) ? NULL : args->head;
		
	analyzer_check_arguments(module, params_head, args_head, scope);

	if (function_type == NULL)
	{
		return NULL;
	}
		
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
	 * TODO: Check if module is exporting field.
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
	 * TODO: Check if module is exporting field.
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
	 * TODO: Check if module is exporting method.
	 */

	return field_symbol->symbol_function->return_type;
}

Type* analyzer_get_member_access_type(Module* module, Node* node, SymbolTable* scope, NodeList* args)
{
	int directly = 0;
	
	Type* type = analyzer_return_type_of_expression(module, node->member_access_node.member_access.object, scope, args, 1, &directly);
	char* field_name = node->member_access_node.member_access.member_name;

	int is_ptr = node->member_access_node.member_access.ptr_acess;
	
	int is_module = (type->type == TYPE_MODULE);

	const char* class_name = type->class_name;
	
	if (type->type == TYPE_PTR && !is_ptr)
	{
		printf("[Analyzer] [Debug] Use '->' to access pointers...\n");

		return NULL;
	}

	if (is_module && is_ptr)
	{
		printf("[Analyzer] [Debug] Use '.' to access modules...\n");

		return NULL;
	}

	if (type->type == TYPE_CLASS && is_ptr)
	{
		printf("[Analyzer] [Debug] Use '.' to access pointers...\n");

		return NULL;
	}

	if (type->type != TYPE_CLASS && type->type != TYPE_PTR && is_ptr)
	{
		printf("[Analyzer] [Debug] Use '.' to access prototype methods...\n");

		return NULL;
	}

	if (node->member_access_node.member_access.ptr_acess)
	{
		class_name = type->base->class_name;
	}

	if (node->member_access_node.member_access.object->type == NODE_THIS)
	{
		printf("[Analyzer [Debug] Using 'this' pointer...\n");
	}

	if (is_module)
	{
		if (node->member_access_node.member_access.is_function)
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

	if (node->member_access_node.member_access.is_function && !is_class)
	{
		return handle_prototype_method(type, field_name, module, scope, args);
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

	if (node->member_access_node.member_access.is_function)
	{
		return handle_function(class_symbol, field_name, module, scope, directly, args);
	}

	Node* member = analyzer_get_member_from_class(class_symbol, field_name);
	
	if (member == NULL)
	{
		return NULL;
	}

	if (member->type == NODE_DECLARATION)
	{
		analyzer_check_directly(directly, member, NODE_DECLARATION);
		
		if (member->declare_node.declare.visibility != VISIBILITY_PUBLIC)
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

static Type* analyzer_get_operation_type(Module* module, Node* node, SymbolTable* scope)
{
	Type* left = analyzer_return_type_of_expression(module, node->operation_node.operation.left, scope, NULL, 0, NULL);
	Type* right = analyzer_return_type_of_expression(module, node->operation_node.operation.right, scope, NULL, 0, NULL);

	switch (node->operation_node.operation.op)
	{
		case TOKEN_OPERATOR_OR: // ||
		case TOKEN_OPERATOR_AND: // &&
		{
			return create_type(TYPE_BOOL, NULL);
		}
				
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

static Type* analyzer_get_function_call_type(Module* module, Node* node, SymbolTable* scope, NodeList* args)
{
	args = node->function_call_node.function_call.arguments;
	Node* callee = node->function_call_node.function_call.callee;

	if (callee->type == NODE_IDENTIFIER)
	{
		Symbol* symbol = analyzer_find_symbol_from_scope(callee->variable_node.variable.identifier, scope, 0, 1, 0, 0);

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

		Node* params_head = symbol->symbol_function->params_head;
		Node* args_head = (args == NULL) ? NULL : args->head;

		analyzer_check_arguments(module, params_head, args_head, scope);

		return symbol->symbol_function->return_type;
	}

	Type* type = analyzer_return_type_of_expression(module, callee, scope, args, 0, NULL);
			
	if (type == NULL)
	{
		return NULL;
	}
	
	return type;
}

static Type* analyzer_get_dereference_type(Module* module, Node* node, SymbolTable* scope)
{
	Type* type = NULL;

	Node* ref = node;
	Node* _ref = node;
			
	while (_ref->type == NODE_DEREFERENCE)
	{
		_ref = _ref->dereference_node.dereference.ptr;
	}
			
	if (_ref->type == NODE_IDENTIFIER)
	{
		type = analyzer_return_type_of_expression(module, _ref, scope, NULL, 0, NULL);
	}
			
	int depth = 0;
			
	while (ref->type == NODE_DEREFERENCE)
	{
		ref = ref->dereference_node.dereference.ptr;
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

static Type* analyzer_get_array_access_type(Module* module, Node* node, SymbolTable* scope)
{
	Node* index = node->acess_array_node.acess_array.index_expr;

	Type* type = analyzer_return_type_of_expression(module, node->acess_array_node.acess_array.array, scope, NULL, 0, NULL);

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

static Type* analyzer_get_this_type(Module* module, Node* node, SymbolTable* scope)
{
	analyzer_analyze_node(module, node, scope, NULL);
			
	Type* type = create_type(TYPE_PTR, NULL);

	/**
	 * Cast unsafe, tomar cuidado. em caso de erros: mudar o tipo pra char* no class_name
	 */
	type->base = create_type(TYPE_CLASS, (char*) analyzer_get_class_scope(scope)->owner_statement->symbol_class->identifier);
			
	return type;
}

static Type* analyzer_get_create_instance_type(Node* node, SymbolTable* scope)
{
	Type* type = create_type(TYPE_PTR, NULL);
	type->base = create_type(TYPE_CLASS, node->create_instance_node.create_instance.class_name);
			
	return type;
}

static Type* analyzer_get_adress_of_type(Module* module, Node* node, SymbolTable* scope)
{
	Type* inner = analyzer_return_type_of_expression(module, node->adress_of_node.adress_of.expression, scope, NULL, 0, NULL);
	
	Type* ptr = create_type(TYPE_PTR, NULL);
	ptr->base = inner;
			
	return ptr;
}

static Type* analyzer_get_identifier_type(Node* node, SymbolTable* scope, int member_access, int* direct)
{
	Symbol* symbol = analyzer_find_symbol_from_scope(node->variable_node.variable.identifier, scope, 1, 0, 1, 1);

	if (symbol == NULL)
	{
		printf("[Analyzer] [Debug] Variable not declared: %s...\n", node->variable_node.variable.identifier);
		exit(1);
	}
	
	if (symbol->type == SYMBOL_CLASS)
	{	
		printf("[Analyzer] [Debug] Accessing directly: \"%s\"...\n", node->variable_node.variable.identifier);
			
		*direct = 1;

		return create_type(TYPE_CLASS, (char*) symbol->symbol_class->identifier);
	}

	if (symbol->type == SYMBOL_MODULE)
	{
		printf("[Analyzer] [Debug] Accessing a module: \"%s\"...\n", node->variable_node.variable.identifier);
		
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

static Type* analyzer_get_argument_type(Module* module, Node* node, SymbolTable* scope)
{
	return analyzer_return_type_of_expression(module, node->argument_node.argument.value, scope, NULL, 0, NULL);
}

static Type* analyzer_get_parameter_type(Node* node, SymbolTable* scope)
{
	return node->param_node.param.argument_type;
}

static Type* analyzer_return_type_of_expression(Module* module, Node* expression, SymbolTable* scope, NodeList* args, int member_access, int* direct)
{
	if (expression == NULL)
	{
		printf("[Analyzer] [Debug] Expression is null...\n");
		exit(1);
	}

	switch (expression->type) 
	{
		case NODE_OPERATION:
		{
			return analyzer_get_operation_type(module, expression, scope);
		}

		case NODE_LITERAL:
		{
			return expression->literal_node.literal.literal_type;
		}

		case NODE_IDENTIFIER:
		{
			return analyzer_get_identifier_type(expression, scope, member_access, direct);
		}

		case NODE_CAST:
		{
			return expression->cast_statement_node.cast_node.cast_type;
		}
		
		case NODE_ADRESS_OF:
		{
			return analyzer_get_adress_of_type(module, expression, scope);
		}
		
		case NODE_DEREFERENCE:
		{
			return analyzer_get_dereference_type(module, expression, scope);
		}
		
		case NODE_DECLARATION:
		{
			return expression->declare_node.declare.var_type;
		}
		
		case NODE_CREATE_INSTANCE:
		{
			return analyzer_get_create_instance_type(expression, scope);
		}
		
		case NODE_THIS:
		{
			return analyzer_get_this_type(module, expression, scope);
		}

		case NODE_FUNCTION:
		{
			return expression->function_node.function.return_type;
		}
		
		case NODE_FUNCTION_CALL:
		{
			return analyzer_get_function_call_type(module, expression, scope, args);
		}
		
		case NODE_MEMBER_ACCESS:
		{
			return analyzer_get_member_access_type(module, expression, scope, args);
		}

		case NODE_ARRAY_ACCESS:
		{
			return analyzer_get_array_access_type(module, expression, scope);
		}

		case NODE_ARGUMENT:
		{
			return analyzer_get_argument_type(module, expression, scope);
		}

		case NODE_PARAMETER:
		{
			return analyzer_get_parameter_type(expression, scope);
		}

		case NODE_ARRAY_LITERAL:
		{
			return expression->array_literal_node.array_literal.array_type;
		}
		
		default:
		{
			printf("[Analyzer] [Debug] Invalid expression type: %d...\n", expression->type);
			exit(1);
		}
	}

	return NULL;
}

static int analyzer_is_operation_compatible(Module* module, Node* node, SymbolTable* scope)
{
	OperationNode* operation = &node->operation_node.operation;

	Type* left = analyzer_return_type_of_expression(module, operation->left, scope, NULL, 0, NULL);
	Type* right = analyzer_return_type_of_expression(module, operation->right, scope, NULL, 0, NULL);

	switch (operation->op) 
	{
		/**
		 * Operadores logicos.
		 */
		case TOKEN_OPERATOR_OR: // ||
		case TOKEN_OPERATOR_AND: // &&
		{
			if (analyzer_is_equals(left, TYPE_BOOL) && analyzer_is_equals(right, TYPE_BOOL))
			{
				return 1;
			}

			break;
		}
		
		/**
		 * Operadores aritmeticos.
		 */
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
		
		/**
		 * Operadores comparativos.
		 */
		case TOKEN_OPERATOR_LESS: // <
		case TOKEN_OPERATOR_LESS_EQUALS: // <=
		case TOKEN_OPERATOR_GREATER_EQUALS: // >=
		case TOKEN_OPERATOR_GREATER: // >
		case TOKEN_OPERATOR_EQUALS: // ==
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

static void analyzer_implictly_cast_all(Module* module, Type* preffered, Node* expression, SymbolTable* scope)
{
	Type* type = analyzer_return_type_of_expression(module, expression, scope, NULL, 0, NULL);

	if ((type->type != TYPE_PTR && preffered->type != TYPE_PTR) && (type->type != TYPE_ARRAY && preffered->type != TYPE_ARRAY))
	{
		if (type->type == preffered->type)
		{
			printf("[Analyzer] [Debug] Not casting, types are the same...\n");

			return;
		}
	}

	if (!analyzer_is_compatible(type, preffered, scope))
	{
		printf("[Analyzer] [Debug] Expression type is incompatible...\n");
		exit(1);
	}

	if (!analyzer_is_the_same(type, preffered))
	{
		analyzer_create_cast(&expression, preffered);
	}
	else
	{
		printf("[Analyzer] [Debug] Not casting, types are the same...\n");
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

static void analyzer_handle_variable_declaration(Module* module, Node* node, SymbolTable* scope, int* offset)
{ 
	if (analyzer_find_symbol_only_from_scope(node->declare_node.declare.identifier, scope, 0, 0, 1) != NULL)
	{
		printf("[Analyzer] [Debug] Statement with identifier already declared in scope...\n");
		exit(1);
	}

	analyzer_analyze_type(node->declare_node.declare.var_type, scope);

	Symbol* owner_function = analyzer_get_owner_function(scope);

	if (!node->declare_node.declare.is_static && owner_function != NULL)
	{
		printf("[Analyzer] [Debug] Local variable \"%s\", adding offset...\n", node->declare_node.declare.identifier);

		int size = analyzer_get_type_size(node->declare_node.declare.var_type, scope);
		*offset -= size;
	}

	analyzer_add_symbol_to_scope(module, node, scope, offset, 0);
	
	Node* expression = node->declare_node.declare.default_value;

	if (expression != NULL)
	{
		analyzer_analyze_node(module, expression, scope, NULL);

		analyzer_implictly_cast_all(module, node->declare_node.declare.var_type, expression, scope);
	}
}

static void analyzer_handle_parameters(Module* module, Node* head, SymbolTable* scope, int* offset)
{
	Node* next = head;

	while (next != NULL)
	{
		int size = analyzer_get_type_size(next->param_node.param.argument_type, scope);

		*offset += size;

		if (analyzer_find_symbol_only_from_scope(next->param_node.param.identifier, scope, 0, 0, 0) != NULL)
		{
			printf("[Analyzer] [Debug] Parameter with name: \"%s\" already declared...\n", next->param_node.param.identifier);
			exit(1);
		}

		analyzer_add_symbol_to_scope(module, next, scope, offset, 0);

		next = next->next;
	}
}

int analyzer_check_parameters(Node* params_head, Node* _params_head)
{
	int function_size = (params_head == NULL) ? 0 : analyzer_get_list_size(params_head);
	int _function_size = (_params_head == NULL) ? 0 : analyzer_get_list_size(_params_head);
	
	if (function_size != _function_size)
	{
		return 0;
	}

	int i = 0;

	Node* parameter = params_head;
	Node* _parameter = _params_head;
	
	while (i < function_size)
	{
		if (parameter == NULL || _parameter == NULL)
		{
			return 0;
		}

		if (strcmp(parameter->param_node.param.identifier, _parameter->param_node.param.identifier) != 0)
		{
			return 0;
		}

		if (!analyzer_is_the_same(parameter->param_node.param.argument_type, _parameter->param_node.param.argument_type))
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
		/**
		 * Caso a classe não extenda nenhuma e de override em uma função.
		 */
		if (symbol->symbol_function->is_override)
		{
			printf("[Analyzer] [Debug] Overriding a non declared function...\n");
			exit(1);
		}

		return;
	}
	
	Node* member = analyzer_get_function_from_class(super_class, (char*) symbol->symbol_function->identifier);

	if (member == NULL)
	{
		/**
		 * Quando a função não existe na super class porém você da override.
		 */
		if (symbol->symbol_function->is_override)
		{
			printf("[Analyzer] [Debug] Overriding a non declared function...\n");
			exit(1);
		}

		return;
	}

	int flag = analyzer_is_the_same(symbol->symbol_function->return_type, member->function_node.function.return_type);
	int _flag = analyzer_check_parameters(symbol->symbol_function->params_head, member->function_node.function.params->head);
	
	if (!flag || !_flag)
	{
		printf("[Analyzer] [Debug] Types / Parameters aren't the same...\n");
		exit(1);
	}
	
	if (member->function_node.function.is_virtual)
	{
		/**
		 * Quando a função da super class é virtual porém você não da override.
		 */
		if (!symbol->symbol_function->is_override)
		{
			printf("[Analyzer] [Debug] Function needs the 'override' modifier...\n");
			exit(1);
		}

		return;
	}
	
	/**
	 * Quando a função da super class existe, nao é virtual e você declara outra com mesmo nome.
	 * - shadowing não é permitido e não tem como dar override em funções não virtuais.
	 */
	printf("[Analyzer] [Debug] Can't apply shadowing on functions...\n");
	exit(1);
}

static void analyzer_handle_function_declaration(Module* module, Node* node, SymbolTable* scope)
{
	const FunctionNode* function_node = &node->function_node.function;
	
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

	// offset das variaveis locais.
	// é pra stack, começa em 0 vai diminuíndo.
	int local_offset = 0;

	// offset dos parametros.
	// começa com 8 e vai aumentando.
	int param_offset = 8;

	analyzer_analyze_type(node->function_node.function.return_type, scope);

	Symbol* func_symbol = analyzer_add_symbol_to_scope(module, node, scope, NULL, 0);

	func_symbol->is_prototype = 0;

	SymbolTable* block_scope = analyzer_create_scope(SYMBOL_FUNCTION, scope, func_symbol);

	if (scope->scope_kind == SYMBOL_CLASS)
	{
		Symbol* super = scope->owner_statement->symbol_class->super;

		analyzer_check_function(func_symbol, (super == NULL) ? NULL : super);
	}

	if (function_node->params != NULL)
	{
		analyzer_handle_parameters(module, function_node->params->head, block_scope, &param_offset);
	}

	analyzer_analyze_node(module, function_node->block_node, block_scope, &local_offset);
}

static void analyzer_create_cast(Node** node, Type* preferred)
{
	printf("[Analyzer] [Debug] Creating a cast node, casting type: %d\n", preferred->type);
	
	Node* cast = malloc(sizeof(Node));

	if (cast == NULL)
	{
		printf("[Analyzer] [Debug] Failed to alllocate memory for cast node...\n");
		exit(1);
	}

	cast->type = NODE_CAST;
	cast->cast_statement_node.cast_node.cast_type = preferred;
	cast->cast_statement_node.cast_node.expression = *node;

	*node = cast;
}

static void analyzer_implictly_cast_operation(Module* module, Node* node, SymbolTable* scope)
{
	OperationNode* operation = &node->operation_node.operation;

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

static void analyzer_handle_operation(Module* module, Node* node, SymbolTable* scope)
{
	OperationNode* operation = &node->operation_node.operation;
	
	analyzer_analyze_node(module, operation->left, scope, NULL);
	analyzer_analyze_node(module, operation->right, scope, NULL);
	
	if (!analyzer_is_operation_compatible(module, node, scope))
	{
		printf("[Analyzer] [Debug] Incompatible operation...\n");
		exit(1);
	}

	analyzer_implictly_cast_operation(module, node, scope);
}

static void analyzer_check_global_scope(Node* node, SymbolTable* scope)
{
	const NodeType type = node->type;
	const SymbolType kind = scope->scope_kind;
	
	if (type == NODE_FUNCTION && kind != GLOBAL_SCOPE && kind != SYMBOL_CLASS)
	{
		printf("[Analyzer] [Debug] Function declaration statement isn't on global scope...\n");
		exit(1);
	}

	if (type == NODE_IMPORT && kind != GLOBAL_SCOPE)
	{
		printf("[Analyzer] [Debug] Function declaration statement isn't on global scope...\n");
		exit(1);
	}

	if (type != NODE_FUNCTION && type != NODE_LITERAL && type != NODE_CLASS && type != NODE_DECLARATION && type != NODE_IMPORT && kind == GLOBAL_SCOPE)
	{
		printf("[Analyzer] [Debug] Statement can't be in a global scope: %d...\n", type);
		exit(1);
	}
}

static void analyzer_handle_block(Module* module, Node* node, SymbolTable* scope, int* local_offset)
{
	Node* next = node->block_node.block.statements->head;
	
	while (next != NULL)
	{
		if (next->type == NODE_DECLARATION || next->type == NODE_IF || next->type == NODE_WHILE_LOOP || next->type == NODE_FOR_LOOP || next->type == NODE_SWITCH_STATEMENT)
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

static void analyzer_handle_return(Module* module, Node* node, SymbolTable* scope)
{
	Node* return_value = node->return_statement_node.return_statement.return_value;
	
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

static int analyzer_get_list_size(Node* list_head)
{
	Node* next = list_head;

	int count = 0;

	while (next !=  NULL)
	{
		next = next->next;
		count++;
	}

	return count;
}

static void analyzer_check_arguments(Module* module, Node* params_head, Node* args_head, SymbolTable* scope)
{
	int arguments_size = (args_head == NULL) ? 0 : analyzer_get_list_size(args_head);
	int params_size = (params_head == NULL) ? 0 : analyzer_get_list_size(params_head);
	
	if (arguments_size != params_size)
	{
		printf("[Analyzer] [Debug] Expected argument size is: %d...\n", params_size);
		printf("[Analyzer] [Debug] Params size: %d, Args size: %d...\n", params_size, arguments_size);
		exit(1);
	}

	Node* current = args_head;
	Node* current_ = params_head;

	while (current != NULL)
	{
		Type* type = analyzer_return_type_of_expression(module, current->argument_node.argument.value, scope, NULL, 0, NULL);
		Type* type_ = current_->param_node.param.argument_type;

		if (!analyzer_is_type_identic(type, type_, scope))
		{
			analyzer_implictly_cast_all(module, type_, current, scope);
		}
		
		current = current->next;
		current_ = current_->next;
	}
}

static char* get_function_call_name(Node* callee)
{
	if (callee->type == NODE_MEMBER_ACCESS)
	{
		return callee->member_access_node.member_access.member_name;
	}

	if (callee->type == NODE_IDENTIFIER)
	{
		return callee->variable_node.variable.identifier;
	}

	return "";
}

static void analyzer_handle_function_call(Module* module, Node* node, SymbolTable* scope)
{
	printf("[Analyzer] [Debug] Trying to call a function...\n");
	
	Type* type = analyzer_return_type_of_expression(module, node, scope, NULL, 0, NULL);

	if (type == NULL)
	{
		printf("[Analyzer] [Debug] Function not found: \"%s\"...\n", get_function_call_name(node->function_call_node.function_call.callee));
		exit(1);
	}
}

static void analyzer_handle_if(Module* module, Node* node, SymbolTable* scope, int* offset)
{
	analyzer_analyze_node(module, node->if_statement_node.if_statement.condition_top, scope, NULL);

	if (analyzer_return_type_of_expression(module, node->if_statement_node.if_statement.condition_top, scope, NULL, 0, NULL)->type != TYPE_BOOL)
	{
		printf("[Analyzer] [Debug] Invalid if condition, expression return type needs to be 'boolean'...\n");
		exit(1);
	}

	SymbolTable* then_scope = analyzer_create_scope(SYMBOL_IF, scope, NULL);
	
	analyzer_analyze_node(module, node->if_statement_node.if_statement.then_branch, then_scope, offset);

	if (node->if_statement_node.if_statement.else_branch != NULL)
	{
		SymbolTable* else_scope = analyzer_create_scope(SYMBOL_ELSE, scope, NULL);
		analyzer_analyze_node(module, node->if_statement_node.if_statement.else_branch, else_scope, offset);
	}
}

static void analyzer_handle_adress_of(Node* node, SymbolTable* scope)
{
	const AdressOfNode* adress_of = &node->adress_of_node.adress_of;
	
	if (adress_of->expression->type != NODE_IDENTIFIER)
	{
		printf("[Analyzer] [Debug] Adressing requires a pointer variable...\n");
		exit(1);
	}
	
	Symbol* var = analyzer_find_symbol_from_scope(adress_of->expression->variable_node.variable.identifier, scope, 1, 0, 0, 0);
	
	if (var == NULL)
	{
		printf("[Analyzer] [Debug] Referencing a not declared variable...\n");
		exit(1);
	}
}

static void analyzer_handle_dereference(Node* node, SymbolTable* scope)
{
	Node* ref = node;

	while (ref->type == NODE_DEREFERENCE)
	{
		ref = ref->dereference_node.dereference.ptr;
	}

	if (ref->type != NODE_IDENTIFIER)
	{
		printf("[Analyzer] [Debug] Dereferencing requires a pointer variable...\n");
		exit(1);
	}
	
	Symbol* field = analyzer_find_symbol_from_scope(ref->variable_node.variable.identifier, scope, 1, 0, 0, 0);
	
	if (field == NULL)
	{
		printf("[Analyzer] [Debug] Dereferencing a not declared variable...\n");
		exit(1);
	}
}

static void analyzer_handle_while_loop(Module* module, Node* node, SymbolTable* scope, int* offset)
{
	analyzer_analyze_node(module, node->while_loop_node.while_loop.condition, scope, NULL);

	if (analyzer_return_type_of_expression(module, node->while_loop_node.while_loop.condition, scope, NULL, 0, NULL)->type != TYPE_BOOL)
	{
		printf("[Analyzer] [Debug] Invalid while condition, expression return type needs to be 'boolean'...\n");
		exit(1);
	}

	SymbolTable* block_scope = analyzer_create_scope(SYMBOL_WHILE, scope, NULL);
	analyzer_analyze_node(module, node->while_loop_node.while_loop.then_block, block_scope, offset);
}

static void analyzer_handle_for_loop(Module* module, Node* node, SymbolTable* scope, int* offset)
{
	SymbolTable* block_scope = analyzer_create_scope(SYMBOL_FOR, scope, NULL);
	
	analyzer_analyze_node(module, node->for_loop_node.for_loop.init, block_scope, offset);

	analyzer_analyze_node(module, node->for_loop_node.for_loop.condition, block_scope, offset);
	analyzer_analyze_node(module, node->for_loop_node.for_loop.then_statement, block_scope, offset);

	if (analyzer_return_type_of_expression(module, node->for_loop_node.for_loop.condition, block_scope, NULL, 0, NULL)->type != TYPE_BOOL)
	{
		printf("[Analyzer] [Debug] Invalid for condition, expression return type needs to be 'boolean'...\n");
		exit(1);
	}

	analyzer_analyze_node(module, node->for_loop_node.for_loop.then_block, block_scope, offset);
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

static void analyzer_handle_break(Node* node, SymbolTable* scope)
{
	if (!analyzer_is_inside(scope, SYMBOL_FOR) && !analyzer_is_inside(scope, SYMBOL_WHILE) && !analyzer_is_inside(scope, SYMBOL_SWITCH))
	{
		printf("[Analyzer] [Debug] Break statement outside a loop or switch statement...\n");
		exit(1);
	}
}

static void analyzer_handle_var_assign(Module* module, Node* node, SymbolTable* scope)
{
	Node* expression = node->variable_assign_node.variable_assign.assign_value;
	Node* left = node->variable_assign_node.variable_assign.left;
	
	Type* type = analyzer_return_type_of_expression(module, left, scope, NULL, 0, NULL);

	if (type == NULL)
	{
		printf("[Analyzer] [Debug] Member not found...\n");
		exit(1);
	}

	analyzer_analyze_node(module, expression, scope, NULL); // analyze function already checks if things exist...
	analyzer_analyze_node(module, expression, scope, NULL);

	if (left->type == NODE_CAST)
	{
		printf("[Analyzer] [Debug] Can't assign value to a cast...\n");
		exit(1);
	}

	analyzer_implictly_cast_all(module, type, expression, scope);
}

static Node* analyzer_get_function_from_class(Symbol* class, char* func_name)
{
	for (int i = 0; i < class->symbol_class->func_count; i++)
	{
		Node* member = class->symbol_class->functions[i];

		if (strcmp(member->function_node.function.identifier, func_name) == 0)
		{
			return member;
		}
	}

	return NULL;
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

static Node* analyzer_get_member_from_class(Symbol* class, char* member_name)
{
	if (class == NULL)
	{
		return NULL;
	}
	
	for (int i = 0; i < class->symbol_class->field_count; i++)
	{
		Node* member = class->symbol_class->fields[i];

		if (strcmp(member->declare_node.declare.identifier, member_name) == 0)
		{
			return member;
		}
	}

	return analyzer_get_member_from_class(class->symbol_class->super, member_name);
}

static void analyzer_handle_class_funcs(Module* module, Node* node, SymbolTable* scope)
{
	const ClassNode* class_node = &node->class_node.class_node;
	
	for (int i = 0; i < class_node->func_count; i++)
	{
		analyzer_analyze_node(module, class_node->func_declare_list[i], scope, NULL);
	}
}

static void analyzer_handle_class_vars(Module* module, Node* node, SymbolTable* scope)
{
	const ClassNode* class_node = &node->class_node.class_node;
	
	for (int i = 0; i < class_node->var_count; i++)
	{
		analyzer_analyze_node(module, class_node->var_declare_list[i], scope, NULL);
	}
}

static void analyzer_handle_class_declaration(Module* module, Node* node, SymbolTable* scope)
{
	const ClassNode* class_node = &node->class_node.class_node;
	
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
	}
	else 
	{
		printf("[Analyzer] [Debug] Class dont have a super class...\n");
	}
	
	class_symbol->symbol_class->class_scope = class_scope;
	class_symbol->symbol_class->constructor = NULL;
	
	if (class_node->constructor != NULL)
	{
		analyzer_analyze_node(module, class_node->constructor, class_scope, NULL);
		class_symbol->symbol_class->constructor = analyzer_find_symbol_from_scope(class_node->constructor->function_node.function.identifier, class_scope, 0, 1, 0, 0);
	}
	
	analyzer_handle_class_vars(module, node, class_scope);
	analyzer_handle_class_funcs(module, node, class_scope);
}

static void analyzer_handle_create_instance(Module* module, Node* node, SymbolTable* scope)
{
	const CreateInstanceNode* instance_node = &node->create_instance_node.create_instance;
	
	Symbol* class = analyzer_find_symbol_from_scope(instance_node->class_name, scope, 0, 0, 1, 0);
	
	if (class == NULL)
	{
		printf("[Analyzer] [Debug] Failed to create a instance, class not found: %s...", instance_node->class_name);
		exit(1);
	}
	
	if (class->symbol_class->constructor != NULL)
	{
		Node* params_head = class->symbol_class->constructor->symbol_function->params_head;
		Node* args_head = (instance_node->constructor_args == NULL) ? NULL : instance_node->constructor_args->head;

		analyzer_check_arguments(module, params_head, args_head, scope);
	}
}

static int analyzer_is_able_to_this(SymbolTable* scope)
{
	if (scope->scope_kind != SYMBOL_FUNCTION)
	{
		printf("[Analyzer] [Debug] Pointer 'this' needs to be used inside a function inside a class scope...\n");

		return 0;
	}

	if (scope->parent == NULL)
	{
		printf("[Analyzer] [Debug] Pointer 'this' needs to be used inside a class scope...\n");

		return 0;
	}

	if (scope->parent->scope_kind != SYMBOL_CLASS)
	{
		printf("[Analyzer] [Debug] Pointer 'this' needs to be used inside a class scope...\n");

		return 0;
	}

	if (scope->owner_statement->symbol_function->is_static)
	{
		printf("[Analyzer] [Debug] Pointer 'this' can't be used inside a static function...\n");

		return 0;
	}

	return 1;
}

static void analyzer_handle_this(Node* node, SymbolTable* scope)
{
	if (!analyzer_is_able_to_this(scope))
	{
		exit(1);
	}
}

static void analyzer_handle_member_access(Module* module, Node* node, SymbolTable* scope)
{
	printf("[Analyzer] [Debug] Trying to acess a member from class object...\n");
	
	Type* type = analyzer_return_type_of_expression(module, node, scope, NULL, 0, NULL);

	if (type == NULL)
	{
		printf("[Analyzer] [Debug] Member not found...\n");
		exit(1);
	}
}

static void analyzer_handle_array_access(Module* module, Node* node, SymbolTable* scope)
{
	Type* type = analyzer_return_type_of_expression(module, node, scope, NULL, 0, NULL);

	if (type == NULL)
	{
		printf("[Analyzer] [Debug] Failed to access array...\n");

		exit(1);
	}
}

static void analyzer_handle_cast(Module* module, Node* node, SymbolTable* scope)
{
	printf("[Analyzer] [Debug] Found a cast node...\n");
	
	Type* expression_type = analyzer_return_type_of_expression(module, node->cast_statement_node.cast_node.expression, scope, NULL, 0, NULL);

	if (expression_type == NULL)
	{
		printf("[Analyzer] [Debug] Invalid expression in cast node...\n");
		exit(1);
	}

	if (!analyzer_is_compatible(node->cast_statement_node.cast_node.cast_type, expression_type, scope))
	{
		printf("[Analyzer] [Debug] Incompatible cast types...\n");
		exit(1);
	}
}

static void analyzer_handle_identifier(Node* node, SymbolTable* scope)
{
	Symbol* symbol = analyzer_find_symbol_from_scope(node->variable_node.variable.identifier, scope, 1, 0, 0, 0);

	if (symbol == NULL)
	{
		printf("[Analyzer] [Debug] Field with name: \"%s\" not found...\n", node->variable_node.variable.identifier);
		exit(1);
	}
}

static void analyzer_check_array_literal_values(Module* module, Node* head, Type* type, SymbolTable* scope)
{
	Node* current = head;
	
	// Pula o primeiro tipo (array), ja que a array necessita de varios daquele tipo.
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

static void analyzer_handle_array_literal(Module* module, Node* node, SymbolTable* scope)
{
	NodeList* values = node->array_literal_node.array_literal.values;
	Type* type = node->array_literal_node.array_literal.array_type;
	
	analyzer_analyze_type(type, scope);

	if (values != NULL)
	{
		analyzer_check_array_literal_values(module, values->head, type, scope);
	}
}

static void analyzer_handle_switch_case(Module* module, Node* node, SymbolTable* scope, Type* required_type, int* offset)
{
	Node* condition = node->switch_case_block_node.switch_case_block.condition;
	Node* block = node->switch_case_block_node.switch_case_block.block;

	Type* expr_type = analyzer_return_type_of_expression(module, condition, scope, NULL, 0, NULL);

	if (!analyzer_is_type_identic(expr_type, required_type, scope))
	{
		printf("[Analyzer] [Debug] Invalid expression return type...\n");
		exit(1);
	} 

	analyzer_analyze_node(module, block, scope, offset);
}

static void analyzer_handle_switch_statement(Module* module, Node* node, SymbolTable* scope, int* offset)
{
	NodeList* case_list = node->switch_statement_node.switch_statement.case_list;
	
	analyzer_analyze_node(module, node->switch_statement_node.switch_statement.value, scope, offset);

	Node* next = case_list->head;

	Type* required_type = analyzer_return_type_of_expression(module, node->switch_statement_node.switch_statement.value, scope, NULL, 0, NULL);

	SymbolTable* switch_scope = analyzer_create_scope(SYMBOL_SWITCH, scope, NULL);

	while (next != NULL)
	{
		SymbolTable* case_scope = NULL;

		if (next->switch_case_block_node.switch_case_block.new_scope)
		{
			case_scope = analyzer_create_scope(SYMBOL_SWITCH_CASE, switch_scope, NULL);
		}

		analyzer_handle_switch_case(module, next, case_scope != NULL ? case_scope : switch_scope, required_type, offset);

		next = next->next;
	}
}

static char* handle_relative_path(char* abs_path, char* relative)
{
	return resolve_path(abs_path, relative);
}

static void analyzer_handle_local_import(Module* module, Node* node, SymbolTable* scope)
{
	ModuleStack* stack = module->stack;

	if (stack == NULL)
	{
		stack = setup_module_stack();
	}
	
	char* path = handle_relative_path(module->handler->root_path, node->import_statement_node.import_node.import_path);

	printf("\n%s: ---------------------------------------------------------------+\n\n", node->import_statement_node.import_node.import_path);

	Module* import_module = compile_module(module->handler, stack, path);

	printf("\n+---------------------------------------------------------------------------------+\n\n");

	Symbol* symbol = analyzer_add_symbol_to_scope(module, node, scope, NULL, 0);
	symbol->symbol_module->module = import_module;

	add_module_to_list(module, import_module);

	// A variavel path é usado agora apenas, depois é copiada.
	free(path);
}

static void analyzer_handle_import(Module* module, Node* node, SymbolTable* scope)
{
	if (analyzer_find_symbol_from_scope(node->import_statement_node.import_node.identifier, scope, 1, 1, 1, 1) != NULL)
	{
		printf("[Analyzer] [Debug] Statement with identifier: \"%s\" already declared...\n", node->import_statement_node.import_node.identifier);
		exit(1);
	}
	
	if (node->import_statement_node.import_node.is_local)
	{
		analyzer_handle_local_import(module, node, scope);
	}
	else
	{
		/**
		 * TODO: Dar handle em imports de libs imbutidas.
		 */
	}
}

static void analyzer_analyze_node(Module* module, Node* node, SymbolTable* scope, int* offset)
{
	analyzer_check_global_scope(node, scope);
	
	switch (node->type) 
	{
		case NODE_DECLARATION:
		{
			analyzer_handle_variable_declaration(module, node, scope, offset);
			
			return;
		}
		
		case NODE_FUNCTION:
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
		
		case NODE_IDENTIFIER:
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

		case NODE_SWITCH_STATEMENT:
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
		
		case NODE_RETURN:
		{
			analyzer_handle_return(module, node, scope);
			
			return;
		}
		
		case NODE_BREAK:
		{
			analyzer_handle_break(node, scope);
			
			return;
		}
		
		case NODE_ADRESS_OF:
		{
			analyzer_handle_adress_of(node, scope);
			
			return;
		}
		
		case NODE_DEREFERENCE:
		{
			analyzer_handle_dereference(node, scope);
			
			return;
		}
		
		case NODE_VARIABLE_ASSIGN:
		{
			analyzer_handle_var_assign(module, node, scope);
			
			return;
		}
		
		case NODE_MEMBER_ACCESS:
		{
			analyzer_handle_member_access(module, node, scope);
			
			return;
		}

		case NODE_FUNCTION_CALL:
		{
			analyzer_handle_function_call(module, node, scope);
			
			return;
		}
		
		case NODE_CLASS:
		{
			analyzer_handle_class_declaration(module, node, scope);

			return;
		}

		case NODE_CREATE_INSTANCE:
		{
			analyzer_handle_create_instance(module, node, scope);

			return;
		}

		case NODE_THIS:
		{
			analyzer_handle_this(node, scope);

			return;
		}

		case NODE_ARRAY_ACCESS:
		{
			analyzer_handle_array_access(module, node, scope);

			return;
		}

		case NODE_CAST:
		{
			analyzer_handle_cast(module, node, scope);

			return;
		}

		case NODE_ARRAY_LITERAL:
		{
			analyzer_handle_array_literal(module, node, scope);

			return;
		}

		default:
		{
			printf("[Analyzer] [Debug] Node type not implemented in analyzer: \"%d\"...", node->type);
			exit(1);
		}
	}
}