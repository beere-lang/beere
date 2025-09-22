#include <stdlib.h>
#include <string.h>

#include "ir-gen.h"
#include "../../utils/logger/logger.h"

#define ENTRY_BLOCK_LABEL ".entry"

static IRNode* curr_func = NULL;
static IRNode* curr_block = NULL;

static int count_linked_list(ASTNode* head);
static IRNode* generate_ir_node(ASTNode* node);
static IRNode* generate_expression(ASTNode* node);
static void generate_func_instructions(ASTNode* head);
static IROperationType convert_op_type(const TokenType type);
static IRNode* create_block(const char* label, const int add_to_func);
static void generate_instructions_in_block(ASTNode* head, IRNode* block);
static void setup_func_params(IRNode* func, ASTNode* head, const int length);

// ==---------------------------------- Statements --------------------------------------== \\

static IRNode* generate_func(ASTNode* node)
{
	IRNode* func = create_ir_node(IR_NODE_FUNC);

	char buff[64];
	snprintf(buff, 64, ".fn_%s", node->function.identifier);

	func->func.name = _strdup(buff);

	func->func.type = node->function.return_type;

	const int length = count_linked_list(node->function.params->head);

	func->func.params = malloc(sizeof(IRNode*) * length);
	func->func.params_size = length;

	setup_func_params(func, node->function.params->head, length);

	curr_func = func;

	IRNode* entry = create_block(ENTRY_BLOCK_LABEL, 1);

	curr_block = entry;

	generate_func_instructions(node->function.block->block.statements->head);

	return func;
}

static IRNode* generate_ret(ASTNode* node)
{
	IRNode* ret = create_ir_node(IR_NODE_RET);
	ret->ret.value = generate_expression(node->return_statement.return_value);

	return ret;
}

/**
 * TODO: implementar labels nisso.
 */
static IRNode* generate_while(ASTNode* node)
{
	IRNode* loopb = create_block(NULL, 1);

	IRNode* go_to = create_ir_node(IR_NODE_GOTO);
	go_to->go_to.block = loopb;

	add_element_to_list(curr_block->block.nodes, go_to);

	IRNode* postb = create_block(NULL, 0);

	IRNode* branch = create_ir_node(IR_NODE_BRANCH);

	curr_block = loopb;

	generate_instructions_in_block(node->while_loop.then_block->block.statements->head, curr_block);

	branch->branch.condition = generate_expression(node->while_loop.condition);
	branch->branch.then_block = loopb;
	branch->branch.else_block = postb;

	add_element_to_list(curr_block->block.nodes, branch);

	add_element_to_list(curr_func->func.blocks, postb);

	curr_block = postb;

	return NULL;
}

// ==---------------------------------- Core --------------------------------------== \\

static IRNode* generate_ir_node(ASTNode* node)
{
	switch (node->type)
	{
		case NODE_FUNC:
		{
			return generate_func(node);
		}

		case NODE_RET:
		{
			return generate_ret(node);
		}

		case NODE_WHILE_LOOP:
		{
			return generate_while(node);
		}

		default:
		{
			println("Node with type id: %d, not implemented...", node->type);
			exit(1);
		}
	}
}

IRNode** generate_ir_nodes(ASTNode** nodes, const int length)
{
	IRNode** irs = malloc(sizeof(IRNode*) * length);

	for (int i = 0; i < length; i++)
	{
		irs[i] = generate_ir_node(nodes[i]);
	}

	return irs;
}

// ==---------------------------------- Expressions --------------------------------------== \\

static IRNode* generate_literal(ASTNode* node)
{
	IRNode* literal = create_ir_node(IR_NODE_LITERAL);
	literal->literal.type = node->literal.literal_type;

	literal->literal.string_val = node->literal.string_value;
	literal->literal.double_val = node->literal.double_value;
	literal->literal.float_val  = node->literal.float_value;
	literal->literal.char_val   = node->literal.char_value;
	literal->literal.bool_val   = node->literal.bool_value;
	literal->literal.int_val    = node->literal.int_value;

	return literal;
}

static IRNode* generate_operation(ASTNode* node)
{
	IRNode* operation = create_ir_node(IR_NODE_OPERATION);

	operation->operation.type = convert_op_type(node->operation.op);

	operation->operation.left = generate_expression(node->operation.left);
	operation->operation.right = generate_expression(node->operation.right);

	return operation;
}

static IRNode* generate_expression(ASTNode* node)
{
	switch (node->type)
	{
		case NODE_LITERAL:
		{
			return generate_literal(node);
		}

		case NODE_OPERATION:
		{
			return generate_operation(node);
		}

		default:
		{
			println("Node with type id: %d, not implemented (expressions)...", node->type);
			exit(1);
		}
	}
}

// ==---------------------------------- Utils --------------------------------------== \\

static int count_linked_list(ASTNode* head)
{
	int count = 0;

	while (head != NULL)
	{
		head = head->next;
		count++;
	}

	return count;
}

static IRNode* generate_param(ASTNode* node)
{
	IRNode* param = create_ir_node(IR_NODE_PARAM);

	param->param.name = _strdup(node->param.identifier);
	param->param.type = node->param.argument_type;

	return param;
}

static void setup_func_params(IRNode* func, ASTNode* head, const int length)
{
	ASTNode* curr = head;

	for (int i = 0; i < length; i++)
	{
		func->func.params[i] = generate_param(curr);

		curr = curr->next;
	}
}

static void generate_func_instructions(ASTNode* head)
{
	ASTNode* curr = head;

	while (curr != NULL)
	{
		IRNode* backup = curr_block;
		IRNode* node = generate_ir_node(curr);

		if (node != NULL)
		{
			add_element_to_list(backup->block.nodes, node);
		}

		curr = curr->next;
	}
}

static void generate_instructions_in_block(ASTNode* head, IRNode* block)
{
	ASTNode* curr = head;

	while (curr != NULL)
	{
		add_element_to_list(block->block.nodes, generate_ir_node(curr));

		curr = curr->next;
	}
}

static IROperationType convert_op_type(const TokenType type)
{
	switch (type)
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

		case TOKEN_OPERATOR_PLUS_EQUALS:
		{
			return IR_OPERATION_ADD_EQUALS;
		}

		case TOKEN_OPERATOR_MINUS_EQUALS:
		{
			return IR_OPERATION_SUB_EQUALS;
		}

		case TOKEN_OPERATOR_TIMES_EQUALS:
		{
			return IR_OPERATION_MUL_EQUALS;
		}

		case TOKEN_OPERATOR_DIVIDED_EQUALS:
		{
			return IR_OPERATION_DIV_EQUALS;
		}

		case TOKEN_OPERATOR_GREATER:
		{
			return IR_OPERATION_GREATER;
		}

		case TOKEN_OPERATOR_LESS:
		{
			return IR_OPERATION_LESS;
		}

		case TOKEN_OPERATOR_EQUALS:
		{
			return IR_OPERATION_EQUALS;
		}

		case TOKEN_OPERATOR_NOT_EQUALS:
		{
			return IR_OPERATION_NOT_EQUALS;
		}

		case TOKEN_OPERATOR_GREATER_EQUALS:
		{
			return IR_OPERATION_GREATER_EQUALS;
		}

		case TOKEN_OPERATOR_LESS_EQUALS:
		{
			return IR_OPERATION_LESS_EQUALS;
		}

		default:
		{
			println("Invalid operation type: %d...", type);
			exit(1);
		}
	}
}

static IRNode* create_block(const char* label, const int add_to_func)
{
	IRNode* block = create_ir_node(IR_NODE_BLOCK);

	block->block.nodes = create_list(8);
	block->block.label = _strdup(label);

	if (add_to_func)
	{
		add_element_to_list(curr_func->func.blocks, block);
	}

	return block;
}
