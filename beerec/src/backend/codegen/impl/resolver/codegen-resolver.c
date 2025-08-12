#include "codegen-resolver.h"
#include "../../../../frontend/semantic/analyzer/analyzer.h"

static Register* resolve_expression(SymbolTable* scope, Node* node);

static char* get_method_name(Node* callee)
{
	if (callee->type == NODE_IDENTIFIER)
	{
		return callee->variable_node.variable.identifier;
	}

	if (callee->type == NODE_MEMBER_ACCESS)
	{
		return callee->member_access_node.member_access.member_name;
	}

	return NULL;
}

static Register* resolve_operation(SymbolTable* scope, Node* node)
{
	Register* left = resolve_expression(scope, node->operation_node.operation.left); // retorna um registrador caso ele seja fixo.
	Register* right = resolve_expression(scope, node->operation_node.operation.right); // retorna um registrador caso ele seja fixo.

	if (left == right)
	{
		/* ADICIONAR CHECK PRA CASO LEFT FOR IGUAL A RIGHT, ELE MOVE O LEFT PRA OUTRO REGISTER */
	}
	
	Type* type = analyzer_return_type_of_expression(NULL, node, scope, NULL, 0, NULL);
	int is_floating = (type->type == TYPE_FLOAT || type->type == TYPE_DOUBLE);
	
	switch (node->operation_node.operation.op)
	{
		case TOKEN_OPERATOR_PLUS:
		{
			return left;
		}

		case TOKEN_OPERATOR_MINUS:
		{
			return left;
		}

		case TOKEN_CHAR_STAR:
		{
			return left;
		}

		case TOKEN_OPERATOR_DIVIDED:
		{
			return (is_floating) ? left : find_register_by_name("eax", type);
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
			break;
		}
	}

	return NULL;
}

static Register* resolve_method_call(SymbolTable* scope, Node* node)
{
	char* method_name = get_method_name(node->function_call_node.function_call.callee);
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

static Register* resolve_expression(SymbolTable* scope, Node* node)
{
	switch (node->type)
	{
		case NODE_OPERATION:
		{
			return resolve_expression(scope, node);
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
			if (node->declare_node.declare.default_value != NULL)
			{
				resolve_expression(scope, node->declare_node.declare.default_value);
			}

			break;
		}

		case NODE_OPERATION:
		{
			resolve_operation(scope, node);

			break;
		}

		case NODE_VARIABLE_ASSIGN:
		{
			resolve_expression(scope, node->variable_assign_node.variable_assign.assign_value);

			break;
		}

		case NODE_IF:
		{
			resolve_expression(scope, node->if_statement_node.if_statement.condition_top);

			Node* next = node->if_statement_node.if_statement.then_branch->block_node.block.statements->head;

			while (next != NULL)
			{
				resolve_instruction(node->if_statement_node.if_statement.then_scope, next);

				next = next->next;
			}

			if (node->if_statement_node.if_statement.then_branch != NULL)
			{
				Node* next = node->if_statement_node.if_statement.else_branch->block_node.block.statements->head;

				while (next != NULL)
				{
					resolve_instruction(node->if_statement_node.if_statement.else_scope, next);

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