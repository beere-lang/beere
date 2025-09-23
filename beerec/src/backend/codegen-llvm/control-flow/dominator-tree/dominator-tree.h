#ifndef DOMINATOR_TREE_H
#define DOMINATOR_TREE_H

#include "../control-flow.h"

typedef struct DTBlock DTBlock;

struct DTBlock
{
	CFBlock* block;

	DTBlock* dominator;
	DList*   dominateds;
};

DTBlock* generate_dominator_tree(CFBlock* entry, int size);

#endif
