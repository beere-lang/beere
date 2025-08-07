#include <string.h>
#include <stdlib.h>

#include "codegen-comparative.h"
#include "../../../../../frontend/structure/parser/parser.h"

char* get_opcode_for_cmp(Type* type)
{
	switch (type->type)
	{
		case TYPE_INT:
		{
			return strdup("cmp");
		}

		case TYPE_FLOAT:
		{
			return strdup("ucomiss");
		}

		case TYPE_DOUBLE:
		{
			return strdup("ucomisd");
		}

		case TYPE_BOOL:
		{
			return strdup("cmp");
		}
	
		default:
		{
			exit(1);
		}
	}
}

AsmReturn* generate_is_equals_operation(CodeGen* codegen, AsmReturn* lreg, AsmReturn* rreg, AsmArea* area, int prefer_second, int argument_flag)
{
	char* temp = prefer_second ? "bl" : "al";
	char* _temp = prefer_second ? "rbx" : "rax";

	char buff[64];
	
	char* opcode = get_opcode_for_cmp(lreg->type);
	
	snprintf(buff, 64, "	%s	%s, %s", opcode, lreg->result, rreg->result);
	add_line_to_area(area, buff);

	// seta o register pra 1 caso igual, caso contrario seta pra 0
	snprintf(buff, 64, "	sete	%s", temp);
	add_line_to_area(area, buff);

	// zera os bits mais altos do register
	snprintf(buff, 64, "	movzx	%s, %s", _temp, temp);
	add_line_to_area(area, buff);

	AsmReturn* ret = create_asm_return(argument_flag ? _temp : temp, create_type(TYPE_BOOL, NULL));

	return ret;
}

AsmReturn* generate_is_not_equals_operation(CodeGen* codegen, AsmReturn* lreg, AsmReturn* rreg, AsmArea* area, int prefer_second, int argument_flag)
{
	char* temp = prefer_second ? "bl" : "al";
	char* _temp = prefer_second ? "rbx" : "rax";

	char buff[64];
	
	char* opcode = get_opcode_for_cmp(lreg->type);
	
	snprintf(buff, 64, "	%s	%s, %s", opcode, lreg->result, rreg->result);
	add_line_to_area(area, buff);

	// seta o register pra 1 caso diferente, caso contrario seta pra 0
	snprintf(buff, 64, "	setne	%s", temp);
	add_line_to_area(area, buff);

	// zera os bits mais altos do register
	snprintf(buff, 64, "	movzx	%s, %s", _temp, temp);
	add_line_to_area(area, buff);

	AsmReturn* ret = create_asm_return(argument_flag ? _temp : temp, create_type(TYPE_BOOL, NULL));

	return ret;
}

AsmReturn* generate_is_greater_operation(CodeGen* codegen, AsmReturn* lreg, AsmReturn* rreg, AsmArea* area, int prefer_second, int argument_flag)
{
	char* temp = prefer_second ? "bl" : "al";
	char* _temp = prefer_second ? "rbx" : "rax";

	char buff[64];

	char* opcode = get_opcode_for_cmp(lreg->type);
	
	snprintf(buff, 64, "	%s	%s, %s", opcode, lreg->result, rreg->result);
	add_line_to_area(area, buff);

	// seta o register pra 1 caso maior, caso contrario seta pra 0
	snprintf(buff, 64, "	setg	%s", temp);
	add_line_to_area(area, buff);

	// zera os bits mais altos do register
	snprintf(buff, 64, "	movzx	%s, %s", _temp, temp);
	add_line_to_area(area, buff);

	AsmReturn* ret = create_asm_return(argument_flag ? _temp : temp, create_type(TYPE_BOOL, NULL));

	return ret;
}

AsmReturn* generate_is_less_operation(CodeGen* codegen, AsmReturn* lreg, AsmReturn* rreg, AsmArea* area, int prefer_second, int argument_flag)
{
	char* temp = prefer_second ? "bl" : "al";
	char* _temp = prefer_second ? "rbx" : "rax";

	char buff[64];
	
	char* opcode = get_opcode_for_cmp(lreg->type);
	
	snprintf(buff, 64, "	%s	%s, %s", opcode, lreg->result, rreg->result);
	add_line_to_area(area, buff);

	// seta o register pra 1 caso menor, caso contrario seta pra 0
	snprintf(buff, 64, "	setl	%s", temp);
	add_line_to_area(area, buff);

	// zera os bits mais altos do register
	snprintf(buff, 64, "	movzx	%s, %s", _temp, temp);
	add_line_to_area(area, buff);

	AsmReturn* ret = create_asm_return(argument_flag ? _temp : temp, create_type(TYPE_BOOL, NULL));

	return ret;
}

AsmReturn* generate_is_greater_equals_operation(CodeGen* codegen, AsmReturn* lreg, AsmReturn* rreg, AsmArea* area, int prefer_second, int argument_flag)
{
	char* temp = prefer_second ? "bl" : "al";
	char* _temp = prefer_second ? "rbx" : "rax";

	char buff[64];
	
	char* opcode = get_opcode_for_cmp(lreg->type);
	
	snprintf(buff, 64, "	%s	%s, %s", opcode, lreg->result, rreg->result);
	add_line_to_area(area, buff);

	// seta o register pra 1 caso maior ou igual, caso contrario seta pra 0
	snprintf(buff, 64, "	setge	%s", temp);
	add_line_to_area(area, buff);

	// zera os bits mais altos do register
	snprintf(buff, 64, "	movzx	%s, %s", _temp, temp);
	add_line_to_area(area, buff);

	AsmReturn* ret = create_asm_return(argument_flag ? _temp : temp, create_type(TYPE_BOOL, NULL));

	return ret;
}

AsmReturn* generate_is_less_equals_operation(CodeGen* codegen, AsmReturn* lreg, AsmReturn* rreg, AsmArea* area, int prefer_second, int argument_flag)
{
	char* temp = prefer_second ? "bl" : "al";
	char* _temp = prefer_second ? "rbx" : "rax";

	char buff[64];
	
	char* opcode = get_opcode_for_cmp(lreg->type);
	
	snprintf(buff, 64, "	%s	%s, %s", opcode, lreg->result, rreg->result);
	add_line_to_area(area, buff);

	// seta o register pra 1 caso menor ou igual, caso contrario seta pra 0
	snprintf(buff, 64, "	setle	%s", temp);
	add_line_to_area(area, buff);

	// zera os bits mais altos do register
	snprintf(buff, 64, "	movzx	%s, %s", _temp, temp);
	add_line_to_area(area, buff);

	AsmReturn* ret = create_asm_return(argument_flag ? _temp : temp, create_type(TYPE_BOOL, NULL));

	return ret;
}

AsmReturn* generate_or_operation(CodeGen* codegen, AsmReturn* lreg, AsmReturn* rreg, AsmArea* area, int prefer_second, int argument_flag)
{
	char* _temp = prefer_second ? "rbx" : "rax";

	char buff[64];
	
	// seta o register pra 1 caso um ou outro for 1, caso constrario seta pra 0
	snprintf(buff, 64, "	or	%s, %s", lreg->result, rreg->result);
	add_line_to_area(area, buff);

	snprintf(buff, 64, "	movzx	%s, %s", _temp, lreg->result);
	add_line_to_area(area, buff);

	AsmReturn* ret = create_asm_return(lreg->result, create_type(TYPE_BOOL, NULL));

	return ret;
}

AsmReturn* generate_and_operation(CodeGen* codegen, AsmReturn* lreg, AsmReturn* rreg, AsmArea* area, int prefer_second, int argument_flag)
{
	char* _temp = prefer_second ? "rbx" : "rax";

	char buff[64];
	
	// seta o register pra 1 caso um e o outro for 1, caso constrario seta pra 0
	snprintf(buff, 64, "	and	%s, %s", lreg->result, rreg->result);
	add_line_to_area(area, buff);

	snprintf(buff, 64, "	movzx	%s, %s", _temp, lreg->result);
	add_line_to_area(area, buff);

	AsmReturn* ret = create_asm_return(lreg->result, create_type(TYPE_BOOL, NULL));

	return ret;
}
