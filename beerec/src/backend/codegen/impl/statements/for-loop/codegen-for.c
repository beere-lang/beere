#include "codegen-for.h"
#include "../../expressions/codegen-expr.h"

int fors_count = 0;

static void generate_for_then(CodeGen* codegen, Node* node, AsmArea* area, int id)
{
	Node* next = node->for_loop_node.for_loop.then_block->block_node.block.statements->head;

	while (next != NULL)
	{
		generate_node(codegen, next, area);

		next = next->next;
	}
}

static void generate_for_condition(CodeGen* codegen, Node* node, AsmArea* area, int id)
{
	AsmReturn* ret = generate_expression(codegen, node->for_loop_node.for_loop.condition, area, 1, 0, 0);
	char buff[64];

	snprintf(buff, 64, ".for_then_%d:", id);
	add_line_to_area(area, buff);
	
	snprintf(buff, 64, "	test	%s, %s", ret->result, ret->result);
	add_line_to_area(area, buff);
	
	snprintf(buff, 64, "	jz	.for_post_%d", id);
	add_line_to_area(area, buff);
}

static void generate_for_inc(CodeGen* codegen, Node* node, AsmArea* area, int id)
{
	char buff[64];

	snprintf(buff, 64, ".for_inc_%d:", id);
	add_line_to_area(area, buff);

	generate_node(codegen, node->for_loop_node.for_loop.then_statement, area);

	snprintf(buff, 64, "	jmp	.for_then_%d", id);
	add_line_to_area(area, buff);
}

static void generate_for_init(CodeGen* codegen, Node* node, AsmArea* area)
{
	generate_node(codegen, node->for_loop_node.for_loop.init, area);
}

void generate_for(CodeGen* codegen, Node* node, AsmArea* area)
{
	int id = fors_count++;
	char buff[64];

	generate_for_init(codegen, node, area);
	
	generate_for_condition(codegen, node, area, id);

	SymbolTable* temp = codegen->scope;
	codegen->scope = node->for_loop_node.for_loop.then_scope;
	
	generate_for_then(codegen, node, area, id);

	codegen->scope = temp;

	generate_for_inc(codegen, node, area, id);

	snprintf(buff, 64, ".for_post_%d:", id);
	add_line_to_area(area, buff);
}