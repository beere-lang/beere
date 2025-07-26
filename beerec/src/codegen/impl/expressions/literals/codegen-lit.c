#include <stdlib.h>

#include "codegen-lit.h"
#include "../../../../parser/parser.h"

static AsmReturn* generate_int_literal(CodeGen* codegen, Node* node, AsmArea* area, int force_reg, int prefer_second)
{
	LiteralNode* literal = &node->literal_node.literal;
	int value = literal->int_value;
	
	char buff[64];
	char* result = NULL;
	
	if (force_reg)
	{
		char* reg = prefer_second ? "ebx" : "eax";

		snprintf(buff, 64, "	mov	%s, %d", reg, value);
		result = reg;

		add_line_to_area(area, buff);
	}
	else
	{
		snprintf(buff, 64, "%d", value);
		result = buff;
	}

	return create_asm_return(result, create_type(TYPE_INT, NULL));
}

static AsmReturn* generate_float_literal(CodeGen* codegen, Node* node, AsmArea* area, int force_reg, int prefer_second)
{
	LiteralNode* literal = &node->literal_node.literal;
	int value = literal->int_value;
	
	Constant* constant = generate_constant(node);

	char* reg = prefer_second ? "xmm1" : "xmm0";

	char buff[64];
	snprintf(buff, 64, "	movss	%s, dword [rip+.LC%d]", reg, constant->id);

	add_line_to_area(area, buff);

	return create_asm_return(reg, create_type(TYPE_FLOAT, NULL));
}

AsmReturn* generate_literal(CodeGen* codegen, Node* node, AsmArea* area, int force_reg, int prefer_second, int argument_flag)
{
	LiteralNode* literal = &node->literal_node.literal;

	switch (literal->literal_type->type)
	{
		case TYPE_INT:
		{
			return generate_int_literal(codegen, node, area, force_reg, prefer_second);
		}
		
		case TYPE_FLOAT:
		{
			return generate_float_literal(codegen, node, area, force_reg, prefer_second);
		}
		
		default:
		{
			printf("[Codegen Literal] Invalid node while generating literal...\n");
			exit(1);
		}
	}
}