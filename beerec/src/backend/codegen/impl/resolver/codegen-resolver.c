#include <stdlib.h>

#include "codegen-resolver.h"
#include "../../../../frontend/semantic/analyzer/analyzer.h"

static Register* resolve_expression(SymbolTable* scope, Node* node, int free);

static void add_register_to_blacklist(Register* reg, Register*** blacklist, int* length)
{
	for (int i = 0; i < *length; i++)
	{
		if (*blacklist[i] == reg)
		{
			return;
		}
	}
	
	if (*blacklist == NULL)
	{
		*blacklist = malloc(sizeof(Register*) * 4); // não precisa de mais que isso.
	}

	*blacklist[*length] = reg;
	(*length)++;
}

static char* get_method_name(Node* callee)
{
	if (callee->type == NODE_IDENTIFIER)
	{
		return callee->variable.identifier;
	}

	if (callee->type == NODE_MEMBER_ACCESS)
	{
		return callee->member_access.member_name;
	}

	return NULL;
}

static Register* resolve_operation(SymbolTable* scope, Node* node, int free)
{
	Register* left = resolve_expression(scope, node->operation.left, 1); // retorna um registrador caso ele seja fixo.
	Register* right = resolve_expression(scope, node->operation.right, 1); // retorna um registrador caso ele seja fixo.

	node->operation.registers_blacklist = NULL;
	node->operation.blacklist_length = 0;

	if ((left != NULL && right != NULL) && (left == right))
	{
		node->operation.change_left = 1;
	}

	node->operation.unuse_fix_return = free;

	if (left != NULL)
	{
		add_register_to_blacklist(left, &(node->operation.registers_blacklist), &(node->operation.blacklist_length));
	}

	if (right != NULL)
	{
		add_register_to_blacklist(right, &(node->operation.registers_blacklist), &(node->operation.blacklist_length));
	}
	
	Type* type = analyzer_return_type_of_expression(NULL, node, scope, NULL, 0, NULL);
	int is_floating = (type->type == TYPE_FLOAT || type->type == TYPE_DOUBLE);
	
	Register* result = NULL;

	switch (node->operation.op)
	{
		case TOKEN_OPERATOR_PLUS:
		{
			result = left;
		}

		case TOKEN_OPERATOR_MINUS:
		{
			result = left;
		}

		case TOKEN_CHAR_STAR:
		{
			result = left;
		}

		case TOKEN_OPERATOR_DIVIDED:
		{
			result = (is_floating) ? left : find_register_by_name("eax", type);
		}

		case TOKEN_OPERATOR_INCREMENT:
		{
			return NULL; // não retorna um operador fixo.
		}

		case TOKEN_OPERATOR_DECREMENT:
		{
			return NULL; // não retorna um operador fixo.
		}

		default:
		{
			return NULL;
		}
	}

	node->operation.fix_return = result;

	return result;
}

static Register* resolve_method_call(SymbolTable* scope, Node* node)
{
	char* method_name = get_method_name(node->function_call.callee);
	Symbol* symbol = analyzer_find_symbol_from_scope(method_name, scope, 0, 1, 0, 0);

	Type* type = symbol->symbol_function->return_type;

	switch (type->type)
	{
		case TYPE_INT:
		{
			return find_register_by_name("rax", type);
		}

		case TYPE_CHAR:
		{
			return find_register_by_name("rax", type);
		}

		case TYPE_FLOAT:
		{
			return find_register_by_name("xmm0", type);
		}
		
		case TYPE_DOUBLE:
		{
			return find_register_by_name("xmm0", type);
		}

		case TYPE_BOOL:
		{
			return find_register_by_name("rax", type);
		}
		
		default:
		{
			break;
		}
	}

	return NULL;
}

static Register* resolve_expression(SymbolTable* scope, Node* node, int free)
{
	switch (node->type)
	{
		case NODE_OPERATION:
		{
			return resolve_operation(scope, node, free);
		}

		case NODE_FUNCTION_CALL:
		{
			return resolve_method_call(scope, node);
		}
		
		default:
		{
			break;
		}
	}

	return NULL;
}

void resolve_instruction(SymbolTable* scope, Node* node)
{
	switch (node->type)
	{
		case NODE_DECLARATION:
		{
			if (node->declare.default_value != NULL)
			{
				resolve_expression(scope, node->declare.default_value, 0);
			}

			break;
		}

		case NODE_OPERATION:
		{
			resolve_operation(scope, node, 1);

			break;
		}

		case NODE_VARIABLE_ASSIGN:
		{
			resolve_expression(scope, node->variable_assign.assign_value, 0);

			break;
		}

		case NODE_IF:
		{
			resolve_expression(scope, node->if_statement.condition_top, 0);

			Node* next = node->if_statement.then_branch->block.statements->head;

			while (next != NULL)
			{
				resolve_instruction(node->if_statement.then_scope, next);

				next = next->next;
			}

			if (node->if_statement.then_branch != NULL)
			{
				Node* next = node->if_statement.else_branch->block.statements->head;

				while (next != NULL)
				{
					resolve_instruction(node->if_statement.else_scope, next);

					next = next->next;
				}
			}

			break;
		}
		
		default:
		{
			break;
		}
	}
}