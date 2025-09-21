#ifndef DOMINATOR_TREE_H
#define DOMINATOR_TREE_H

#include "../control-flow.h"

typedef struct DTBlock DTBlock;

struct DTBlock
{
	CFBlock* block;

	CFBlock* parent;
	DList* childs;
};

CFBlock* generate_dominator_tree(CFBlock* entry, int size);

#endif
