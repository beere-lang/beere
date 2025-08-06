#include <string.h>
#include <stdlib.h>

#include "codegen-ref.h"
#include "../../../../../frontend/semantic/analyzer/analyzer.h"
#include "../../oop/codegen-class.h"

Symbol* find_class_owner(SymbolTable* scope)
{
	if (scope == NULL)
	{
		return NULL;
	}

	if (scope->scope_kind == SYMBOL_CLASS)
	{
		return scope->owner_statement;
	}

	return find_class_owner(scope->parent);
}

static Symbol* find_symbol(CodeGen* codegen, Node* node)
{
	Symbol* symbol = analyzer_find_symbol_from_scope(node->variable_node.variable.identifier, codegen->scope, 1, 0, 0, 0);

	SymbolTable* curr_scope = codegen->scope;
	
	// procura pelo symbol nas supers caso ele não esteja em nenhum escopo anexado ao atual
	while (symbol == NULL)
	{
		if (curr_scope == NULL)
		{
			break;
		}
		
		Symbol* class_owner = find_class_owner(curr_scope);

		if (class_owner == NULL)
		{
			break;
		}
		
		symbol = analyzer_find_symbol_from_scope(node->variable_node.variable.identifier, class_owner->symbol_class->class_scope, 1, 0, 0, 0);

		if (class_owner->symbol_class->super == NULL)
		{
			break;
		}

		curr_scope = class_owner->symbol_class->super->symbol_class->class_scope;
	}

	return symbol;
}

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

		case TYPE_CLASS:
		{
			return strdup("qword");
		}

		case TYPE_BOOL:
		{
			return strdup("byte");
		}

		default:
		{
			printf("Codegen debug fail #123...\n");
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

		case TYPE_CLASS:
		{
			return strdup("mov");
		}

		case TYPE_BOOL:
		{
			return strdup("mov");
		}

		default:
		{
			printf("Codegen debug fail #089...\n");
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

		case TYPE_CLASS:
		{
			temp = prefer_second ? "rbx" : "rax";

			return strdup(temp);
		}

		case TYPE_BOOL:
		{
			temp = prefer_second ? "bl" : "al";

			return strdup(temp);
		}

		default:
		{
			printf("Codegen debug fail #102...\n");
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

		snprintf(buff, 64, "	%s	%s, %s [rbp%+d]", get_mov_op_code_access(codegen, type), reg, get_reference_access_size(codegen, type), symbol->symbol_variable->offset);
		add_line_to_area(area, buff);

		AsmReturn* ret = create_asm_return(reg, type);
		ret->is_reg = 1;
		
		return ret;
	}
	else
	{
		snprintf(buff, 64, "%s [rbp%+d]", get_reference_access_size(codegen, type), symbol->symbol_variable->offset);
		
		AsmReturn* ret = create_asm_return(buff, type);

		return ret;
	} 
}

AsmReturn* generate_global_variable_reference(CodeGen* codegen, Symbol* symbol, AsmArea* area, int force_reg, int prefer_second)
{
	char buff[64];
	Type* type = symbol->symbol_variable->type;
	
	if (force_reg)
	{
		char* reg = get_register_access(codegen, type, prefer_second);

		snprintf(buff, 64, "	%s	%s, %s [rel %s]", get_mov_op_code_access(codegen, type), reg, get_reference_access_size(codegen, type), symbol->symbol_variable->identifier);
		add_line_to_area(area, buff);

		AsmReturn* ret = create_asm_return(reg, type);
		ret->is_reg = 1;
		
		return ret;
	}
	else
	{
		snprintf(buff, 64, "%s [rel %s]", get_reference_access_size(codegen, type),symbol->symbol_variable->identifier);
		
		AsmReturn* ret = create_asm_return(buff, type);
		return ret;
	}
}

AsmReturn* generate_static_variable_reference(CodeGen* codegen, Symbol* symbol, AsmArea* area, int force_reg, int prefer_second)
{
	char buff[64];
	Type* type = symbol->symbol_variable->type;
	
	if (force_reg)
	{
		char* reg = get_register_access(codegen, type, prefer_second);

		snprintf(buff, 64, "	%s	%s, %s [rel .static_%s]", get_mov_op_code_access(codegen, type), reg, get_reference_access_size(codegen, type), symbol->symbol_variable->identifier);
		add_line_to_area(area, buff);

		AsmReturn* ret = create_asm_return(reg, type);
		ret->is_reg = 1;
		
		return ret;
	}
	else
	{
		snprintf(buff, 64, "%s [rel .static_%s]", get_reference_access_size(codegen, type),symbol->symbol_variable->identifier);
		
		AsmReturn* ret = create_asm_return(buff, type);
		return ret;
	}
}

AsmReturn* generate_class_global_variable_reference(CodeGen* codegen, Symbol* symbol, AsmArea* area, int force_reg, int prefer_second)
{
	char buff[64];
	Type* type = symbol->symbol_variable->type;
	
	Symbol* class_owner = find_class_owner(codegen->scope);
	ClassOffsets* offsets = find_class_offsets(class_offsets_table, (char*) class_owner->symbol_class->identifier);

	int offset = find_field_offset(offsets, (char*) symbol->symbol_variable->identifier);
	
	if (force_reg)
	{
		char* reg = get_register_access(codegen, type, prefer_second);

		snprintf(buff, 64, "	%s	%s, %s [r8%+d]", get_mov_op_code_access(codegen, type), reg, get_reference_access_size(codegen, type), offset);
		add_line_to_area(area, buff);

		AsmReturn* ret = create_asm_return(reg, type);
		ret->is_reg = 1;
		
		return ret;
	}
	else
	{
		snprintf(buff, 64, "%s [r8%+d]", get_reference_access_size(codegen, type), offset);
		
		AsmReturn* ret = create_asm_return(buff, type);

		return ret;
	}
}

AsmReturn* generate_variable_reference(CodeGen* codegen, Node* node, AsmArea* area, int force_reg, int prefer_second, int argument_flag)
{
	VariableNode* identifier_node = &node->variable_node.variable;
	
	Symbol* symbol = find_symbol(codegen, node);

	if (symbol->symbol_variable->is_static)
	{
		return generate_static_variable_reference(codegen, symbol, area, force_reg, prefer_second);
	}
	
	if (symbol->symbol_variable->is_global)
	{
		return generate_global_variable_reference(codegen, symbol, area, force_reg, prefer_second);
	}

	if (symbol->symbol_variable->is_class_global)
	{
		return generate_class_global_variable_reference(codegen, symbol, area, force_reg, prefer_second);
	}

	return generate_local_variable_reference(codegen, symbol, area, force_reg, prefer_second);
}