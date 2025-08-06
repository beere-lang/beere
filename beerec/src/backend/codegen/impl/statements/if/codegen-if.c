#include "codegen-if.h"
#include "../../expressions/codegen-expr.h"

static char* generate_else(CodeGen* codegen, Node* node, AsmArea* area)
{

}

static char* generate_if_then(CodeGen* codegen, Node* node, AsmArea* area)
{

}

static AsmReturn* generate_if_condition(CodeGen* codegen, Node* expr, AsmArea* area)
{
	AsmReturn* ret = generate_expression(codegen, expr, area, 0, 0, 0);

	return ret;
}

static void generate_if_statement(CodeGen* codegen, Node* node, AsmArea* area)
{
	AsmReturn* ret = generate_if_condition(codegen, node->if_statement_node.if_statement.condition_top, area);

	int then_id = if_thens_count++;

	char buff[64];

	snprintf(buff, 64, "	jnz	.if_then_%d", then_id);
	add_line_to_area(area, buff);
}

void generate_if(CodeGen* codegen, Node* node, AsmArea* area)
{
	Node* if_node = node;

	while (if_node->type == NODE_IF)
	{
		generate_if_statement(codegen, if_node, area);

		if_node = if_node->if_statement_node.if_statement.else_branch;
	}

	generate_else(codegen, if_node, area);
}