#include <string.h>
#include <stdlib.h>

#include "codegen-arith.h"

static char* get_add_opcode_for_type(Type* type)
{
	switch (type->type)
	{
		case TYPE_INT:
		{
			return strdup("add");
		}

		case TYPE_FLOAT:
		{
			return strdup("addss");
		}

		case TYPE_DOUBLE:
		{
			return strdup("addsd");
		}

		default:
		{
			printf("Codegen debug fail #0296...\n");
			exit(1);
		}
	}
}

static char* get_sub_opcode_for_type(Type* type)
{
	switch (type->type)
	{
		case TYPE_INT:
		{
			return strdup("sub");
		}

		case TYPE_FLOAT:
		{
			return strdup("subss");
		}

		case TYPE_DOUBLE:
		{
			return strdup("subsd");
		}

		default:
		{
			printf("Codegen debug fail #384...\n");
			exit(1);
		}
	}
}

static char* get_mov_opcode_for_type(Type* type)
{
	switch (type->type)
	{
		case TYPE_INT:
		{
			return strdup("mov");
		}

		case TYPE_FLOAT:
		{
			return strdup("movss");
		}

		case TYPE_DOUBLE:
		{
			return strdup("movsd");
		}

		default:
		{
			printf("Codegen debug fail #1209...\n");
			exit(1);
		}
	}
}

AsmReturn* generate_plus_operation(CodeGen* codegen, AsmReturn* left_value, AsmReturn* right_value, AsmArea* area, int argument_flag)
{
	char buff[64];
	snprintf(buff, 64, "	%s	%s, %s", get_add_opcode_for_type(left_value->type), left_value->result, right_value->result);

	add_line_to_area(area, buff);

	if (argument_flag && left_value->type->type != TYPE_DOUBLE && left_value->type->type != TYPE_FLOAT)
	{
		snprintf(buff, 64, "	mov	rax, %s", left_value->result);
		add_line_to_area(area, buff);

		return create_asm_return("rax", left_value->type);
	}
	
	return create_asm_return(left_value->result, left_value->type);
}

AsmReturn* generate_minus_operation(CodeGen* codegen, AsmReturn* left_value, AsmReturn* right_value, AsmArea* area, int argument_flag)
{
	char buff[64];
	snprintf(buff, 64, "	%s	%s, %s", get_sub_opcode_for_type(left_value->type), left_value->result, right_value->result);

	add_line_to_area(area, buff);

	if (argument_flag && left_value->type->type != TYPE_DOUBLE && left_value->type->type != TYPE_FLOAT)
	{
		snprintf(buff, 64, "	mov	rax, %s", left_value->result);
		add_line_to_area(area, buff);

		return create_asm_return("rax", left_value->type);
	}
	
	return create_asm_return(left_value->result, left_value->type);
}

static AsmReturn* generate_floating_increment_operation(CodeGen* codegen, AsmReturn* left_value, AsmArea* area)
{
	char buff[64];

	int is_double = (left_value->type->type == TYPE_DOUBLE);
	char* opcode = (is_double) ? "movsd" : "movss";
	char* _opcode = (is_double) ? "addsd" : "addss";

	snprintf(buff, 64, "	%s	xmm0, %s", opcode, left_value->result);
	add_line_to_area(area, buff);

	Constant* constant = generate_directly_constant(1, is_double);

	snprintf(buff, 64, "	%s	xmm1, [rel .LC%d]", opcode, constant->id);
	add_line_to_area(area, buff);
	
	snprintf(buff, 64, "	%s	%s, xmm1", _opcode, left_value->result);
	add_line_to_area(area, buff);
	
	return create_asm_return("xmm0", left_value->type);
}

AsmReturn* generate_increment_operation(CodeGen* codegen, AsmReturn* left_value, AsmArea* area)
{
	if (left_value->type->type == TYPE_FLOAT || left_value->type->type == TYPE_DOUBLE)
	{
		return generate_floating_increment_operation(codegen, left_value, area);
	}
	
	char buff[64];

	snprintf(buff, 64, "	mov	eax, %s", left_value->result);
	add_line_to_area(area, buff);
	
	snprintf(buff, 64, "	inc	%s", left_value->result);
	add_line_to_area(area, buff);
	
	return create_asm_return("eax", left_value->type);
}

static AsmReturn* generate_floating_decrement_operation(CodeGen* codegen, AsmReturn* left_value, AsmArea* area)
{
	char buff[64];

	int is_double = (left_value->type->type == TYPE_DOUBLE);
	char* opcode = (is_double) ? "movsd" : "movss";
	char* _opcode = (is_double) ? "subsd" : "subss";

	snprintf(buff, 64, "	%s	xmm0, %s", opcode, left_value->result);
	add_line_to_area(area, buff);

	Constant* constant = generate_directly_constant(1, is_double);

	snprintf(buff, 64, "	%s	xmm1, [rel .LC%d]", opcode, constant->id);
	add_line_to_area(area, buff);
	
	snprintf(buff, 64, "	%s	%s, xmm1", _opcode, left_value->result);
	add_line_to_area(area, buff);
	
	return create_asm_return("xmm0", left_value->type);
}

AsmReturn* generate_decrement_operation(CodeGen* codegen, AsmReturn* left_value, AsmArea* area)
{
	if (left_value->type->type == TYPE_FLOAT || left_value->type->type == TYPE_DOUBLE)
	{
		return generate_floating_decrement_operation(codegen, left_value, area);
	}

	char buff[64];

	snprintf(buff, 64, "	mov	eax, %s", left_value->result);
	add_line_to_area(area, buff);

	snprintf(buff, 64, "	dec	%s", left_value->result);
	add_line_to_area(area, buff);

	return create_asm_return("eax", left_value->type);
}

static AsmReturn* generate_floating_multiply(CodeGen* codegen, AsmReturn* lreg, AsmReturn* rreg, AsmArea* area, int is_double)
{
	char buff[64];

	char* opcode = (is_double) ? "mulsd" : "mulss";

	snprintf(buff, 64, "	%s	%s, %s", opcode, lreg->result, rreg->result);
	add_line_to_area(area, buff);
	
	return create_asm_return(lreg->result, lreg->type);
}

AsmReturn* generate_multiply_operation(CodeGen* codegen, AsmReturn* lreg, AsmReturn* rreg, AsmArea* area)
{
	if (lreg->type->type == TYPE_FLOAT || lreg->type->type == TYPE_DOUBLE)
	{
		generate_floating_multiply(codegen, lreg, rreg, area, (lreg->type->type == TYPE_DOUBLE));
	}

	char buff[64];

	snprintf(buff, 64, "	imul	%s, %s", lreg->result, rreg->result);
	add_line_to_area(area, buff);
	
	return create_asm_return(lreg->result, lreg->type);
}

static AsmReturn* generate_floating_div(CodeGen* codegen, AsmReturn* lreg, AsmReturn* rreg, AsmArea* area, int is_double)
{
	char buff[64];

	char* opcode = (is_double) ? "divsd" : "divss";

	snprintf(buff, 64, "	%s	%s, %s", opcode, lreg->result, rreg->result);
	add_line_to_area(area, buff);
	
	return create_asm_return(lreg->result, lreg->type);
}

AsmReturn* generate_div_operation(CodeGen* codegen, AsmReturn* lreg, AsmReturn* rreg, AsmArea* area)
{
	if (lreg->type->type == TYPE_FLOAT || lreg->type->type == TYPE_DOUBLE)
	{
		generate_floating_div(codegen, lreg, rreg, area, (lreg->type->type == TYPE_DOUBLE));
	}

	char buff[64];

	add_line_to_area(area, "	cdq");

	snprintf(buff, 64, "	mov	ebx, %s", rreg->result);
	add_line_to_area(area, buff);
	
	add_line_to_area(area, "	idiv	ebx");
	
	return create_asm_return("eax", lreg->type);
}

AsmReturn* generate_plus_equals_operation(CodeGen* codegen, AsmReturn* left_value, AsmReturn* right_value, AsmArea* area, int argument_flag)
{
	char buff[64];
	int is_floating = left_value->type->type == TYPE_FLOAT || left_value->type->type == TYPE_DOUBLE;
	char* reg = (is_floating) ? "xmm0" : "eax";
	
	snprintf(buff, 64, "	%s	%s, %s", get_mov_opcode_for_type(left_value->type), "eax", left_value->result);
	add_line_to_area(area, buff);

	snprintf(buff, 64, "	%s	%s, %s", get_add_opcode_for_type(left_value->type), "eax", right_value->result);
	add_line_to_area(area, buff);

	snprintf(buff, 64, "	%s	%s, %s", get_mov_opcode_for_type(left_value->type), left_value->result, "eax");
	add_line_to_area(area, buff);

	return create_asm_return((argument_flag && !is_floating) ? "rax" : reg, left_value->type);
}

AsmReturn* generate_minus_equals_operation(CodeGen* codegen, AsmReturn* left_value, AsmReturn* right_value, AsmArea* area, int argument_flag)
{
	char buff[64];
	int is_floating = left_value->type->type == TYPE_FLOAT || left_value->type->type == TYPE_DOUBLE;
	char* reg = (is_floating) ? "xmm0" : "eax";
	
	snprintf(buff, 64, "	%s	%s, %s", get_mov_opcode_for_type(left_value->type), "eax", left_value->result);
	add_line_to_area(area, buff);

	snprintf(buff, 64, "	%s	%s, %s", get_sub_opcode_for_type(left_value->type), "eax", right_value->result);
	add_line_to_area(area, buff);

	snprintf(buff, 64, "	%s	%s, %s", get_mov_opcode_for_type(left_value->type), left_value->result, "eax");
	add_line_to_area(area, buff);

	return create_asm_return((argument_flag && !is_floating) ? "rax" : reg, left_value->type);
}