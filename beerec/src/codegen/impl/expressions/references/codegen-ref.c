#include <string.h>
#include <stdlib.h>

#include "codegen-ref.h"
#include "../../../../analyzer/analyzer.h"

char* get_reference_access_size(CodeGen* codegen, Type* type)
{
	switch (type->type)
	{
		case TYPE_INT:
		{
			return strdup("dword");
		}

		case TYPE_FLOAT:
		{
			return strdup("dword");
		}

		case TYPE_DOUBLE:
		{
			return strdup("qword");
		}

		case TYPE_PTR:
		{
			return strdup("qword");
		}

		case TYPE_STRING:
		{
			return strdup("qword");
		}

		default:
		{
			exit(1);
		}
	}
}

char* get_mov_op_code_access(CodeGen* codegen, Type* type)
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

		case TYPE_PTR:
		{
			return strdup("mov");
		}

		case TYPE_STRING:
		{
			return strdup("mov");
		}

		default:
		{
			exit(1);
		}
	}
}

char* get_register_access(CodeGen* codegen, Type* type, int prefer_second)
{
	char* temp = NULL;
	
	switch (type->type)
	{
		case TYPE_INT:
		{
			temp = prefer_second ? "ebx" : "eax";

			return strdup(temp);
		}

		case TYPE_FLOAT:
		{
			temp = prefer_second ? "xmm1" : "xmm0";

			return strdup(temp);
		}

		case TYPE_DOUBLE:
		{
			temp = prefer_second ? "xmm1" : "xmm0";

			return strdup(temp);
		}

		case TYPE_PTR:
		{
			temp = prefer_second ? "rbx" : "rax";

			return strdup(temp);
		}

		case TYPE_STRING:
		{
			temp = prefer_second ? "rbx" : "rax";

			return strdup(temp);
		}

		default:
		{
			exit(1);
		}
	}
}

AsmReturn* generate_local_variable_reference(CodeGen* codegen, Symbol* symbol, AsmArea* area, int force_reg, int prefer_second)
{
	char buff[64];
	Type* type = symbol->symbol_variable->type;
	
	if (force_reg)
	{
		char* reg = get_register_access(codegen, type, prefer_second);

		snprintf(buff, 64, "	%s	%s, %s [rbp%+d]", get_mov_op_code_access(codegen, type), reg, get_reference_access_size(codegen, type),symbol->symbol_variable->offset);
		add_line_to_area(area, buff);

		AsmReturn* ret = create_asm_return(reg, type);
		ret->is_reg = 1;
		
		return ret;
	}
	else
	{
		snprintf(buff, 64, "%s [rbp%+d]", get_reference_access_size(codegen, type),symbol->symbol_variable->offset);
		
		AsmReturn* ret = create_asm_return(buff, type);
		return ret;
	}
}

AsmReturn* generate_variable_reference(CodeGen* codegen, Node* node, AsmArea* area, int force_reg, int prefer_second)
{
	VariableNode* identifier_node = &node->variable_node.variable;
	
	Symbol* symbol = analyzer_find_symbol_from_scope(identifier_node->identifier, codegen->scope, 1, 0, 0, 0);

	if (symbol->symbol_variable->is_static)
	{
		// TODO
	}
	
	if (symbol->symbol_variable->is_global)
	{
		// TODO:
	}

	return generate_local_variable_reference(codegen, symbol, area, force_reg, prefer_second);
}