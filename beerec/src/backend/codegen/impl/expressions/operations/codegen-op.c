#include <stdlib.h>

#include "codegen-op.h"
#include "../../../codegen.h"
#include "../arithmetic/codegen-arith.h"
#include "../comparative/codegen-comparative.h"
#include "../codegen-expr.h"

AsmReturn* generate_operation(CodeGen* codegen, Node* node, AsmArea* area, int force_reg)
{
	OperationNode* op_node = &node->operation;
	
	int right_force_reg = 1;
	int left_force_reg = 1;

	int left_prefer_third = 0;
	
	if (op_node->right->type == NODE_LITERAL)
	{
		right_force_reg = 0;
	}

	if (op_node->op == TOKEN_OPERATOR_INCREMENT || op_node->op == TOKEN_OPERATOR_DECREMENT || op_node->op == TOKEN_OPERATOR_PLUS_EQUALS || op_node->op == TOKEN_OPERATOR_MINUS_EQUALS || op_node->op == TOKEN_OPERATOR_TIMES_EQUALS || op_node->op == TOKEN_OPERATOR_DIVIDED_EQUALS)
	{
		left_force_reg = 0;
		left_prefer_third = 1;
	}
	
	/* DAR HANDLE NOS REGISTERS DA BLACKLIST */
	/* IMPLEMENTAR OS CHECKS PRA FIX RETURN TAMBÉM */
	
	AsmReturn* left = generate_expression(codegen, node->operation.left, area, left_force_reg);
	AsmReturn* right = NULL;
	
	if (op_node->op != TOKEN_OPERATOR_INCREMENT && op_node->op != TOKEN_OPERATOR_DECREMENT)
	{
		right = generate_expression(codegen, node->operation.right, area, right_force_reg);
	}

	AsmReturn* result = NULL;

	switch (op_node->op)
	{
		case TOKEN_OPERATOR_EQUALS:
		{
			result = generate_is_equals_operation(codegen, left, right, area);

			break;
		}
		
		case TOKEN_OPERATOR_NOT_EQUALS:
		{
			result = generate_is_not_equals_operation(codegen, left, right, area);

			break;
		}
		
		case TOKEN_OPERATOR_GREATER:
		{
			result = generate_is_greater_operation(codegen, left, right, area);

			break;
		}
		
		case TOKEN_OPERATOR_LESS:
		{
			result = generate_is_less_operation(codegen, left, right, area);

			break;
		}
		
		case TOKEN_OPERATOR_GREATER_EQUALS:
		{
			result = generate_is_greater_equals_operation(codegen, left, right, area);

			break;
		}
		
		case TOKEN_OPERATOR_LESS_EQUALS:
		{
			result = generate_is_less_equals_operation(codegen, left, right, area);

			break;
		}
		
		case TOKEN_OPERATOR_OR:
		{
			result = generate_or_operation(codegen, left, right, area);

			break;
		}
		
		case TOKEN_OPERATOR_AND:
		{
			result = generate_and_operation(codegen, left, right, area);

			break;
		}
		
		case TOKEN_OPERATOR_PLUS:
		{
			result = generate_plus_operation(codegen, left, right, area);

			break;
		}

		case TOKEN_OPERATOR_MINUS:
		{
			result = generate_minus_operation(codegen, left, right, area);

			break;
		}

		case TOKEN_OPERATOR_INCREMENT:
		{
			result = generate_increment_operation(codegen, left, area);

			break;
		}
		
		case TOKEN_OPERATOR_DECREMENT:
		{
			result = generate_decrement_operation(codegen, left, area);

			break;
		}
		
		case TOKEN_OPERATOR_DIVIDED:
		{
			result = generate_div_operation(codegen, left, right, area);

			break;
		}
		
		case TOKEN_CHAR_STAR:
		{
			result = generate_multiply_operation(codegen, left, right, area);

			break;
		}
		
		case TOKEN_OPERATOR_PLUS_EQUALS:
		{
			result = generate_plus_equals_operation(codegen, left, right, area);

			break;
		}

		case TOKEN_OPERATOR_MINUS_EQUALS:
		{
			result = generate_minus_equals_operation(codegen, left, right, area);

			break;
		}

		case TOKEN_OPERATOR_TIMES_EQUALS:
		{
			result = generate_times_equals_operation(codegen, left, right, area);

			break;
		}

		case TOKEN_OPERATOR_DIVIDED_EQUALS:
		{
			result = generate_div_equals_operation(codegen, left, right, area);

			break;
		}

		default:
		{
			printf("Codegen debug fail #634...\n");
			exit(1);
		}
	}

	/* DAR UNBLOCK NOS REGISTERS DA BLACKLIST */

	return result;
}