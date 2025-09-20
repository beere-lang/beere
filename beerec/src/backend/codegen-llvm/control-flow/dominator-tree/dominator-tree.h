#ifndef DOMINATOR_TREE_H
#define DOMINATOR_TREE_H

#include "../control-flow.h"

typedef struct DTBlock DTBlock;
typedef struct DTBlockList DTBlockList;
typedef struct CFPathBlock CFPathBlock;
typedef struct CFPathBlockList CFPathBlockList;

struct DTBlock
{
	CFBlock* block;
	DTBlockList* dominators;
};

struct DTBlockList
{
	DTBlock** elements;
	
	int length;
	int capacity;
};

struct CFPathBlockList
{
	CFPathBlock** elements;

	int length;
	int capacity;
};

struct CFPathBlock
{
	int paths;
	CFBlock* block;
};

DTBlock* setup_generate_dominator_tree(CFBlock* init);

#endif