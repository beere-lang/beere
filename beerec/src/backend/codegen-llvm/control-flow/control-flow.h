#ifndef CONTROL_FLOW_H
#define CONTROL_FLOW_H

typedef struct CFBlock CFBlock;

#include "../../structure/ir/ir-nodes.h"
#include "../../../utils/list/list.h"

struct CFBlock
{
	IRNode* block;

	DList* predecessors;
	DList* successors;

	int visited;
};

CFBlock* generate_control_flow(IRNodeList* func_blocks, IRNode* block, CFBlock* predecessor);

#endif
