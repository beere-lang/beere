#include <stdlib.h>
#include <string.h>

#include "codegen-ptr.h"
#include "../codegen-expr.h"
#include "../../../../../frontend/semantic/analyzer/analyzer.h"

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

char* correct_register(VarType type, int prefer_second, int prefer_third)
{
	switch (type)
	{
		case TYPE_INT:
		{
			if (prefer_third)
			{
				return "edx";
			}

			return prefer_second ? "ebx" : "eax";
		}

		case TYPE_FLOAT:
		{
			if (prefer_third)
			{
				return "xmm3";
			}

			return prefer_second ? "xmm1" : "xmm0";
		}

		case TYPE_DOUBLE:
		{
			if (prefer_third)
			{
				return "xmm3";
			}

			return prefer_second ? "xmm1" : "xmm0";
		}

		case TYPE_PTR:
		{
			if (prefer_third)
			{
				return "rdx";
			}

			return prefer_second ? "rbx" : "rax";
		}

		case TYPE_BOOL:
		{
			if (prefer_third)
			{
				return "dl";
			}

			return prefer_second ? "bl" : "al";
		}

		case TYPE_CHAR:
		{
			if (prefer_third)
			{
				return "dl";
			}

			return prefer_second ? "bl" : "al";
		}

		case TYPE_STRING:
		{
			if (prefer_third)
			{
				return "rdx";
			}
			
			return prefer_second ? "rbx" : "rax";
		}
		
		default:
		{
			printf("Codegen debug fail #427...\n");
			exit(1);
		}
	}
}

AsmReturn* generate_dereference(CodeGen* codegen, Node* node, AsmArea* area, int force_reg, int prefer_second, int prefer_third)
{
	char buff[64];
	int depth = 0;
	Node* main_expr = node->dereference_node.dereference.ptr;
	
	AsmReturn* expr = generate_expression(codegen, main_expr, area, 1, prefer_second, prefer_third, 0);

	Type* type = analyzer_return_type_of_expression(NULL, main_expr, codegen->scope, NULL, 0, NULL);
	char* temp = correct_register(type->type, prefer_second, prefer_third);

	printf("Tipo: %d\n", type->type);
	
	/* ----------------------------------------------------------- */

	snprintf(buff, 64, "%s [%s]", field_get_reference_access_size(codegen, type->base), temp);
	char* res = strdup(buff);
	
	if (force_reg)
	{
		free(res);

		Type* type = analyzer_return_type_of_expression(NULL, node, codegen->scope, NULL, 0, NULL);
		
		char* opcode = mov_opcode(type->type);
		char* access_size = field_get_reference_access_size(codegen, type);
		char* reg = correct_register(type->type, prefer_second, prefer_third);

		res = reg;

		snprintf(buff, 64, "	%s	%s, %s [%s]", opcode, reg, access_size, temp);
		add_line_to_area(area, buff);
	}

	/* ----------------------------------------------------------- */

	Type* res_type = analyzer_return_type_of_expression(NULL, node, codegen->scope, NULL, 0, NULL);

	AsmReturn* ret = create_asm_return(res, res_type);
	ret->is_reg = force_reg;

	return ret;
}

AsmReturn* generate_reference(CodeGen* codegen, Node* node, AsmArea* area, int force_reg, int prefer_second, int prefer_third)
{
	char buff[64];
	int depth = 0;
	
	Node* main_expr = node->adress_of_node.adress_of.expression;

	char* temp = prefer_second ? "rbx" : "rax";

	if (prefer_third)
	{
		temp = "rdx";
	}

	AsmReturn* expr = generate_expression(codegen, main_expr, area, 0, prefer_second, prefer_third, 0);

	/* ----------------------------------------------------------- */

	snprintf(buff, 64, "	lea	%s, %s", temp, expr->result);
	add_line_to_area(area, buff);

	/* ----------------------------------------------------------- */

	Type* type = analyzer_return_type_of_expression(NULL, node, codegen->scope, NULL, 0, NULL);

	AsmReturn* ret = create_asm_return(temp, type);
	ret->is_reg = 1;

	return ret;
}