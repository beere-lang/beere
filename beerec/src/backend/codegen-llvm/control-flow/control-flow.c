#include <stdlib.h>

#include "control-flow.h"
#include "../../../utils/logger/logger.h"

static const int CF_BLOCK_LIST_DEFAULT_START_CAPACITY = 4;

CFBlockList* cf_blocks = NULL;

CFBlockList* setup_list()
{
	CFBlockList* list = malloc(sizeof(CFBlockList));

	list->capacity = CF_BLOCK_LIST_DEFAULT_START_CAPACITY;
	list->length = 0;

	list->elements = malloc(sizeof(CFBlock*) * CF_BLOCK_LIST_DEFAULT_START_CAPACITY);

	return list;
}

void add_element_to_list(CFBlockList* list, CFBlock* block)
{
	if (list->length >= list->capacity)
	{
		list->capacity *= 2;
		list->elements = realloc(list->elements, sizeof(CFBlock*) * list->capacity);
	}

	list->elements[list->length++] = block;
}

void setup_cf_blocks()
{
	cf_blocks = setup_list();
}

CFBlock* find_cf_block(IRNode* block)
{
	if (cf_blocks == NULL)
	{
		return NULL;
	}
	
	int length = cf_blocks->length;

	for (int i = 0; i < length; i++)
	{
		CFBlock* curr = cf_blocks->elements[i];

		if (curr == NULL)
		{
			continue;
		}

		if (curr->block != block)
		{
			continue;
		}

		return curr;
	}

	return NULL;
}

IRNode* get_tail(IRNode* block)
{
	int length =  block->block.nodes->length;

	for (int i = 0; i < length; i++)
	{
		IRNode* it = block->block.nodes->elements[i];

		if (it == NULL || i < length - 1)
		{
			continue;
		}

		return it;
	}

	return NULL;
}

IRNode* find_next_block(IRNodeList* blocks, IRNode* block)
{
	int length = blocks->length;
	
	for (int i = 0; i < length; i++)
	{
		IRNode* curr = blocks->elements[i];

		if (curr == NULL)
		{
			continue;
		}

		if (curr != block)
		{
			continue;
		}

		if (i >= length - 1)
		{
			continue;
		}

		return blocks->elements[i + 1];
	}

	return NULL;
}

CFBlock* generate_control_flow(IRNodeList* func_blocks, IRNode* block, CFBlock* predecessor)
{
	if (block == NULL)
	{
		println("Block is NULL...");
		exit(1);
	}
	
	IRNode* tail = get_tail(block);

	if (tail == NULL)
	{
		println("Failed to find block tail...");
		exit(1);
	}

	int already_analyzed = 0;

	CFBlock* cf_block = find_cf_block(block);
	
	if (cf_block == NULL)
	{
		cf_block = malloc(sizeof(CFBlock));

		cf_block->predecessors = setup_list();
		cf_block->successors = setup_list();
		cf_block->block = block;

		add_element_to_list(cf_blocks, cf_block);
	}
	else
	{
		already_analyzed = 1;
	}

	if (predecessor != NULL)
	{
		add_element_to_list(cf_block->predecessors, predecessor);
	}

	if (already_analyzed)
	{
		return cf_block;
	}

	if (tail->type == IR_NODE_GOTO)
	{
		CFBlock* successor = generate_control_flow(func_blocks, tail->go_to.block, cf_block);
		add_element_to_list(cf_block->successors, successor);
	}
	else if (tail->type == IR_NODE_BRANCH)
	{
		CFBlock* thenb = generate_control_flow(func_blocks, tail->branch.then_block, cf_block);
		CFBlock* elseb = generate_control_flow(func_blocks, tail->branch.else_block, cf_block);
		
		add_element_to_list(cf_block->successors, thenb);
		add_element_to_list(cf_block->successors, elseb);
	}
	else if (tail->type != IR_NODE_RET)
	{
		IRNode* next_block = find_next_block(func_blocks, block);
		CFBlock* successor = generate_control_flow(func_blocks, next_block, cf_block);

		add_element_to_list(cf_block->successors, successor);
	}

	return cf_block;
}

void init_control_flow(IRNode* func)
{
	IRNode* entry = func->func.blocks->elements[0];

	generate_control_flow(func->func.blocks, entry, NULL);
}