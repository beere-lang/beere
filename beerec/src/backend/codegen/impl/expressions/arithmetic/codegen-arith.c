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

AsmReturn* generate_increment_operation(CodeGen* codegen, AsmReturn* left_value, AsmReturn* right_value, AsmArea* area)
{
	return NULL;
}

AsmReturn* generate_decrement_operation(CodeGen* codegen, AsmReturn* left_value, AsmReturn* right_value, AsmArea* area)
{
	return NULL;
}