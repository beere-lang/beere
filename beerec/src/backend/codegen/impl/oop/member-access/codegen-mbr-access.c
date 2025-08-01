#include <stdlib.h>
#include <string.h>

#include "codegen-mbr-access.h"
#include "../../expressions/codegen-expr.h"
#include "../../../../../frontend/analyzer/analyzer.h"

extern char* field_get_reference_access_size(CodeGen* codegen, Type* type);
extern char* correct_register(VarType type, int prefer_second);
extern char* mov_opcode(VarType type);

char* get_object_class_name(CodeGen* codegen, Node* expr)
{
	Type* class_type = analyzer_return_type_of_expression(NULL, expr, codegen->scope, NULL, 0, NULL);

	if (class_type->type != TYPE_CLASS)
	{
		exit(1);
	}

	char* class_name = class_type->class_name;

	return class_name;
}

Symbol* find_field_symbol_from_class(CodeGen* codegen, Symbol* object, char* field_name)
{
	SymbolTable* scope = object->symbol_class->class_scope;

	for (int i = 0; i < object->symbol_class->field_count; i++)
	{
		Node* field = object->symbol_class->fields[i];
		char* field_name = field->declare_node.declare.identifier;

		if (strcmp(field_name, field_name) == 0)
		{
			Symbol* field_symbol = analyzer_find_symbol_from_scope(field_name, scope, 1, 0, 0, 0);
			
			return field_symbol;
		}
	}

	return NULL;
}

AsmReturn* generate_member_access(CodeGen* codegen, Node* node, AsmArea* area, int force_reg, int prefer_second, int argument_flag)
{
	char buff[64];
	
	Node* expr = node->member_access_node.member_access.object;
	char* member_name = node->member_access_node.member_access.member_name;
	AsmReturn* object_expr = generate_expression(codegen, expr, area, 1, prefer_second, 0);

	char* class_name = get_object_class_name(codegen, expr);
	Symbol* obj_symbol = analyzer_find_symbol_from_scope(class_name, codegen->scope, 0, 0, 1, 0);

	Symbol* field_symbol = find_field_symbol_from_class(codegen, obj_symbol, member_name);
	int offset = field_symbol->symbol_variable->offset;

	Type* type = field_symbol->symbol_variable->type;
	
	if (force_reg)
	{
		char* mov_op = mov_opcode(type->type);
		char* reg = correct_register(type->type, prefer_second);
		char* access_size = field_get_reference_access_size(codegen, type);
		snprintf(buff, 64, "	%s	%s [%s+%d], %s", mov_op, access_size, object_expr->result, offset, reg);

		add_line_to_area(area, buff);
		
		AsmReturn* ret = create_asm_return(reg, type);
		ret->is_reg = 1;

		return ret;
	}
	else
	{
		char* mov_op = mov_opcode(type->type);
		char* reg = correct_register(type->type, prefer_second);
		char* access_size = field_get_reference_access_size(codegen, type);
		snprintf(buff, 64, "%s [%s+%d]", access_size, object_expr->result, offset);

		AsmReturn* ret = create_asm_return(reg, type);
		
		return ret;
	}
}