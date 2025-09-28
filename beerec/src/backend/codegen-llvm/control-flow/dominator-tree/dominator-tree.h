#ifndef DOMINATOR_TREE_H
#define DOMINATOR_TREE_H

#include "../control-flow.h"

typedef struct DTBlock DTBlock;
typedef struct DominatorTree DominatorTree;

struct DTBlock
{
	CFBlock* block;

	DTBlock* dominator;
	DList*   dominateds;
};

struct DominatorTree
{
	DList* blocks;
	int*   idominators;
};

DominatorTree* generate_dominator_tree(CFBlock* entry, int size);

#endif
