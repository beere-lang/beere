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

DList* dt_blocks = NULL;

static void setup_data(int size)
{
	length = 0;

	bparents = malloc(sizeof(CFBlock*) * size);
	blocks = malloc(sizeof(CFBlock*) * size);
	semi = malloc(sizeof(int) * size);
	idom = malloc(sizeof(int) * size);
	dt_blocks = create_list(DT_BLOCKS_LIST_DEFAULT_START_CAPACITY);

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
	dtb->dominator = NULL;
	dtb->dominateds = create_list(DT_BLOCKS_LIST_DEFAULT_START_CAPACITY);

	return dtb;
}

static void dfs_control_flow(CFBlock* block, CFBlock* parent)
{
	if (block == NULL)
	{
		return;
	}

	const int size = block->successors->length;

	block->visited = 1;

	bparents[length] = parent;
	blocks[length] = block;

	block->dt_index = length;

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
			int index = pred->dt_index;

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

	while (first != NULL)
	{
		add_element_to_list(visited, first);
		first = bparents[first->dt_index];
	}

	while (second != NULL)
	{
		if (contains_element(visited, second))
		{
			return second;
		}

		second = bparents[second->dt_index];
	}

	return NULL;
}

static void get_real_dominators()
{
	for (int i = length - 1; i >= 1; i--)
	{
		CFBlock* block = blocks[i];
		idom[i] = semi[i];

		if (idom[i] == 0)
		{
			continue;
		}

		for (int j = 0; j < block->predecessors->length; j++)
		{
			CFBlock* pred = block->predecessors->elements[j];
			CFBlock* idm = nca_blocks(block, pred);

			if (idm == NULL)
			{
				continue;
			}

			const int index = idm->dt_index;

			if (idom[i] < index && idom[i] != semi[i])
			{
				continue;
			}

			idom[i] = index;
		}
	}
}

static DTBlock* link_dominator(CFBlock* block, DTBlock* dominator)
{
	const int index = block->dt_index;
	DTBlock* dt = create_dt_block(block);

	add_element_to_list(dt_blocks, dt);

	dt->dominator = dominator;

	for (int i = 0; i < length; i++)
	{
		if (idom[i] != index)
		{
			continue;
		}

		add_element_to_list(dt->dominateds, blocks[i]);
		link_dominator(blocks[i], dt);
	}

	return dt;
}

static DTBlock* build_dominator_tree()
{
	return link_dominator(blocks[0], NULL);
}

DominatorTree* generate_dominator_tree(CFBlock* entry, int size)
{
	setup_data(size);

	dfs_control_flow(entry, NULL);

	get_semi_dominators();
	get_real_dominators();

	build_dominator_tree();

	DominatorTree* tree = malloc(sizeof(DominatorTree));
	
	tree->blocks = dt_blocks;
	tree->idominators = idom;

	return tree;
}
