#include "codegen-field-assgn.h"
#include "../../../expressions/codegen-expr.h"

extern char* field_get_mov_op_code_access(CodeGen* codegen, Type* type);

void generate_field_assign(CodeGen* codegen, Node* node, AsmArea* area)
{
	AsmReturn* value = generate_expression(codegen, node->variable_assign_node.variable_assign.assign_value, area, 1, 0);
	AsmReturn* ref = generate_expression(codegen, node->variable_assign_node.variable_assign.left, area, 0, 0);
	char buff[64];

	snprintf(buff, 64, ref->is_reg ? "	%s	[%s], %s" : "	%s	%s, %s", field_get_mov_op_code_access(codegen, value->type), ref->result, value->result);
	add_line_to_area(area, buff);
}