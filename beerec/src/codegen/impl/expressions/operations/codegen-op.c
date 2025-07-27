#include <stdlib.h>

#include "codegen-op.h"
#include "../../../codegen.h"
#include "../arithmetic/codegen-arith.h"
#include "../codegen-expr.h"

AsmReturn* generate_operation(CodeGen* codegen, Node* node, AsmArea* area, int force_reg, int prefer_second, int argument_flag)
{
	OperationNode* op_node = &node->operation_node.operation;
	
	int right_force_arg = 1;
	
	if (node->operation_node.operation.right->type == NODE_LITERAL)
	{
		right_force_arg = 0;
	}
	
	AsmReturn* left = generate_expression(codegen, node->operation_node.operation.left, area, 1, 0, 0);
	AsmReturn* right = generate_expression(codegen, node->operation_node.operation.right, area, right_force_arg, 1, 0);
	
	switch (op_node->op)
	{
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
			return generate_increment_operation(codegen, left, right, area);
		}
		
		case TOKEN_OPERATOR_DECREMENT:
		{
			return generate_decrement_operation(codegen, left, right, area);
		}

		default:
		{
			printf("Codegen debug fail #634...\n");
			exit(1);
		}
	}
}