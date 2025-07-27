#include <stdlib.h>

#include "codegen-expr.h"
#include "cast/codegen-cast.h"
#include "literals/codegen-lit.h"
#include "operations/codegen-op.h"
#include "pointers/codegen-ptr.h"
#include "references/codegen-ref.h"

AsmReturn* generate_expression(CodeGen* codegen, Node* node, AsmArea* area, int force_reg, int prefer_second, int argument_flag)
{
	switch (node->type)
	{
		case NODE_OPERATION:
		{
			return generate_operation(codegen, node, area, force_reg, prefer_second, argument_flag);
		}

		case NODE_LITERAL:
		{
			return generate_literal(codegen, node, area, force_reg, prefer_second, argument_flag);
		}

		case NODE_IDENTIFIER:
		{
			return generate_variable_reference(codegen, node, area, force_reg, prefer_second, argument_flag);
		}

		case NODE_CAST:
		{
			return generate_cast(codegen, node, area, prefer_second, argument_flag);
		}

		case NODE_DEREFERENCE:
		{
			return generate_dereference(codegen, node, area, force_reg, prefer_second);
		}

		case NODE_ADRESS_OF:
		{
			return generate_reference(codegen, node, area, force_reg, prefer_second);
		}

		default:
		{
			printf("[Codegen Literal] Invalid node while generating expression...\n");
			exit(1);
		}
	}
}