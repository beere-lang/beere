#include <stdlib.h>

#include "../../../utils/logger/logger.h"
#include "nodes.h"

ASTNode* create_node(ASTNodeType type)
{
	ASTNode* node = calloc(1, sizeof(ASTNode));

	if (node == NULL)
	{
		log_error("Failed to alloc memory for node...");
		exit(1);
	}

	node->type = type;

	return node;
}
