#include <stdlib.h>

#include "dominator-tree.h"
#include "../control-flow.h"

static DTBlock* link_dominator(CFBlock* block, DTBlock* dominator);

#define DT_BLOCKS_LIST_DEFAULT_START_CAPACITY 4
#define NCA_LIST_DEFAULT_START_CAPACITY 4

static int length = 0;

static int* semi = NULL;
static int* idom = NULL;

static CFBlock** bparents = NULL;
static CFBlock** blocks = NULL;

static DList* dt_blocks = NULL;

// ==----------------------------- Dominator Tree -----------------------------== \\

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
			free(visited);
			return second;
		}

		second = bparents[second->dt_index];
	}

	free(visited);
	return NULL;
}

static void dfs_control_flow(CFBlock* block, CFBlock* parent)
{
	if (block == NULL || block->visited)
	{
		return;
	}

	bparents[length] = parent;
	blocks[length] = block;

	block->dt_index = length;
	block->visited = 1;

	length++;

	const unsigned int succs_length = block->successors->length;

	for (int i = 0; i < succs_length; i++)
	{
		CFBlock* succ = block->successors->elements[i];

		dfs_control_flow(succ, block);
	}
}

static void get_semi_dominators()
{
	for (int i = length - 1; i > 0; i--)
	{
		CFBlock* block = blocks[i];
		const unsigned int preds_length = block->predecessors->length;

		for (int j = 0; j < preds_length; j++)
		{
			CFBlock* pred = block->predecessors->elements[j];
			int index = pred->dt_index;

			int fetch = (index <= i) ? index : semi[index];

			if (fetch < semi[i])
			{
				semi[i] = fetch;
			}
		}
	}
}

static void get_real_dominators()
{
	for (int i = length - 1; i > 0; i--)
	{
		if (semi[i] == 0)
		{
			continue;
		}
		
		CFBlock* block = blocks[i];
		idom[i] = semi[i];

		const unsigned int preds_length = block->predecessors->length;

		int min_semi = semi[idom[i]];
		
		for (int j = 0; j < preds_length; j++)
		{
			CFBlock* pred = block->predecessors->elements[j];
			CFBlock* idm = nca_blocks(block, pred);

			if (idm == NULL)
			{
				continue;
			}

			const int index = idm->dt_index;
			const int idm_semi = semi[index];

			if (idm_semi < min_semi)
			{
				min_semi = idm_semi;
				idom[i] = index;
			}
		}
	}
}

static DTBlock* build_dominator_tree()
{
	return link_dominator(blocks[0], NULL);
}

// ==----------------------------- Core -----------------------------== \\

static void setup_data(const unsigned int size)
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

static void free_data()
{
	length = 0;

	free(bparents);

	free(semi);
	free(idom);

	free(dt_blocks);
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

// ==----------------------------- Utils -----------------------------== \\

static DTBlock* create_dt_block(CFBlock* block, DTBlock* dominator)
{
	DTBlock* dt_block = malloc(sizeof(DTBlock));

	dt_block->dominateds = create_list(8);

	dt_block->block = block;
	dt_block->dominator = dominator;

	return dt_block;
}

static DTBlock* link_dominator(CFBlock* block, DTBlock* dominator)
{
	const int index = block->dt_index;
	
	DTBlock* dt_block = create_dt_block(block, dominator);
	add_element_to_list(dt_blocks, dt_block);

	for (int i = 0; i < length; i++)
	{
		if (idom[i] != index)
		{
			continue;
		}

		DTBlock* dominated = link_dominator(blocks[i], dt_block);
		add_element_to_list(dt_block->dominateds, dominated);
	}

	return dt_block;
}