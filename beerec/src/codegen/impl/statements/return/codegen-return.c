#include <stdlib.h>
#include <string.h>

#include "codegen-return.h"
#include "../../expressions/codegen-expr.h"

extern char* get_mov_op_code_access(CodeGen* codegen, Type* type);

char* get_return_register(Type* type)
{
	switch (type->type)
	{
		case TYPE_BOOL:
		{
			return strdup("al");
		}

		case TYPE_CHAR:
		{
			return strdup("al");
		}
		
		case TYPE_INT:
		{
			return strdup("eax");
		}

		case TYPE_FLOAT:
		{
			return strdup("xmm0");
		}

		case TYPE_DOUBLE:
		{
			return strdup("xmm0");
		}

		case TYPE_PTR:
		{
			return strdup("rax");
		}

		case TYPE_STRING:
		{
			return strdup("rax");
		}

		default:
		{
			exit(1);
		}
	}
}

AsmReturn* generate_return(CodeGen* codegen, Node* node, AsmArea* area)
{
	AsmReturn* ret = generate_expression(codegen, node->return_statement_node.return_statement.return_value, area, 0, 1, 0);
	char* reg = get_return_register(ret->type);

	char buff[64];
	snprintf(buff, 64, "	%s	%s, %s", get_mov_op_code_access(codegen, ret->type), reg, ret->result);

	add_line_to_area(area, buff);

	return create_asm_return(reg, ret->type);
}