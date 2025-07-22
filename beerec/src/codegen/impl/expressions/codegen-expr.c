#include <stdlib.h>

#include "codegen-expr.h"
#include "operations/codegen-op.h"

static AsmReturn* generate_expression(CodeGen* codegen, Node* node, AsmArea* area, Flag flag)
{
	switch (node->type)
	{
		case NODE_OPERATION:
		{
			generate_operation(codegen, node, area, flag);
		}

		default:
		{
			printf("[Codegen Literal] Invalid node while generating expression...\n");
			exit(1);
		}
	}
}

AsmReturn* generate_expr(CodeGen* codegen, Node* node, AsmArea* area, Flag flag)
{
	return generate_expression(codegen, node, area, flag);
}