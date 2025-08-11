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

	snprintf(buff, 64, "	%s	%s, %s", get_add_opcode_for_type(left_value->type), left_value->value->reg->reg, right_value->value->reg->reg);
	add_line_to_area(area, buff);

	unuse_register(right_value->value->reg);

	return create_asm_return(NULL, left_value->value->reg, left_value->type, 1);
}

AsmReturn* generate_minus_operation(CodeGen* codegen, AsmReturn* left_value, AsmReturn* right_value, AsmArea* area, int argument_flag)
{
	char buff[64];

	snprintf(buff, 64, "	%s	%s, %s", get_sub_opcode_for_type(left_value->type), left_value->value->reg->reg, right_value->value->reg->reg);
	add_line_to_area(area, buff);

	unuse_register(right_value->value->reg);

	return create_asm_return(NULL, left_value->value->reg, left_value->type, 1);
}

AsmReturn* generate_decrement_operation(CodeGen* codegen, AsmReturn* left_value, AsmArea* area, int argument_flag)
{
	char buff[64];

	Register* reg = find_and_use_register(left_value->type, BITS_SIZE_32, find_owner_method(codegen->scope));

	snprintf(buff, 64, "	mov	%s, %s", reg->reg, left_value->value->reg->reg);
	add_line_to_area(area, buff);

	snprintf(buff, 64, "	dec	%s", reg->reg);
	add_line_to_area(area, buff);

	snprintf(buff, 64, "	mov	%s, %s", left_value->value->reg->reg, reg->reg);
	add_line_to_area(area, buff);

	unuse_register(left_value->value->reg);

	return create_asm_return(NULL, reg, left_value->type, 1);
}

AsmReturn* generate_increment_operation(CodeGen* codegen, AsmReturn* left_value, AsmArea* area, int argument_flag)
{
	char buff[64];

	Register* reg = find_and_use_register(left_value->type, BITS_SIZE_32, find_owner_method(codegen->scope));

	snprintf(buff, 64, "	mov	%s, %s", reg->reg, left_value->value->reg->reg);
	add_line_to_area(area, buff);

	snprintf(buff, 64, "	inc	%s", reg->reg);
	add_line_to_area(area, buff);

	snprintf(buff, 64, "	mov	%s, %s", left_value->value->reg->reg, reg->reg);
	add_line_to_area(area, buff);

	unuse_register(left_value->value->reg);

	return create_asm_return(NULL, reg, left_value->type, 1);
}

static AsmReturn* generate_floating_decrement_operation(CodeGen* codegen, AsmReturn* left_value, AsmArea* area)
{
	char buff[64];

	Register* reg = find_and_use_register(left_value->type, BITS_SIZE_32, find_owner_method(codegen->scope));

	snprintf(buff, 64, "	mov	%s, %s", reg->reg, left_value->value->reg->reg);
	add_line_to_area(area, buff);

	snprintf(buff, 64, "	dec	%s", reg->reg);
	add_line_to_area(area, buff);

	snprintf(buff, 64, "	mov	%s, %s", left_value->value->reg->reg, reg->reg);
	add_line_to_area(area, buff);

	unuse_register(left_value->value->reg);

	return create_asm_return(NULL, reg, left_value->type, 1);
}

static AsmReturn* generate_floating_increment_operation(CodeGen* codegen, AsmReturn* left_value, AsmArea* area)
{
	char buff[64];
	int is_double = left_value->type->type == TYPE_DOUBLE;

	Register* reg = find_and_use_register(left_value->type, is_double ? BITS_SIZE_64 : BITS_SIZE_32, find_owner_method(codegen->scope));
	
	char* opcode = (is_double) ? "movsd" : "movss";
	char* add_opcode = (is_double) ? "addsd" : "addss";

	snprintf(buff, 64, "	%s	%s, %s", opcode, reg->reg, left_value->value->reg->reg);
	add_line_to_area(area, buff);

	snprintf(buff, 64, "	%s	%s", add_opcode, reg->reg);
	add_line_to_area(area, buff);

	snprintf(buff, 64, "	%s	%s, %s", opcode, left_value->value->reg->reg, reg->reg);
	add_line_to_area(area, buff);

	unuse_register(left_value->value->reg);

	return create_asm_return(NULL, reg, left_value->type, 1);
}

static AsmReturn* generate_floating_multiply(CodeGen* codegen, AsmReturn* left_value, AsmReturn* right_value, AsmArea* area, int is_double)
{
	char buff[64];
	char* opcode = (is_double) ? "mulsd" : "mulss";

	snprintf(buff, 64, "	%s	%s, %s", opcode, left_value->value->reg->reg, right_value->value->reg->reg);
	add_line_to_area(area, buff);

	unuse_register(right_value->value->reg);

	return create_asm_return(NULL, left_value->value->reg, left_value->type, 1);
}

AsmReturn* generate_multiply_operation(CodeGen* codegen, AsmReturn* left_value, AsmReturn* right_value, AsmArea* area, int argument_flag)
{
	char buff[64];

	snprintf(buff, 64, "	imul	%s, %s", left_value->value->reg->reg, right_value->value->reg->reg);
	add_line_to_area(area, buff);

	unuse_register(right_value->value->reg);

	return create_asm_return(NULL, left_value->value->reg, left_value->type, 1);
}

static AsmReturn* generate_floating_div(CodeGen* codegen, AsmReturn* left_value, AsmReturn* right_value, AsmArea* area, int is_double)
{
	char buff[64];
	char* opcode = (is_double) ? "divsd" : "divss";

	snprintf(buff, 64, "	%s	%s, %s", opcode, left_value->value->reg->reg, right_value->value->reg->reg);
	add_line_to_area(area, buff);

	unuse_register(right_value->value->reg);

	return create_asm_return(NULL, left_value->value->reg, left_value->type, 1);
}

// used registers (eax, edx)
AsmReturn* generate_div_operation(CodeGen* codegen, AsmReturn* left_value, AsmReturn* right_value, AsmArea* area, int argument_flag)
{
	char buff[64];

	snprintf(buff, 64, "	mov	eax, %s", left_value->value->reg->reg);
	add_line_to_area(area, buff);

	add_line_to_area(area, "cdq");

	snprintf(buff, 64, "	idiv	%s", right_value->value->reg->reg);
	add_line_to_area(area, buff);

	unuse_register(right_value->value->reg);
	unuse_register(left_value->value->reg);

	Register* eax = find_register_by_name("eax", left_value->type);

	return create_asm_return(NULL, eax, left_value->type, 1);
}

AsmReturn* generate_plus_equals_operation(CodeGen* codegen, AsmReturn* left_value, AsmReturn* right_value, AsmArea* area, int argument_flag)
{
	char buff[64];
	const BitsSize SIZE = (left_value->type->type == TYPE_DOUBLE) ? BITS_SIZE_64 : BITS_SIZE_32;

	Register* reg = find_and_use_register(left_value->type, SIZE, find_owner_method(codegen->scope));

	snprintf(buff, 64, "	%s	%s, %s", get_mov_opcode_for_type(left_value->type), reg->reg, left_value->value->reg->reg);
	add_line_to_area(area, buff);

	snprintf(buff, 64, "	%s	%s, %s", get_add_opcode_for_type(left_value->type), reg->reg, left_value->value->reg->reg);
	add_line_to_area(area, buff);

	snprintf(buff, 64, "	%s	%s, %s", get_mov_opcode_for_type(left_value->type), left_value->value->reg->reg, reg->reg);
	add_line_to_area(area, buff);

	unuse_register(left_value->value->reg);
	unuse_register(right_value->value->reg);
	unuse_register(reg);

	return create_asm_return(NULL, reg, left_value->type, 1);
}

AsmReturn* generate_minus_equals_operation(CodeGen* codegen, AsmReturn* left_value, AsmReturn* right_value, AsmArea* area, int argument_flag)
{
	char buff[64];
	const BitsSize SIZE = (left_value->type->type == TYPE_DOUBLE) ? BITS_SIZE_64 : BITS_SIZE_32;

	Register* reg = find_and_use_register(left_value->type, SIZE, find_owner_method(codegen->scope));

	snprintf(buff, 64, "	%s	%s, %s", get_mov_opcode_for_type(left_value->type), reg->reg, left_value->value->reg->reg);
	add_line_to_area(area, buff);

	snprintf(buff, 64, "	%s	%s, %s", get_sub_opcode_for_type(left_value->type), reg->reg, left_value->value->reg->reg);
	add_line_to_area(area, buff);

	snprintf(buff, 64, "	%s	%s, %s", get_mov_opcode_for_type(left_value->type), left_value->value->reg->reg, reg->reg);
	add_line_to_area(area, buff);

	unuse_register(left_value->value->reg);
	unuse_register(right_value->value->reg);
	unuse_register(reg);

	return create_asm_return(NULL, reg, left_value->type, 1);
}

AsmReturn* generate_times_equals_operation(CodeGen* codegen, AsmReturn* left_value, AsmReturn* right_value, AsmArea* area, int argument_flag)
{
	char buff[64];
	const BitsSize SIZE = (left_value->type->type == TYPE_DOUBLE) ? BITS_SIZE_64 : BITS_SIZE_32;
	
	char* opcode = "imul";

	if (left_value->type->type == TYPE_DOUBLE || left_value->type->type == TYPE_FLOAT)
	{
		opcode = (left_value->type->type == TYPE_DOUBLE) ? "mulsd" : "mulss";
	}

	Register* reg = find_and_use_register(left_value->type, SIZE, find_owner_method(codegen->scope));

	snprintf(buff, 64, "	%s	%s, %s", get_mov_opcode_for_type(left_value->type), reg->reg, left_value->value->reg->reg);
	add_line_to_area(area, buff);

	snprintf(buff, 64, "	%s	%s, %s", opcode, reg->reg, left_value->value->reg->reg);
	add_line_to_area(area, buff);

	snprintf(buff, 64, "	%s	%s, %s", get_mov_opcode_for_type(left_value->type), left_value->value->reg->reg, reg->reg);
	add_line_to_area(area, buff);

	unuse_register(left_value->value->reg);
	unuse_register(right_value->value->reg);
	unuse_register(reg);

	return create_asm_return(NULL, reg, left_value->type, 1);
}

static AsmReturn* generate_floating_div_equals(CodeGen* codegen, AsmReturn* left_value, AsmReturn* right_value, AsmArea* area, int is_double)
{
	char buff[64];

	const BitsSize size = (is_double) ? BITS_SIZE_64 : BITS_SIZE_32;
	Register* reg = find_and_use_register(left_value->type, size, find_owner_method(codegen->scope));

	char* opcode = (is_double) ? "divsd" : "divss";
	
	snprintf(buff, 64, "	%s	%s, %s", get_mov_opcode_for_type(left_value->type), reg->reg, left_value->value->reg->reg);
	add_line_to_area(area, buff);

	snprintf(buff, 64, "	%s	%s, %s", opcode, reg->reg, right_value->value->reg->reg);
	add_line_to_area(area, buff);

	snprintf(buff, 64, "	%s	%s, %s", get_mov_opcode_for_type(left_value->type), left_value->value->reg->reg, reg->reg);
	add_line_to_area(area, buff);

	unuse_register(left_value->value->reg);
	unuse_register(right_value->value->reg);
	unuse_register(reg);

	return create_asm_return(NULL, reg, left_value->type, 1);
}

// used registers (eax, edx)
AsmReturn* generate_div_equals_operation(CodeGen* codegen, AsmReturn* left_value, AsmReturn* right_value, AsmArea* area, int argument_flag)
{
	if (left_value->type->type == TYPE_FLOAT || right_value->type->type == TYPE_DOUBLE)
	{
		return generate_floating_div_equals(codegen, left_value, right_value, area, (left_value->type->type == TYPE_DOUBLE));
	}

	char buff[64];
	const BitsSize SIZE = BITS_SIZE_32;
	
	char* opcode = "idiv";

	Register* reg = find_and_use_register(left_value->type, SIZE, find_owner_method(codegen->scope));

	snprintf(buff, 64, "	%s	eax, %s", get_mov_opcode_for_type(left_value->type), left_value->value->reg->reg);
	add_line_to_area(area, buff);

	add_line_to_area(area, "cdq");

	snprintf(buff, 64, "	%s	%s", opcode, reg->reg);
	add_line_to_area(area, buff);

	snprintf(buff, 64, "	%s	%s, eax", get_mov_opcode_for_type(left_value->type), left_value->value->reg->reg);
	add_line_to_area(area, buff);

	unuse_register(left_value->value->reg);
	unuse_register(right_value->value->reg);
	unuse_register(reg);

	return create_asm_return(NULL, reg, left_value->type, 1);
}