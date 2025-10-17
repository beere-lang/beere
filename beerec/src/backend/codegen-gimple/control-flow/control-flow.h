#ifndef CONTROL_FLOW_H
#define CONTROL_FLOW_H

typedef struct CFBlock CFBlock;

#include "../../../utils/list/list.h"
#include "../../structure/ir/ir-nodes.h"

typedef struct SizedArr SizedArr;

struct SizedArr
{
	int	   length;
	IRNode** elements;
};

struct CFBlock
{
	IRNode* block;

	DList*  predecessors;
	DList*  successors;

	int	  visited;
	int	  dt_index;
	int	  cf_index;
};

DList* init_control_flow(IRNode* func);

#endif
