#include <stdlib.h>

#include "dominator-tree.h"
#include "../control-flow.h"

#define DT_BLOCKS_LIST_DEFAULT_START_CAPACITY 4
#define NCA_LIST_DEFAULT_START_CAPACITY 4

int length = 0;

int* semi = NULL;
int* idom = NULL;

CFBlock** bparents = NULL;
CFBlock** blocks = NULL;

static void setup_data(int size)
{
	length = 0;

	bparents = malloc(sizeof(CFBlock*) * size);
	blocks = malloc(sizeof(CFBlock*) * size);
	semi = malloc(sizeof(int) * size);
	idom = malloc(sizeof(int) * size);

	for (int i = 0; i < size; i++)
	{
		idom[i] = 0;
		semi[i] = i;
	}
}

static DTBlock* create_dt_block(CFBlock* block)
{
	DTBlock* dtb = malloc(sizeof(DTBlock));

	dtb->block = block;
	dtb->childs = create_list(DT_BLOCKS_LIST_DEFAULT_START_CAPACITY);

	return dtb;
}

static int get_block_number(CFBlock* block)
{
	for (int i = 0; i < length; i++)
	{
		CFBlock* curr = blocks[i];

		if (block != curr)
		{
			continue;
		}

		return i;
	}

	return -1;
}

static void dfs_control_flow(CFBlock* block, CFBlock* parent)
{
	if (block == NULL)
	{
		return;
	}

	int size = block->successors->length;

	block->visited = 1;

	bparents[length] = parent;
	blocks[length] = block;

	semi[length] = length;

	length++;

	DTBlock* dtb = create_dt_block(block);

	for (int i = 0; i < size; i++)
	{
		CFBlock* curr = block->successors->elements[i];

		if (curr->visited)
		{
			continue;
		}

		dfs_control_flow(curr, block);
	}
}

static void get_semi_dominators()
{
	for (int i = length - 1; i >= 1; i--)
	{
		CFBlock* block = blocks[i];

		for (int j = 0; j < block->predecessors->length; j++)
		{
			CFBlock* pred = block->predecessors->elements[j];
			int index = get_block_number(pred);

			if (index < semi[i])
			{
				semi[i] = index;
			}
		}
	}
}

static CFBlock* nca_blocks(CFBlock* first, CFBlock* second)
{
	DList* visited = create_list(NCA_LIST_DEFAULT_START_CAPACITY);

	while (first != NULL && second != NULL)
	{

		if (first != NULL)
		{
			if (contains_element(visited, first))
			{
				return first;
			}

			first = bparents[get_block_number(first)];
		}

		if (second != NULL)
		{
			if (contains_element(visited, second))
			{
				return second;
			}

			second = bparents[get_block_number(second)];
		}
	}

	return NULL;
}

static void get_real_dominators()
{
	for (int i = length - 1; i >= 1; i--)
	{
		CFBlock* block = blocks[i];
		idom[i] = semi[i];

		for (int j = 0; j < block->predecessors->length; j++)
		{
			CFBlock* pred = block->predecessors->elements[j];
			CFBlock* idm = nca_blocks(block, pred);

			if (idm == NULL)
			{
				continue;
			}

			const int index = get_block_number(idm);

			if (idom[i] < index && idom[i] != semi[i])
			{
				continue;
			}

			idom[i] = index;
		}
	}
}

static CFBlock* build_dominator_tree()
{
	return NULL;
}

CFBlock* generate_dominator_tree(CFBlock* entry, int size)
{
	setup_data(size);

	dfs_control_flow(entry, NULL);

	get_semi_dominators();
	get_real_dominators();

	return build_dominator_tree();
}
