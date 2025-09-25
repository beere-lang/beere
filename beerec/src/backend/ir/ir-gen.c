#include <stdlib.h>
#include <string.h>

#include "ir-gen.h"
#include "../../utils/logger/logger.h"

#define ENTRY_BLOCK_LABEL ".entry"
#define BLOCK_START_INSTRUCTIONS_CAPACITY 16

static IRNode* curr_func = NULL;
static IRNode* curr_block = NULL;

static int whiles_count = 0;

static Type* copy_type(Type* type);
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

	func->func.type = copy_type(node->function.return_type);

	func->func.params = NULL;

        if (node->function.params != NULL)
        {
                const int length = count_linked_list(node->function.params->head);

	        func->func.params = malloc(sizeof(IRNode*) * length);
	        func->func.params_size = length;

                setup_func_params(func, node->function.params->head, length);
        }

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
	char buff[64];
	snprintf(buff, 64, ".while_body_%d", whiles_count);

	IRNode* loopb = create_block(_strdup(buff), 1);

	IRNode* go_to = create_ir_node(IR_NODE_GOTO);
	go_to->go_to.block = loopb;

	add_element_to_list(curr_block->block.nodes, go_to);

	snprintf(buff, 64, ".while_post_%d", whiles_count);
	IRNode* postb = create_block(_strdup(buff), 0);

	IRNode* branch = create_ir_node(IR_NODE_BRANCH);

	curr_block = loopb;

	whiles_count++;

	generate_instructions_in_block(node->while_loop.then_block->block.statements->head, curr_block);

	branch->branch.condition = generate_expression(node->while_loop.condition);
	branch->branch.then_block = loopb;
	branch->branch.else_block = postb;

	add_element_to_list(curr_block->block.nodes, branch);

	add_element_to_list(curr_func->func.blocks, postb);

	curr_block = postb;

	return NULL;
}

static IRNode* generate_field(ASTNode* node)
{
	IRNode* field = create_ir_node(IR_NODE_FIELD);

	field->field.name = _strdup(node->declare.identifier);
	field->field.type = copy_type(node->declare.var_type);

	field->field.value = generate_expression(node->declare.default_value);

	return field;
}

static IRNode* generate_store(ASTNode* node)
{
	IRNode* store = create_ir_node(IR_NODE_STORE);

	store->store.dest = generate_expression(node->variable_assign.left);
	store->store.expr = generate_expression(node->variable_assign.expr);

	return store;
}

static IRNode* generate_operation(ASTNode* node)
{
	IRNode* operation = create_ir_node(IR_NODE_OPERATION);

	operation->operation.type = convert_op_type(node->operation.op);

	operation->operation.left = generate_expression(node->operation.left);
	operation->operation.right = generate_expression(node->operation.right);

	return operation;
}

static IRNode* generate_argument(ASTNode* node)
{
        IRNode* arg = create_ir_node(IR_NODE_ARGUMENT);
        arg->argument.value = generate_expression(node->argument.value);

        return arg;
}

static IRNode* generate_call(ASTNode* node)
{
	IRNode* call = create_ir_node(IR_NODE_CALL);
	call->call.func = generate_expression(node->function_call.callee);
	
        call->call.args = NULL;
	
        if (node->function_call.arguments != NULL)
        {
                call->call.args = create_list(8);

	        const int length = count_linked_list(node->function_call.arguments->head);
                ASTNode* curr = node->function_call.arguments->head;

	        for (int i = 0; i < length; i++)
	        {
		        if (curr == NULL)
		        {
			        continue;
		        }

		        IRNode* arg = generate_argument(curr);
		        add_element_to_list(call->call.args, curr);

		        curr = curr->next;
	        }
        }
        
	return call;
}

// ==---------------------------------- Core --------------------------------------== \\

/**
 * TODO: terminar de implementar todas as possiveis nodes.
 */
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

		case NODE_FIELD:
		{
			return generate_field(node);
		}

		case NODE_ASSIGN:
		{
			return generate_store(node);
		}

		case NODE_OPERATION:
		{
			return generate_operation(node);
		}

		case NODE_CALL:
		{
			return generate_call(node);
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
	literal->literal.type = copy_type(node->literal.literal_type);

	if (literal->literal.string_val != NULL)
	{
		literal->literal.string_val = _strdup(node->literal.string_value);
	}
	else
	{
		literal->literal.string_val = NULL;
	}

	literal->literal.double_val = node->literal.double_value;
	literal->literal.float_val  = node->literal.float_value;
	literal->literal.char_val   = node->literal.char_value;
	literal->literal.bool_val   = node->literal.bool_value;
	literal->literal.int_val    = node->literal.int_value;

	return literal;
}

static IRNode* generate_field_literal(ASTNode* node)
{
	IRNode* field_literal = create_ir_node(IR_NODE_FIELD_LITERAL);
	field_literal->field_literal.name = _strdup(node->variable.identifier);

	return field_literal;
}

static IRNode* generate_reference(ASTNode* node)
{
	IRNode* reference = create_ir_node(IR_NODE_REFERENCE);
	reference->reference.expr = generate_expression(node->adress_of.expr);

	return reference;
}

static IRNode* generate_dereference(ASTNode* node)
{
	IRNode* reference = create_ir_node(IR_NODE_DEREFERENCE);
	reference->reference.expr = generate_expression(node->dereference.expr);

	return reference;
}

static IRNode* generate_member_access(ASTNode* node)
{
	IRNode* maccess = create_ir_node(IR_NODE_MEMBER_ACCESS);

	maccess->member_access.object = generate_expression(node->member_access.object);
	maccess->member_access.member = _strdup(node->member_access.member_name);
	maccess->member_access.is_func = node->member_access.is_function;

	return maccess;
}

/**
 * TODO: terminar de implementar todas as possiveis nodes.
 */
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

		case NODE_IDENT:
		{
			return generate_field_literal(node);
		}

		case NODE_REFERENCE:
		{
			return generate_reference(node);
		}

		case NODE_DEREFERENCE:
		{
			return generate_dereference(node);
		}

		case NODE_CALL:
		{
			return generate_call(node);
		}

		case NODE_MEMBER_ACCESS:
		{
			return generate_member_access(node);
		}

		default:
		{
			println("Node with type id: %d, not implemented (expressions)...", node->type);
			exit(1);
		}
	}
}

// ==---------------------------------- Utils --------------------------------------== \\

static Type* copy_type(Type* type)
{
	Type* copy = malloc(sizeof(Type));

	copy->type = type->type;
	copy->class_name = (type->class_name != NULL) ? _strdup(type->class_name) : NULL;

	if (type->base != NULL)
	{
		copy->base = copy_type(type->base);
	}

	return copy;
}

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

	block->block.nodes = create_list(BLOCK_START_INSTRUCTIONS_CAPACITY);
	block->block.label = _strdup(label);

	if (add_to_func)
	{
		add_element_to_list(curr_func->func.blocks, block);
	}

	return block;
}

// ==---------------------------------- Memory Management --------------------------------------== \\

static void free_type(Type* type)
{
	if (type->class_name != NULL)
	{
		free(type->class_name);
	}

	if (type->base == NULL)
	{
		free_type(type->base);
		type->base = NULL;
	}

	free(type);
}

static void free_node(IRNode* node)
{
	switch (node->type)
	{
		case IR_NODE_FUNC:
		{
			free(node->func.name);
			free(node->func.name);

			free_type(node->func.type);

			if (node->func.params != NULL)
                        {
                                for (int i = 0; i < node->func.params_size; i++)
			        {
				        IRNode* param = node->func.params[i];

                                        if (param == NULL)
                                        {
                                                continue;
                                        }

				        free_node(param);
			        }
                        }

			const int length = node->func.blocks->length;

			for (int i = 0; i < length; i++)
			{
				IRNode* block = node->func.blocks->elements[i];

				if (block == NULL)
				{
					continue;
				}

				free_node(block);
				node->func.blocks->elements[i] = NULL;
			}

			free(node->func.blocks->elements);
			free(node->func.blocks);

			break;
		}

		case IR_NODE_BLOCK:
		{
			if (node->block.label != NULL)
			{
				free(node->block.label);
			}

			const int length = node->block.nodes->length;

			for (int i = 0; i < length; i++)
			{
				IRNode* block = node->block.nodes->elements[i];

				if (block == NULL)
				{
					continue;
				}

				free_node(block);
				node->block.nodes->elements[i] = NULL;
			}

			free(node->block.nodes->elements);
			free(node->block.nodes);

			break;
		}

		case IR_NODE_FIELD:
		{
			free(node->field.name);
			free_type(node->field.type);

			free_node(node->field.value);

			break;
		}

		case IR_NODE_RET:
		{
			free_node(node->ret.value);

			break;
		}

		case IR_NODE_BRANCH:
		{
			free_node(node->branch.condition);

			//free_node(node->branch.then_block); ALERT: o bloco deve ser free na propria função.
			//free_node(node->branch.else_block); ALERT: o bloco deve ser free na propria função.

			break;
		}

		case IR_NODE_GOTO:
		{
			//free_node(node->go_to.block); ALERT: o bloco deve ser free na propria função.

			break;
		}

		case IR_NODE_OPERATION:
		{
			free_node(node->operation.left);
			free_node(node->operation.right);

			break;
		}

		case IR_NODE_LITERAL:
		{
			free_type(node->literal.type);

			if (node->literal.string_val != NULL)
			{
				free(node->literal.string_val);
			}

			break;
		}

		case IR_NODE_FIELD_LITERAL:
		{
			free(node->field_literal.name);

			break;
		}

		case IR_NODE_STORE:
		{
			free_node(node->store.dest);
			free_node(node->store.expr);

			break;
		}

		case IR_NODE_CALL:
		{
			free_node(node->call.func);

			if (node->call.args != NULL)
                        {
                                const int length = node->call.args->length;

			        for (int i = 0; i < length; i++)
			        {
				        IRNode* arg = node->call.args->elements[i];

				        if (arg == NULL)
				        {
					        continue;
				        }

				        free_node(arg);
			        }
                        }

			break;
		}

		case IR_NODE_REFERENCE:
		{
			free_node(node->reference.expr);

			break;
		}

		case IR_NODE_DEREFERENCE:
		{
			free_node(node->dereference.expr);
		
			break;
		}

		case IR_NODE_MEMBER_ACCESS:
		{
			free_node(node->member_access.object);

			if (node->member_access.member != NULL)
			{
				free(node->member_access.member);
			}

			break;
		}

                case IR_NODE_ARGUMENT:
                {
                        free_node(node->argument.value);
                }

		default:
		{
			println("Invalid node while freeing with type id: %d...", node->type);
			exit(1);
		}
	}

	free(node);
}

void free_nodes(IRNode** nodes, int length)
{
	for (int i = 0; i < length; i++)
	{
		IRNode* node = nodes[i];

		free_node(node);
	}
}
