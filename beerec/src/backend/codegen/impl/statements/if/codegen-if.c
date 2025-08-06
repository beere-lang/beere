#include "codegen-if.h"
#include "../../expressions/codegen-expr.h"

int if_thens_count = 0;
int if_elses_count = 0;
int if_posts_count = 0;

static void generate_else_block(CodeGen* codegen, Node* node, AsmArea* area, int id)
{
	char buff[64];
	
	snprintf(buff, 64, ".if_else_%d", id);
	add_line_to_area(area, buff);
	
	Node* next = node->block_node.block.statements->head;

	while (next != NULL)
	{
		generate_node(codegen, next, area);
		
		next = next->next;
	}
}

static void generate_else(CodeGen* codegen, Node* node, AsmArea* area)
{
	int else_id = if_elses_count++;

	generate_else_block(codegen, node, area, else_id);
}

static void generate_then_block(CodeGen* codegen, Node* node, AsmArea* area, int id)
{
	char buff[64];
	Node* next = node->block_node.block.statements->head;

	snprintf(buff, 64, ".if_then_%d", id);
	add_line_to_area(text_section, buff);

	while (next != NULL)
	{
		generate_node(codegen, next, text_section);
		
		next = next->next;
	}

	snprintf(buff, 64, "	jmp	.if_post_%d", if_posts_count);
	add_line_to_area(text_section, buff);
}

static void generate_post(CodeGen* codegen, Node* node, AsmArea* area)
{
	int id = if_posts_count++;
	
	char buff[64];
	
	snprintf(buff, 64, ".if_post_%d", id);
	add_line_to_area(area, buff);
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

	generate_then_block(codegen, node->if_statement_node.if_statement.then_branch, area, then_id);

	snprintf(buff, 64, "	test	%s, %s", ret->result, ret->result);
	add_line_to_area(area, buff);
	
	// da jump se não for zero (1 --> true)
	snprintf(buff, 64, "	jnz	.if_then_%d", then_id);
	add_line_to_area(area, buff);
}

void generate_if(CodeGen* codegen, Node* node, AsmArea* area)
{
	Node* if_node = node;
	SymbolTable* else_scope = NULL;

	while (if_node->type == NODE_IF)
	{
		SymbolTable* temp = codegen->scope;
		codegen->scope = if_node->if_statement_node.if_statement.then_scope;
		
		generate_if_statement(codegen, if_node, area);

		else_scope = if_node->if_statement_node.if_statement.else_scope;
		if_node = if_node->if_statement_node.if_statement.else_branch;
		
		codegen->scope = temp;
		
		if (if_node == NULL)
		{
			else_scope = NULL;
			break;
		}
	}

	if (else_scope != NULL)
	{
		SymbolTable* temp = codegen->scope;
		codegen->scope = else_scope;

		generate_else(codegen, if_node, area);

		codegen->scope = temp;
	}

	generate_post(codegen, node, area);
}