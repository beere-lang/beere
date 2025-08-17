#include <stdlib.h>
#include <string.h>

#include "ir-gen.h"

IRNode* generate_func(ASTNode* node)
{
	IRNode* ir = create_ir_node(IR_NODE_FUNC);
	
	ir->func.name = strdup(node->function.identifier);
	ir->func.type = node->function.return_type;
	
	ir->func.block = create_ir_node(IR_NODE_BLOCK);

	ir->func.block->block.nodes->elements = malloc(sizeof(IRNode*) * 4);
	ir->func.block->block.nodes->capacity = 4;
	ir->func.block->block.nodes->length = 0;

	ASTNode* next = node->function.block->block.statements->head;
	
	while (next != NULL)
	{
		if (ir->block.nodes->length >= ir->block.nodes->capacity)
		{
			ir->block.nodes->capacity *= 2;
			ir->block.nodes->elements = realloc(ir->block.nodes->elements, sizeof(IRNode*) * ir->block.nodes->capacity);
		}

		ir->block.nodes->elements[ir->block.nodes->length++] = generate_ir_node(next);

		next = next->next;
	}

	return ir;
}

IRNode* generate_field(ASTNode* node)
{
	IRNode* ir = create_ir_node(IR_NODE_FIELD);
	
	ir->field.type = node->declare.var_type;
	ir->field.name = strdup(node->declare.identifier);

	ir->field.value = generate_ir_node(node->declare.default_value);

	return ir;
}

IRNode* generate_ir_node(ASTNode* node)
{
	switch (node->type)
	{
		case NODE_FUNC:
		{
			return generate_func(node);
		}

		case NODE_FIELD:
		{
			return generate_field(node);
		}

		default:
		{
			exit(1);
		}
	}
}