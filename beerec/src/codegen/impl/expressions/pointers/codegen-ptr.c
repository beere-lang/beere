#include <stdlib.h>

#include "codegen-ptr.h"
#include "../codegen-expr.h"
#include "../../../../analyzer/analyzer.h"

extern char* field_get_reference_access_size(CodeGen* codegen, Type* type);

char* mov_opcode(VarType type)
{
	switch (type)
	{
		case TYPE_INT:
		{
			return "mov";
		}

		case TYPE_FLOAT:
		{
			return "movss";
		}

		case TYPE_DOUBLE:
		{
			return "movsd";
		}

		case TYPE_PTR:
		{
			return "mov";
		}

		case TYPE_BOOL:
		{
			return "mov";
		}

		case TYPE_CHAR:
		{
			return "mov";
		}

		case TYPE_STRING:
		{
			return "mov";
		}
		
		default:
		{
			printf("Codegen debug fail #235...\n");
			exit(1);
		}
	}
}

char* correct_register(VarType type, int prefer_second)
{
	switch (type)
	{
		case TYPE_INT:
		{
			return prefer_second ? "ebx" : "eax";
		}

		case TYPE_FLOAT:
		{
			return prefer_second ? "xmm1" : "xmm0";
		}

		case TYPE_DOUBLE:
		{
			return prefer_second ? "xmm1" : "xmm0";
		}

		case TYPE_PTR:
		{
			return prefer_second ? "rbx" : "rax";
		}

		case TYPE_BOOL:
		{
			return prefer_second ? "bl" : "al";
		}

		case TYPE_CHAR:
		{
			return prefer_second ? "bl" : "al";
		}

		case TYPE_STRING:
		{
			return prefer_second ? "rbx" : "rax";
		}
		
		default:
		{
			printf("Codegen debug fail #427...\n");
			exit(1);
		}
	}
}

AsmReturn* generate_dereference(CodeGen* codegen, Node* node, AsmArea* area, int force_reg, int prefer_second)
{
	char buff[64];
	int depth = 0;
	Node* main_expr = node->dereference_node.dereference.ptr;
	
	AsmReturn* expr = generate_expression(codegen, main_expr, area, 0, prefer_second, 0);

	Type* type = analyzer_return_type_of_expression(NULL, main_expr, codegen->scope, NULL, 0, NULL);
	char* temp = correct_register(type->type, prefer_second);
	
	/* ----------------------------------------------------------- */

	char* _temp = expr->is_reg ? "	mov	%s, [%s]" : "	mov	%s, %s";

	snprintf(buff, 64, _temp, temp, expr->result);
	add_line_to_area(area, buff);

	char* res = temp;
	
	if (force_reg)
	{
		Type* type = analyzer_return_type_of_expression(NULL, node, codegen->scope, NULL, 0, NULL);
		
		char* opcode = mov_opcode(type->type);
		char* access_size = field_get_reference_access_size(codegen, type);
		char* reg = correct_register(type->type, prefer_second);

		res = reg;
		
		snprintf(buff, 64, "	%s	%s, %s [%s]", opcode, reg, access_size, temp);
		add_line_to_area(area, buff);
	}

	/* ----------------------------------------------------------- */

	Type* res_type = analyzer_return_type_of_expression(NULL, node, codegen->scope, NULL, 0, NULL);

	AsmReturn* ret = create_asm_return(res, res_type);
	ret->is_reg = 1;

	return ret;
}

AsmReturn* generate_reference(CodeGen* codegen, Node* node, AsmArea* area, int force_reg, int prefer_second)
{
	char buff[64];
	int depth = 0;
	
	Node* main_expr = node->adress_of_node.adress_of.expression;

	char* temp = prefer_second ? "rbx" : "rax";

	AsmReturn* expr = generate_expression(codegen, main_expr, area, 0, prefer_second, 0);

	/* ----------------------------------------------------------- */

	snprintf(buff, 64, "	lea	%s, %s", temp, expr->result);
	add_line_to_area(area, buff);

	/* ----------------------------------------------------------- */

	Type* type = analyzer_return_type_of_expression(NULL, node, codegen->scope, NULL, 0, NULL);

	AsmReturn* ret = create_asm_return(temp, type);
	ret->is_reg = 1;

	return ret;
}