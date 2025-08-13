#include <stdlib.h>

#include "codegen-lit.h"
#include "../../../../../frontend/structure/parser/parser.h"

static AsmReturn* generate_int_literal(CodeGen* codegen, Node* node, AsmArea* area, int force_reg)
{
	LiteralNode* literal = &node->literal;
	int value = literal->int_value;
	
	char buff[64];
	Type* type = create_type(TYPE_INT, NULL);
	
	if (force_reg)
	{
		Type* type = create_type(TYPE_INT, NULL);
		Register* reg = find_and_use_register(type, BITS_SIZE_32, find_owner_method(codegen->scope));

		snprintf(buff, 64, "	mov	%s, %d", reg->reg, value);
		add_line_to_area(area, buff);

		return create_asm_return(NULL, reg, type, 1);
	}
	else
	{
		SegmentNode* segment = generate_segment_literal(SEGMENT_LITERAL_INT, value);

		return create_asm_return(segment, NULL, type, 0);
	}
}

static AsmReturn* generate_float_literal(CodeGen* codegen, Node* node, AsmArea* area)
{
	LiteralNode* literal = &node->literal;
	int value = literal->int_value;
	
	Type* type = create_type(TYPE_FLOAT, NULL);
	
	Constant* constant = generate_constant(node);
	Register* reg = find_and_use_register(type, BITS_SIZE_32, find_owner_method(codegen->scope));

	char buff[64];
	snprintf(buff, 64, "	movss	%s, dword [rel .LC%d]", reg->reg, constant->id);

	add_line_to_area(area, buff);

	return create_asm_return(NULL, reg, type, 1);
}

static AsmReturn* generate_bool_literal(CodeGen* codegen, Node* node, AsmArea* area, int force_reg)
{
	LiteralNode* literal = &node->literal;
	int value = literal->bool_value;
	
	Type* type = create_type(TYPE_BOOL, NULL);
	Register* reg = find_and_use_register(type, BITS_SIZE_8, find_owner_method(codegen->scope));

	char buff[64];
	
	if (force_reg)
	{
		snprintf(buff, 64, "	mov	%s, %d", reg->reg, value);
		add_line_to_area(area, buff);
	
		return create_asm_return(NULL, reg, type, 1);
	}
	else
	{
		SegmentNode* segment = generate_segment_literal(SEGMENT_LITERAL_INT, value);
		
		return create_asm_return(segment, NULL, type, 0);
	}
}

AsmReturn* generate_literal(CodeGen* codegen, Node* node, AsmArea* area, int force_reg)
{
	LiteralNode* literal = &node->literal;

	switch (literal->literal_type->type)
	{
		case TYPE_INT:
		{
			return generate_int_literal(codegen, node, area, force_reg);
		}
		
		case TYPE_FLOAT:
		{
			return generate_float_literal(codegen, node, area);
		}

		case TYPE_BOOL:
		{
			return generate_bool_literal(codegen, node, area, force_reg);
		}
		
		default:
		{
			printf("[Codegen Literal] Invalid node while generating literal...\n");
			exit(1);
		}
	}
}