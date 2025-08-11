#include <stdlib.h>

#include "codegen-expr.h"
#include "cast/codegen-cast.h"
#include "literals/codegen-lit.h"
#include "operations/codegen-op.h"
#include "pointers/codegen-ptr.h"
#include "references/codegen-ref.h"
#include "../oop/member-access/codegen-mbr-access.h"
#include "../oop/instances/codegen-create-instance.h"
#include "../statements/calls/codegen-method-call.h"

AsmReturn* generate_expression(CodeGen* codegen, Node* node, AsmArea* area, int force_reg)
{
	switch (node->type)
	{
		case NODE_OPERATION:
		{
			return generate_operation(codegen, node, area, force_reg);
		}

		case NODE_LITERAL:
		{
			return generate_literal(codegen, node, area, force_reg);
		}

		case NODE_IDENTIFIER:
		{
			return generate_variable_reference(codegen, node, area, force_reg);
		}

		case NODE_CAST:
		{
			return generate_cast(codegen, node, area);
		}

		case NODE_DEREFERENCE:
		{
			return generate_dereference(codegen, node, area, force_reg);
		}

		case NODE_ADRESS_OF:
		{
			return generate_reference(codegen, node, area, force_reg);
		}

		case NODE_MEMBER_ACCESS:
		{
			return generate_member_access(codegen, node, area, force_reg);
		}

		case NODE_FUNCTION_CALL:
		{
			return generate_method_call(codegen, node, area);
		}

		case NODE_CREATE_INSTANCE:
		{
			return generate_create_class_instance(codegen, node, area);
		}

		case NODE_SUPER:
		{
			return generate_super(codegen, area, force_reg);
		}

		case NODE_THIS:
		{
			return generate_this(codegen, area, force_reg);
		}

		case NODE_DIRECT_CLASS:
		{
			return generate_class_direct_access(codegen, node);
		}

		default:
		{
			printf("[Codegen Literal] Invalid node while generating expression: %d...\n", node->type);
			exit(1);
		}
	}
}