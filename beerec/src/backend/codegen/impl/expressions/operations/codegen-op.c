#include <stdlib.h>

#include "codegen-op.h"
#include "../../../codegen.h"
#include "../arithmetic/codegen-arith.h"
#include "../comparative/codegen-comparative.h"
#include "../codegen-expr.h"

AsmReturn* generate_operation(CodeGen* codegen, Node* node, AsmArea* area, int force_reg, int prefer_second, int prefer_third, int argument_flag)
{
	OperationNode* op_node = &node->operation_node.operation;
	
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
	
	AsmReturn* left = generate_expression(codegen, node->operation_node.operation.left, area, left_force_reg, 0, left_prefer_third, 0);
	AsmReturn* right = NULL;
	
	if (op_node->op != TOKEN_OPERATOR_INCREMENT && op_node->op != TOKEN_OPERATOR_DECREMENT)
	{
		right = generate_expression(codegen, node->operation_node.operation.right, area, right_force_reg, 1, 0, 0);
	}

	switch (op_node->op)
	{
		case TOKEN_OPERATOR_EQUALS:
		{
			return generate_is_equals_operation(codegen, left, right, area, prefer_second, prefer_third, argument_flag);
		}
		
		case TOKEN_OPERATOR_NOT_EQUALS:
		{
			return generate_is_not_equals_operation(codegen, left, right, area, prefer_second, prefer_third, argument_flag);
		}
		
		case TOKEN_OPERATOR_GREATER:
		{
			return generate_is_greater_operation(codegen, left, right, area, prefer_second, prefer_third, argument_flag);
		}
		
		case TOKEN_OPERATOR_LESS:
		{
			return generate_is_less_operation(codegen, left, right, area, prefer_second, prefer_third, argument_flag);
		}
		
		case TOKEN_OPERATOR_GREATER_EQUALS:
		{
			return generate_is_greater_equals_operation(codegen, left, right, area, prefer_second, prefer_third, argument_flag);
		}
		
		case TOKEN_OPERATOR_LESS_EQUALS:
		{
			return generate_is_less_equals_operation(codegen, left, right, area, prefer_second, prefer_third, argument_flag);
		}
		
		case TOKEN_OPERATOR_OR:
		{
			return generate_or_operation(codegen, left, right, area, prefer_second, prefer_third, argument_flag);
		}
		
		case TOKEN_OPERATOR_AND:
		{
			return generate_and_operation(codegen, left, right, area, prefer_second, prefer_third, argument_flag);
		}
		
		case TOKEN_OPERATOR_PLUS:
		{
			return generate_plus_operation(codegen, left, right, area, argument_flag);
		}

		case TOKEN_OPERATOR_MINUS:
		{
			return generate_minus_operation(codegen, left, right, area, argument_flag);
		}

		case TOKEN_OPERATOR_INCREMENT:
		{
			return generate_increment_operation(codegen, left, area, argument_flag);
		}
		
		case TOKEN_OPERATOR_DECREMENT:
		{
			return generate_decrement_operation(codegen, left, area, argument_flag);
		}
		
		case TOKEN_OPERATOR_DIVIDED:
		{
			return generate_div_operation(codegen, left, right, area, argument_flag);
		}
		
		case TOKEN_CHAR_STAR:
		{
			return generate_multiply_operation(codegen, left, right, area, argument_flag);
		}
		
		case TOKEN_OPERATOR_PLUS_EQUALS:
		{
			return generate_plus_equals_operation(codegen, left, right, area, argument_flag);
		}

		case TOKEN_OPERATOR_MINUS_EQUALS:
		{
			return generate_minus_equals_operation(codegen, left, right, area, argument_flag);
		}

		case TOKEN_OPERATOR_TIMES_EQUALS:
		{
			return generate_times_equals_operation(codegen, left, right, area, argument_flag);
		}

		case TOKEN_OPERATOR_DIVIDED_EQUALS:
		{
			return generate_div_equals_operation(codegen, left, right, area, argument_flag);
		}

		default:
		{
			printf("Codegen debug fail #634...\n");
			exit(1);
		}
	}
}