#ifndef DOMINATOR_TREE_H
#define DOMINATOR_TREE_H

#include "../control-flow.h"

typedef struct DTBlock DTBlock;
typedef struct CFPathBlock CFPathBlock;

struct DTBlock
{
	CFBlock* block;
	DList* dominators;
};

struct CFPathBlock
{
	int paths;
	CFBlock* block;
};

DTBlock* setup_generate_dominator_tree(CFBlock* init);

#endif