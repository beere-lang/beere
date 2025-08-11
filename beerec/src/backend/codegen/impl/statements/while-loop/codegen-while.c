#include "codegen-while.h"
#include "../../expressions/codegen-expr.h"

int whiles_count = 0;

static void generate_while_post(CodeGen* codegen, Node* node, AsmArea* area, int id)
{
	char buff[64];
	
	snprintf(buff, 64, ".while_post_%d", id);
	add_line_to_area(area, buff);
}

static void generate_while_block(CodeGen* codegen, Node* node, AsmArea* area, int id)
{
	Node* next = node->while_loop_node.while_loop.then_block->block_node.block.statements->head;
	SymbolTable* temp = codegen->scope;
	
	codegen->scope = node->while_loop_node.while_loop.then_scope;

	char buff[64];

	snprintf(buff, 64, ".while_then_%d", id);
	add_line_to_area(area, buff);
	
	while (next != NULL)
	{
		generate_node(codegen, next, area);

		next = next->next;
	}

	codegen->scope = temp;
}

static void generate_while_condition(CodeGen* codegen, Node* node, AsmArea* area, int id)
{
	AsmReturn* ret = generate_expression(codegen, node->while_loop_node.while_loop.condition, area, 1, 0, 0, 0);
	char buff[64];

	snprintf(buff, 64, "	test	%s, %s", ret->result, ret->result);
	add_line_to_area(area, buff);

	snprintf(buff, 64, "	jnz	.while_then_%d", id);
	add_line_to_area(area, buff);

	snprintf(buff, 64, "	jmp	.while_post_%d", id);
	add_line_to_area(area, buff);
}

void generate_while(CodeGen* codegen, Node* node, AsmArea* area)
{
	int id = whiles_count++;

	generate_while_block(codegen, node, area, id);
	generate_while_condition(codegen, node, area, id);
	generate_while_post(codegen, node, area, id);
}