#include <stdlib.h>

#include "codegen-op.h"
#include "../../../codegen.h"
#include "../codegen-expr.h"

AsmReturn* generate_operation(CodeGen* codegen, Node* node, AsmArea* area, Flag flag)
{
	OperationNode* op_node = &node->operation_node.operation;
	
	AsmReturn* left = generate_expr(codegen, node, area, flag);
	
	codegen->prefer_second = 1;
	AsmReturn* right = generate_expr(codegen, node, area, flag);
	codegen->prefer_second = 0;

	switch (op_node->op)
	{
		
		
		default:
		{
			exit(1);
		}
	}
}