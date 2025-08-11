#include <string.h>

#include "codegen-switch.h"
#include "../../expressions/codegen-expr.h"

int switches_count = 0;
int switches_case_count = 0;

extern char* mov_opcode(VarType type);
extern char* get_opcode_for_cmp(Type* type);
extern char* field_get_reference_access_size(CodeGen* codegen, Type* type);
extern void handle_float_argument(CodeGen* codegen, AsmReturn* reg, AsmArea* area, int is_double);

static void generate_case(CodeGen* codegen, Node* node, AsmArea* area, AsmReturn* expr, int id)
{
	char buff[64];

	snprintf(buff, 64, ".switch_case_%d", id);
	add_line_to_area(area, buff);

	Node* next = node->switch_case_block_node.switch_case_block.block->block_node.block.statements->head;
	int scope = node->switch_case_block_node.switch_case_block.new_scope;

	SymbolTable* temp = codegen->scope;

	if (scope)
	{
		codegen->scope = node->switch_case_block_node.switch_case_block.scope;
	}
	
	while (next != NULL)
	{
		generate_node(codegen, next, area);

		next = next->next;
	}

	if (scope)
	{
		codegen->scope = temp;
	}
}

static void generate_switch_post(CodeGen* codegen, Node* node, AsmArea* area, int id)
{
	char buff[64];

	snprintf(buff, 64, ".switch_post_%d", id);
	add_line_to_area(area, buff);
}

void generate_switch_cases(CodeGen* codegen, Node* node, AsmArea* area, AsmReturn* expr)
{
	Node* next = node->switch_statement_node.switch_statement.case_list->head;

	while (next != NULL)
	{
		generate_case(codegen, next, area, expr, switches_case_count++);

		next = next->next;
	}
}

void generate_switch_jmp(CodeGen* codegen, Node* node, AsmArea* area, char* expr, int id)
{
	Node* next = node->switch_statement_node.switch_statement.case_list->head;
	char buff[64];

	while (next != NULL)
	{
		AsmReturn* ret = generate_expression(codegen, next->switch_case_block_node.switch_case_block.condition, area, 1, 0, 0, 0);

		char* op = get_opcode_for_cmp(ret->type);
		
		snprintf(buff, 64, "	%s	%s, %s", op, ret->result, expr);
		add_line_to_area(area, buff);

		snprintf(buff, 64, "	je	.switch_case_%d", id);
		add_line_to_area(area, buff);

		next = next->next;
		id++;
	}
}

void generate_switch(CodeGen* codegen, Node* node, AsmArea* area)
{
	AsmReturn* expr = generate_expression(codegen, node->switch_statement_node.switch_statement.value, area, 1, 0, 0, 0);
	int id = switches_count++;
	
	char buff[64];
	char* reg = strdup("rdx");

	snprintf(buff, 64, "	%s	%s, %s", mov_opcode(expr->type->type), reg, expr->result); // registrador não usado (nao ser sobrescrito)
	add_line_to_area(area, buff);

	if (expr->type->type != TYPE_FLOAT && expr->type->type != TYPE_DOUBLE)
	{
		snprintf(buff, 64, "	push	%s", reg); // salva o valor na stack (pra não ser sobrescrito)
		add_line_to_area(area, buff);
	}
	else
	{
		AsmReturn* ret = create_asm_return(reg, expr->type);
		handle_float_argument(codegen, ret, area, expr->type->type != TYPE_DOUBLE);
	}

	int case_id = switches_case_count;

	snprintf(buff, 64, "%s [rsp]", field_get_reference_access_size(codegen, expr->type));
	
	generate_switch_jmp(codegen, node, area, strdup(buff), case_id++);

	generate_switch_cases(codegen, node, area, expr);

	generate_switch_post(codegen, node, area, id);

	snprintf(buff, 64, "	pop	%s", reg);
	add_line_to_area(area, buff);
}