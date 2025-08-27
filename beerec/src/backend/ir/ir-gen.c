#include <stdlib.h>
#include <string.h>

#include "ir-gen.h"


static IROperationType convert_op_type(TokenType token)
{
	switch (token)
	{
		case TOKEN_OPERATOR_PLUS:
		{
			return IR_OPERATION_ADD;
		}

		case TOKEN_OPERATOR_MINUS:
		{
			return IR_OPERATION_SUB;
		}

		case TOKEN_CHAR_STAR:
		{
			return IR_OPERATION_MUL;
		}

		case TOKEN_OPERATOR_DIVIDED:
		{
			return IR_OPERATION_DIV;
		}

		default:
		{
			exit(1);
		}
	}
}

IRNode* generate_func(ASTNode* node)
{
	IRNode* ir = create_ir_node(IR_NODE_FUNC);
	
	ir->func.name = strdup(node->function.identifier);
	ir->func.type = node->function.return_type;
	
	ir->func.block = create_ir_node(IR_NODE_BLOCK);

	ir->func.block->block.nodes->elements = malloc(sizeof(IRNode*) * 4);
	ir->func.block->block.nodes->capacity = 4;
	ir->func.block->block.nodes->length = 0;

	ir->func.block = generate_ir_node(node->function.block);

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

IRNode* generate_operation(ASTNode* node)
{
	IRNode* ir = create_ir_node(IR_NODE_OPERATION);

	ir->operation.type = convert_op_type(node->operation.op);

	ir->operation.left = generate_ir_node(node->operation.left);
	ir->operation.right = generate_ir_node(node->operation.right);

	return ir;
}

IRNode* generate_literal(ASTNode* node)
{
	IRNode* ir = create_ir_node(IR_NODE_LITERAL);

	ir->literal.type = node->literal.literal_type;

	ir->literal.bool_val = node->literal.bool_value;
	ir->literal.char_val = node->literal.char_value;
	ir->literal.double_val = node->literal.double_value;
	ir->literal.float_val = node->literal.float_value;
	ir->literal.int_val = node->literal.int_value;
	ir->literal.string_val = node->literal.string_value;

	return ir;
}

IRNode* generate_if(ASTNode* node)
{
	IRNode* ir = create_ir_node(IR_NODE_IF);

	ir->if_statement.cond = generate_ir_node(node->if_statement.condition_top);
	
	ir->if_statement.then_block = generate_ir_node(node->if_statement.else_branch);
	ir->if_statement.else_block = generate_ir_node(node->if_statement.then_branch);

	return ir;
}

IRNode* generate_block(ASTNode* node)
{
	IRNode* ir = create_ir_node(IR_NODE_BLOCK);
	ASTNode* next = node->block.statements->head;

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

IRNode* generate_dereference(ASTNode* node)
{
	IRNode* ir = create_ir_node(IR_NODE_DEREFERENCE);

	ir->dereference.expr = generate_ir_node(node->dereference.expr);

	return ir;
}

IRNode* generate_reference(ASTNode* node)
{
	IRNode* ir = create_ir_node(IR_NODE_REFERENCE);

	ir->reference.expr = generate_ir_node(node->adress_of.expr);

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

		case NODE_OPERATION:
		{
			return generate_operation(node);
		}

		case NODE_LITERAL:
		{
			return generate_literal(node);
		}

		case NODE_IF:
		{
			return generate_if(node);
		}

		case NODE_BLOCK:
		{
			return generate_block(node);
		}

		case NODE_DEREFERENCE:
		{
			return generate_dereference(node);
		}

		case NODE_REFERENCE:
		{
			return generate_reference(node);
		}

		default:
		{
			exit(1);
		}
	}
}