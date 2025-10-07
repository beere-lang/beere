#include <stdlib.h>

#include "ir-nodes.h"
#include "../../../utils/logger/logger.h"

IRNode* create_ir_node(IRNodeType type)
{
	IRNode* node = malloc(sizeof(IRNode));

	if (node == NULL)
	{
		println("Failed to alloc memory for IR Node...");
	}

	node->type = type;
	
	return node;
}
