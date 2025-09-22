#include <stdlib.h>
#include <string.h>

#include "ir-gen.h"
#include "../../utils/logger/logger.h"

#define ENTRY_BLOCK_LABEL ".entry"

static IRNode* curr_func = NULL;
static IRNode* curr_block = NULL;

static IRNode* generate_ir_node(ASTNode* node);

static int count_linked_list(ASTNode* head)
{
	int count = 0;

	while (head != NULL)
	{
		head = head->next;
		count++;
	}

	return count;
}

static IRNode* generate_param(ASTNode* node)
{
	IRNode* param = create_ir_node(IR_NODE_PARAM);

	param->param.name = _strdup(node->param.identifier);
	param->param.type = node->param.argument_type;

	return param;
}

static void setup_func_params(IRNode* func, ASTNode* head, const int length)
{
	ASTNode* curr = head;

	for (int i = 0; i < length; i++)
	{
		func->func.params[i] = generate_param(curr);

		curr = curr->next;
	}
}

static void generate_func_instructions(ASTNode* head)
{
	ASTNode* curr = head;

	while (curr != NULL)
	{
		IRNode* backup = curr_block;
		add_element_to_list(backup->block.nodes, generate_ir_node(curr));

		curr = curr->next;
	}
}

static IRNode* generate_func(ASTNode* node)
{
	IRNode* func = create_ir_node(IR_NODE_FUNC);

	char buff[64];
	snprintf(buff, 64, ".fn_%s", node->function.identifier);

	func->func.name = _strdup(buff);

	func->func.type = node->function.return_type;

	const int length = count_linked_list(node->function.params->head);

	func->func.params = malloc(sizeof(IRNode*) * length);
	func->func.params_size = length;

	setup_func_params(func, node->function.params->head, length);

	IRNode* entry = create_ir_node(IR_NODE_BLOCK);

	entry->block.nodes = create_list(16);
	entry->block.label = _strdup(ENTRY_BLOCK_LABEL);

	curr_func = func;
	curr_block = entry;

	generate_func_instructions(node->function.block->block.statements->head);

	return func;
}

static IRNode* generate_ir_node(ASTNode* node)
{
	switch (node->type)
	{
		case NODE_FUNC:
		{
			return generate_func(node);
		}

		default:
		{
			println("Node with type id: %d, not implemented...", node->type);
			exit(1);
		}
	}
}

IRNode** generate_ir_nodes(ASTNode** nodes, const int length)
{
	IRNode** irs = malloc(sizeof(IRNode*) * length);

	for (int i = 0; i < length; i++)
	{
		irs[i] = generate_ir_node(nodes[i]);
	}

	return irs;
}
