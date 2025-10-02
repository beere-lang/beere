#include <stdlib.h>

#include "ir-nodes.h"

IRNode* create_ir_node(IRNodeType type)
{
	 IRNode* node = malloc(sizeof(IRNode));	
        
        node->type = type;
        
        return node;
}
