#include <stdlib.h>

#include "dominator-tree.h"
#include "../control-flow.h"

size_t length = 0;

int* bindex = NULL;
int* semi = NULL;

CFBlock** bparents = NULL;
CFBlock** blocks = NULL;

static void setup_data(size_t size)
{
	length = 0;

	bindex = malloc(sizeof(int) * size);
	bparents = malloc(sizeof(CFBlock*) * size);
	blocks = malloc(sizeof(CFBlock*) * size);
	semi = malloc(sizeof(CFBlock*) * size);
}

static DTBlock* create_dt_block(CFBlock* block)
{
	DTBlock* dtb = malloc(sizeof(DTBlock));

	dtb->block = block;
	dtb->childs = create_list(4);

	return dtb;
}

static void dfs_control_flow(CFBlock* block, CFBlock* parent)
{
	if (block == NULL)
	{
		return;
	}

	size_t size = block->successors->length;

	block->visited = 1;

	bparents[length] = parent;
	blocks[length] = block;

	const int index = (length == 0) ? 0 : bindex[length - 1] + 1;

	bindex[length] = index;

	semi[length] = index;

	length++;

	DTBlock* dtb = create_dt_block(block);

	for (size_t i = 0; i < size; i++)
	{
		CFBlock* curr = block->successors->elements[i];

		if (curr->visited)
		{
			continue;
		}

		dfs_control_flow(curr, block);
	}
}

static int get_block_number(CFBlock* block)
{
	for (size_t i = 0; i < length; i++)
	{
		CFBlock* curr = blocks[i];

		if (block != curr)
		{
			continue;
		}

		return bindex[i];
	}

	return -1;
}

static void get_semi_dominators()
{
	for (int i = length - 1; i >= 2; i--)
	{
		CFBlock* block = blocks[i];
		const size_t s = block->predecessors->length;

		for (int j = 0; j < s; j++)
		{
			CFBlock* curr = block->predecessors->elements[j];

			int index = get_block_number(curr);

			if (index < i && index < semi[i])
			{
				semi[i] = index;
			}
			else if (index >= i && semi[index] < semi[i])
			{
				semi[i] = semi[index];
			}
		}
	}
}

static void get_real_dominators()
{

}

static void build_dominator_tree()
{

}

CFBlock* generate_dominator_tree(CFBlock* entry, size_t size)
{
	setup_data(size);

	dfs_control_flow(entry, NULL);

	get_semi_dominators();
	get_real_dominators();

	build_dominator_tree();

	return entry;
}
