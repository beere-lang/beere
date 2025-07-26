#include <stdlib.h>

#include "codegen-cast.h"
#include "../codegen-expr.h"
#include "../../../../parser/parser.h"

static AsmReturn* generate_to_int(CodeGen* codegen, Node* node, AsmArea* area, int prefer_second)
{
	AsmReturn* expr = generate_expression(codegen, node->cast_statement_node.cast_node.expression, area, 1, 0, 0);
	char buff[64];
	
	switch (expr->type->type)
	{
		case TYPE_FLOAT:
		{
			snprintf(buff, 64, "	cvttss2si	ecx, %s", expr->result);
			add_line_to_area(area, buff);

			return create_asm_return("ecx", create_type(TYPE_INT, NULL));
		}

		case TYPE_DOUBLE:
		{
			snprintf(buff, 64, "	cvttsd2si	ecx, %s", expr->result);
			add_line_to_area(area, buff);

			return create_asm_return("ecx", create_type(TYPE_INT, NULL));
		}

		default:
		{
			return expr;
		}
	}
}

static AsmReturn* generate_to_float(CodeGen* codegen, Node* node, AsmArea* area, int prefer_second)
{
	AsmReturn* expr = generate_expression(codegen, node->cast_statement_node.cast_node.expression, area, 1, 0, 0);
	char buff[64];
	
	switch (expr->type->type)
	{
		case TYPE_INT:
		{
			snprintf(buff, 64, "	cvttsi2ss	xmm2, %s", expr->result);
			add_line_to_area(area, buff);

			return create_asm_return("xmm2", create_type(TYPE_FLOAT, NULL));
		}

		case TYPE_DOUBLE:
		{
			snprintf(buff, 64, "	cvttsd2ss	xmm2, %s", expr->result);
			add_line_to_area(area, buff);

			return create_asm_return("xmm2", create_type(TYPE_FLOAT, NULL));
		}

		default:
		{
			return expr;
		}
	}
}

static AsmReturn* generate_to_double(CodeGen* codegen, Node* node, AsmArea* area, int prefer_second)
{
	AsmReturn* expr = generate_expression(codegen, node->cast_statement_node.cast_node.expression, area, 1, 0, 0);
	char buff[64];
	
	switch (expr->type->type)
	{
		case TYPE_INT:
		{
			snprintf(buff, 64, "	cvttsi2sd	xmm2, %s", expr->result);
			add_line_to_area(area, buff);

			return create_asm_return("xmm2", create_type(TYPE_DOUBLE, NULL));
		}

		case TYPE_FLOAT:
		{
			snprintf(buff, 64, "	cvttss2sd	xmm2, %s", expr->result);
			add_line_to_area(area, buff);

			return create_asm_return("xmm2", create_type(TYPE_DOUBLE, NULL));
		}

		default:
		{
			return expr;
		}
	}
}

AsmReturn* generate_cast(CodeGen* codegen, Node* node, AsmArea* area, int prefer_second, int argument_flag)
{
	switch (node->cast_statement_node.cast_node.cast_type->type)
	{
		case TYPE_INT:
		{
			return generate_to_int(codegen, node, area, prefer_second);
		}

		case TYPE_FLOAT:
		{
			return generate_to_float(codegen, node, area, prefer_second);
		}

		case TYPE_DOUBLE:
		{
			return generate_to_double(codegen, node, area, prefer_second);
		}
		
		default:
		{
			exit(1);
		}
	}
}