#ifndef CONTROL_FLOW_H
#define CONTROL_FLOW_H

typedef struct CFBlock CFBlock;
typedef struct CFBlockList CFBlockList;

#include "../../structure/ir/ir-nodes.h"

struct CFBlockList
{
	CFBlock** elements;

	int length;
	int capacity;
};

struct CFBlock
{
	IRNode* block;
	
	CFBlockList* predecessors;
	CFBlockList* successors;
};

CFBlock* generate_control_flow(IRNodeList* func_blocks, IRNode* block, CFBlock* predecessor);

#endif