#include "codegen-field-assgn.h"
#include "../../../expressions/codegen-expr.h"
#include "../../../../../../frontend/semantic/analyzer/analyzer.h"

extern char* field_get_mov_op_code_access(CodeGen* codegen, Type* type);
extern char* field_get_reference_access_size(CodeGen* codegen, Type* type);

void generate_field_assign(CodeGen* codegen, Node* node, AsmArea* area)
{
	int force_reg = (node->variable_assign_node.variable_assign.assign_value->type == NODE_LITERAL) ? 0 : 1;
	
	AsmReturn* value = generate_expression(codegen, node->variable_assign_node.variable_assign.assign_value, area, force_reg, 0, 0, 0);
	AsmReturn* ref = generate_expression(codegen, node->variable_assign_node.variable_assign.left, area, 0, 0, 1, 0);
	char buff[64];

	Type* type = analyzer_return_type_of_expression(NULL, node->variable_assign_node.variable_assign.left, codegen->scope, NULL, 0, NULL);
	
	char* fmt = ref->is_reg ? "	%s	%s [%s], %s" : "	%s	%s, %s";
	
	if (ref->is_reg)
	{
		snprintf(buff, 64, fmt, field_get_mov_op_code_access(codegen, value->type), field_get_reference_access_size(codegen, type), ref->result, value->result);
	}
	else
	{
		snprintf(buff, 64, fmt, field_get_mov_op_code_access(codegen, value->type), ref->result, value->result);
	}

	add_line_to_area(area, buff);
}