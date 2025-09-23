#include <stdlib.h>

#include "control-flow.h"
#include "../../../utils/logger/logger.h"
#include "../../../utils/list/list.h"

#define CF_BLOCK_LIST_DEFAULT_START_CAPACITY 4

DList* cf_blocks = NULL;

void setup_cf_blocks()
{
	cf_blocks = create_list(CF_BLOCK_LIST_DEFAULT_START_CAPACITY);
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

SizedArr* get_flow_cases(IRNode* block)
{
	IRNode** list = malloc(sizeof(IRNode*) * block->block.nodes->length);

	const int length = block->block.nodes->length;
	int count = 0;

	for (int i = 0; i < length; i++)
	{
		IRNode* curr = block->block.nodes->elements[i];

		if (curr->type != IR_NODE_GOTO && curr->type != IR_NODE_BRANCH && curr->type != IR_NODE_RET)
		{
			continue;
		}

		list[count++] = curr;
	}

	SizedArr* arr = malloc(sizeof(SizedArr));

	arr->elements = list;
	arr->length = count;

	return arr;
}

IRNode* find_next_block(DList* blocks, IRNode* block)
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

CFBlock* generate_control_flow(DList* func_blocks, IRNode* block, CFBlock* predecessor)
{
	if (block == NULL)
	{
		println("Block is NULL...");
		exit(1);
	}

	int already_analyzed = 0;

	CFBlock* cf_block = find_cf_block(block);

	if (cf_block == NULL)
	{
		cf_block = malloc(sizeof(CFBlock));

		cf_block->predecessors = create_list(CF_BLOCK_LIST_DEFAULT_START_CAPACITY);
		cf_block->successors = create_list(CF_BLOCK_LIST_DEFAULT_START_CAPACITY);
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

	SizedArr* flow_cases =  get_flow_cases(block);

	IRNode** jmps = flow_cases->elements;
	const int length = flow_cases->length;

	int next_block = 1;

	for (int i = 0; i < length; i++)
	{
		IRNode* jmp = jmps[i];

		switch (jmp->type)
		{
			case IR_NODE_GOTO:
			{
				CFBlock* successor = generate_control_flow(func_blocks, jmp->go_to.block, cf_block);
				add_element_to_list(cf_block->successors, successor);

				next_block = 0;

				break;
			}

			case IR_NODE_BRANCH:
			{
				CFBlock* thenb = generate_control_flow(func_blocks, jmp->branch.then_block, cf_block);
				CFBlock* elseb = generate_control_flow(func_blocks, jmp->branch.else_block, cf_block);

				add_element_to_list(cf_block->successors, thenb);
				add_element_to_list(cf_block->successors, elseb);

				break;
			}

			case IR_NODE_RET:
			{
				next_block = 0;

				break;
			}

			default:
			{
				exit(1);
			}
		}
	}

	if (next_block)
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
