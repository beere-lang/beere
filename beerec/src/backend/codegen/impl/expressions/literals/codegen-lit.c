#include <stdlib.h>

#include "codegen-lit.h"
#include "../../../../../frontend/structure/parser/parser.h"

static AsmReturn* generate_int_literal(CodeGen* codegen, Node* node, AsmArea* area, int force_reg, int prefer_second, int prefer_third, int argument_flag)
{
	LiteralNode* literal = &node->literal_node.literal;
	int value = literal->int_value;
	
	char buff[64];
	char* result = NULL;
	
	if (force_reg)
	{
		char* reg = prefer_second ? "ebx" : "eax";

		if (prefer_third)
		{
			reg = "edx";
		}

		snprintf(buff, 64, "	mov	%s, %d", reg, value);
		result = reg;

		add_line_to_area(area, buff);
	}
	else
	{
		snprintf(buff, 64, "%d", value);
		result = buff;
	}

	if (argument_flag && force_reg)
	{
		char* regs = prefer_second ? "rbx" : "rax";

		if (prefer_third)
		{
			regs = "rdx";
		}

		result = regs;
	}

	return create_asm_return(result, create_type(TYPE_INT, NULL));
}

static AsmReturn* generate_float_literal(CodeGen* codegen, Node* node, AsmArea* area, int prefer_second, int prefer_third)
{
	LiteralNode* literal = &node->literal_node.literal;
	int value = literal->int_value;
	
	Constant* constant = generate_constant(node);

	char* reg = prefer_second ? "xmm1" : "xmm0";

	if (prefer_third)
	{
		reg = "xmm3";
	}

	char buff[64];
	snprintf(buff, 64, "	movss	%s, dword [rel .LC%d]", reg, constant->id);

	add_line_to_area(area, buff);

	return create_asm_return(reg, create_type(TYPE_FLOAT, NULL));
}

static AsmReturn* generate_bool_literal(CodeGen* codegen, Node* node, AsmArea* area, int force_reg, int prefer_second, int prefer_third)
{
	LiteralNode* literal = &node->literal_node.literal;
	int value = literal->bool_value;
	
	char* reg = prefer_second ? "bl" : "al";

	if (prefer_third)
	{
		reg = "dl";
	}

	char buff[64];
	
	if (force_reg)
	{
		snprintf(buff, 64, "	mov	%s, %d", reg, value);
		add_line_to_area(area, buff);
	
		AsmReturn* ret = create_asm_return(reg, create_type(TYPE_BOOL, NULL));
		ret->is_reg = 1;

		return ret;
	}
	else
	{
		snprintf(buff, 64, "%d", value);
		
		return create_asm_return(buff, create_type(TYPE_BOOL, NULL));
	}
}

AsmReturn* generate_literal(CodeGen* codegen, Node* node, AsmArea* area, int force_reg, int prefer_second, int prefer_third, int argument_flag)
{
	LiteralNode* literal = &node->literal_node.literal;

	switch (literal->literal_type->type)
	{
		case TYPE_INT:
		{
			return generate_int_literal(codegen, node, area, force_reg, prefer_second, prefer_third, argument_flag);
		}
		
		case TYPE_FLOAT:
		{
			return generate_float_literal(codegen, node, area, prefer_second, prefer_third);
		}

		case TYPE_BOOL:
		{
			return generate_bool_literal(codegen, node, area, force_reg, prefer_second, prefer_third);
		}
		
		default:
		{
			printf("[Codegen Literal] Invalid node while generating literal...\n");
			exit(1);
		}
	}
}