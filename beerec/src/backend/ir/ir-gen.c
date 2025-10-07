#include <stdlib.h>
#include <string.h>

#include "ir-gen.h"
#include "../../utils/logger/logger.h"

#define ENTRY_BLOCK_LABEL ".entry"
#define BLOCK_START_INSTRUCTIONS_CAPACITY 16
#define FUNC_START_BLOCKS_CAPACITY 8

#define CLASSES_START_FIELDS_CAPACITY 8
#define CLASSES_START_METHODS_CAPACITY 8

static IRNode* curr_func = NULL;
static IRNode* curr_block = NULL;

static int whiles_count = 0;
static int ifs_count = 0; // then & post
static int elses_count = 0; // elses

static Type* copy_type(Type* type);
static IRNode* copy_node(IRNode* node);
static int count_linked_list(ASTNode* head);
static IRNode* generate_ir_node(ASTNode* node);
static IRNode* generate_expression(ASTNode* node);
static void dump_node(IRNode* node, const int depth);
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

	func->func.is_static = node->function.is_static;

	func->func.params = NULL;

	if (node->function.params != NULL)
	{
		const int length = count_linked_list(node->function.params->head);

		func->func.params = malloc(sizeof(IRNode*) * length);
		func->func.params_size = length;

		setup_func_params(func, node->function.params->head, length);
	}

	func->func.blocks = create_list(FUNC_START_BLOCKS_CAPACITY);

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

static IRNode* generate_while(ASTNode* node)
{
	char buff[128];

	const int count = whiles_count++;
	
	snprintf(buff, 128, ".while_then_%d", count);
	const char* thenl = _strdup(buff);

	snprintf(buff, 128, ".while_post_%d", count);
	const char* postl = _strdup(buff);

	snprintf(buff, 128, ".while_cond_%d", count);
	const char* condl = _strdup(buff);

	IRNode* thenb = create_block(thenl, 1);
	IRNode* postb = create_block(postl, 1);
	IRNode* condb = create_block(condl, 1);

	IRNode* branch = create_ir_node(IR_NODE_BRANCH);
	branch->branch.condition = generate_expression(node->while_loop.condition);
	branch->branch.then_block = thenb;
	branch->branch.else_block = postb;

	IRNode* loops = create_ir_node(IR_NODE_GOTO);
	loops->go_to.block = condb;
	
	IRNode* loopl = create_ir_node(IR_NODE_GOTO);
	loopl->go_to.block = condb;

	add_element_to_list(curr_block->block.nodes, loops);

	curr_block = condb;

	add_element_to_list(curr_block->block.nodes, branch);

	curr_block = thenb;
	
	generate_instructions_in_block(node->while_loop.then_block->block.statements->head, curr_block);
	add_element_to_list(curr_block->block.nodes, loopl);
	
	curr_block = postb;

	return NULL;
}

static IRNode* generate_if(ASTNode* node)
{
	char buff[128];
	
	const int count = ifs_count++;

	snprintf(buff, 128, ".if_post_%d", count);
	const char* postl = _strdup(buff);
	
	IRNode* bpost = create_block(postl, 1);

	IRNode* branch = create_ir_node(IR_NODE_BRANCH);
	branch->branch.then_block = NULL;
	branch->branch.else_block = NULL;
	branch->branch.condition = generate_expression(node->if_statement.condition_top);
	
	IRNode* go_to = create_ir_node(IR_NODE_GOTO); // usado só no then pra não ter double free
	go_to->go_to.block = bpost;

	IRNode* mblock = curr_block;

	{ // then block
		snprintf(buff, 128, ".if_then_%d", count);
		const char* thenl = _strdup(buff);
		
		IRNode* bthen = create_block(thenl, 1);
		
		curr_block = bthen;
		
		generate_instructions_in_block(node->if_statement.then_branch->block.statements->head, bthen);
		branch->branch.then_block = bthen;

		add_element_to_list(curr_block->block.nodes, go_to); // post
		add_element_to_list(mblock->block.nodes, branch);
	}

	IRNode* go_to_ = create_ir_node(IR_NODE_GOTO);
	go_to_->go_to.block = NULL;
	
	if (node->if_statement.else_branch != NULL) // elses (else if, else)
	{
		ASTNode* currelse = node->if_statement.else_branch;
		
		while (currelse != NULL && currelse->type != NODE_BLOCK)
		{
			const int elsec = elses_count++;
			
			snprintf(buff, 128, ".if_else_%d", elsec);
			const char* elsel = _strdup(buff);
			
			IRNode* belseif = create_block(elsel, 1);
			
			curr_block = belseif;
		
			IRNode* elseifbranch = create_ir_node(IR_NODE_BRANCH);
			elseifbranch->branch.then_block = belseif;
			elseifbranch->branch.else_block = NULL;
			elseifbranch->branch.condition = generate_expression(currelse->if_statement.condition_top);
			
			generate_instructions_in_block(currelse->if_statement.then_branch->block.statements->head, belseif);
			add_element_to_list(curr_block->block.nodes, copy_node(go_to)); // post

			currelse = currelse->if_statement.else_branch;

			add_element_to_list(mblock->block.nodes, elseifbranch);
		}
		
		if (currelse != NULL)
		{
			const int elsec = elses_count++;
			
			snprintf(buff, 128, ".if_else_%d", elsec);
			const char* elsel = _strdup(buff);
			
			IRNode* belse = create_block(elsel, 1);
			
			curr_block = belse;

			generate_instructions_in_block(currelse->block.statements->head, belse);
			add_element_to_list(curr_block->block.nodes, copy_node(go_to)); // post

			go_to_->go_to.block = belse;
			add_element_to_list(mblock->block.nodes, go_to_);
		}
		else // non else
		{
			go_to_->go_to.block = bpost;
			add_element_to_list(mblock->block.nodes, go_to_); // post
		}
	}
	else // non else
	{
		go_to_->go_to.block = bpost;
		add_element_to_list(mblock->block.nodes, go_to_); // post
	}

	curr_block = bpost;

	return NULL;
}

static IRNode* generate_field(ASTNode* node)
{
	IRNode* field = create_ir_node(IR_NODE_FIELD);

	field->field.name = _strdup(node->declare.identifier);
	field->field.type = copy_type(node->declare.var_type);

	field->field.value = generate_expression(node->declare.default_value);

	field->field.is_static = node->declare.is_static;

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

	operation->operation.operation = convert_op_type(node->operation.op);
	operation->operation.type = copy_type(node->operation.type);

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

static IRNode* generate_class(ASTNode* node)
{
	IRNode* class = create_ir_node(IR_NODE_CLASS);

	const unsigned int fields_length = node->class_node.fields_count;
	class->class.fields = create_list(CLASSES_START_FIELDS_CAPACITY);

	for (int i = 0; i < fields_length; i++)
	{
		ASTNode* field = node->class_node.fields[i];

		if (field == NULL)
		{
			continue;
		}

		add_element_to_list(class->class.fields, generate_ir_node(field));
	}

	class->class.constructor = NULL;
	ASTNode* constructor = node->class_node.constructor;

	if (constructor != NULL)
	{
		class->class.constructor = generate_ir_node(constructor);
	}

	const unsigned int methods_length = node->class_node.funcs_count;
	class->class.methods = create_list(CLASSES_START_METHODS_CAPACITY);

	for (int i = 0; i < methods_length; i++)
	{
		ASTNode* method = node->class_node.funcs[i];

		if (method == NULL)
		{
			continue;
		}

		add_element_to_list(class->class.methods, generate_ir_node(method));
	}

	return class;
}

// ==---------------------------------- Core --------------------------------------== \\

/**
 * TODO: terminar de implementar as nodes que faltam (continue, break, create inst, static access, arr access, arr literal, for loop, switch)
 */
static IRNode* generate_ir_node(ASTNode* node)
{
	if (node == NULL)
	{
		return NULL;
	}
	
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

		case NODE_IF:
		{
			return generate_if(node);
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

		case NODE_CLASS:
		{
			return generate_class(node);
		}

		default:
		{
			println("Node with type id: %d, not implemented...", node->type);
			exit(1);
		}
	}
}

IRNode** generate_ir_nodes(ASTNode** nodes, const unsigned int length)
{
	IRNode** irs = malloc(sizeof(IRNode*) * length);

	for (int i = 0; i < length; i++)
	{
		irs[i] = generate_ir_node(nodes[i]);
	}

	dump_module(irs, length); // debug

	return irs;
}

// ==---------------------------------- Expressions --------------------------------------== \\

static IRNode* generate_literal(ASTNode* node)
{
	IRNode* literal = create_ir_node(IR_NODE_LITERAL);
	literal->literal.type = copy_type(node->literal.literal_type);

	if (literal->literal.string_val != NULL && literal->literal.type->type == TYPE_STRING)
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
 * TODO: adicionar o from logo no frotend, pra ja pegar direto aqui
 */
static IRNode* generate_cast(ASTNode* node)
{
	IRNode* cast = create_ir_node(IR_NODE_CAST);

	cast->cast.to = copy_type(node->cast_node.cast_type);
	//cast->cast.from = analyzer_return_type_of_expression(NULL, node->cast_node.expr, NULL, NULL, 0, NULL);

	cast->cast.expr = generate_expression(node->cast_node.expr);

	return cast;
}

static IRNode* generate_this(ASTNode* node)
{
	IRNode* this = create_ir_node(IR_NODE_THIS);

	return this;
}

static IRNode* generate_super(ASTNode* node)
{
	IRNode* super = create_ir_node(IR_NODE_SUPER);

	return super;
}

static IRNode* generate_arr_access(ASTNode* node)
{
	IRNode* arr_access = create_ir_node(IR_NODE_ARR_ACCESS);

	arr_access->arr_access.arr = generate_expression(node->acess_array.array);
	arr_access->arr_access.index = generate_expression(node->acess_array.index_expr);

	return arr_access;
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

		case NODE_CAST:
		{
			return generate_cast(node);
		}

		case NODE_THIS:
		{
			return generate_this(node);
		}

		case NODE_SUPER:
		{
			return generate_super(node);
		}

		case NODE_ARR_ACCESS:
		{
			return generate_arr_access(node);
		}

		default:
		{
			println("Node with type id: %d, not implemented (expressions)...", node->type);
			exit(1);
		}
	}
}

// ==---------------------------------- Utils --------------------------------------== \\

static IRNode* copy_node(IRNode* node)
{
	if (node == NULL)
	{
		exit(1);
	}
	
	IRNode* copy = malloc(sizeof(IRNode));
	memcpy(copy, node, sizeof(IRNode));

	return copy;
}

static Type* copy_type(Type* type)
{
	if (type == NULL)
	{
		return NULL;
	}
	
	Type* copy = malloc(sizeof(Type));

	copy->type = type->type;
	copy->class_name = (type->class_name != NULL) ? _strdup(type->class_name) : NULL;

	copy->base = NULL;

	copy->base = copy_type(type->base);

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
	block->block.phis = NULL;

	if (add_to_func)
	{
		add_element_to_list(curr_func->func.blocks, block);
	}

	return block;
}

// ==---------------------------------------- Debug --------------------------------------------== \\

static void get_type_str(Type* type, char* buff)
{
	if (type == NULL)
	{
		return;
	}
	
	get_type_str(type->base, buff);
	char* text = malloc(256);
	
	switch (type->type)
	{
		case TYPE_INT:
		{
			text = _strdup("int");

			break;
		}

		case TYPE_FLOAT:
		{
			text = _strdup("float");

			break;
		}
		
		case TYPE_DOUBLE:
		{
			text = _strdup("double");

			break;
		}

		case TYPE_CHAR:
		{
			text = _strdup("char");

			break;
		}

		case TYPE_STRING:
		{
			text = _strdup("string");

			break;
		}

		case TYPE_ARRAY:
		{
			text = _strdup("[]");

			break;
		}

		case TYPE_BOOL:
		{
			text = _strdup("bool");

			break;
		}

		case TYPE_PTR:
		{
			text = _strdup("*");

			break;
		}

		case TYPE_CLASS:
		{
			snprintf(text, 256, "Class (%s)", type->class_name);

			break;
		}

		case TYPE_VOID:
		{
			text = _strdup("void");

			break;
		}

		default:
		{
			break;
		}
	}

	snprintf(buff, 128, "%s%s", buff, text);
	free(text);
}

static char* type_to_str(Type* type)
{
	char buff[512] = "";
	get_type_str(type, buff);

	return _strdup(buff);
}

static void print_depth(int depth)
{
	for (int i = 0; i < depth; i++)
	{
		printf("	");
	}
}

static void dump_func(IRNode* func, int depth)
{
	print_depth(depth);
	printf("Func %s - %s:\n", func->func.name, type_to_str(func->func.type));

	const unsigned int blocks_length = func->func.blocks->length;

	for (int i = 0; i < blocks_length; i++)
	{
		IRNode* block = func->func.blocks->elements[i];

		if (block == NULL)
		{
			continue;
		}

		dump_node(block, depth + 1);
	}

	printf("\n"); // pular linha
}

static void dump_block(IRNode* block, int depth)
{
	print_depth(depth);
	printf("%s:\n", block->block.label);

	const unsigned int nodes_length = block->block.nodes->length;
	
	for (int i = 0; i < nodes_length; i++)
	{
		IRNode* node = block->block.nodes->elements[i];

		if (node == NULL)
		{
			continue;
		}

		dump_node(node, depth + 1);
	}
}

static void dump_node(IRNode* node, const int depth)
{
	switch (node->type)
	{
		case IR_NODE_FUNC:
		{
			dump_func(node, depth);

			break;
		}

		case IR_NODE_BLOCK:
		{
			dump_block(node, depth);

			break;
		}

		default:
		{
			print_depth(depth);
			printf("Node: %d\n", node->type);

			break;
		}
	}
}

/**
 * TODO: adicionar classes aqui
 * TODO: implementar isso com modulos reais quando implementar modulos no ir-gen
 * 
 * WARNING: não muito util, mas ja ajuda no debug
 */
void dump_module(IRNode** nodes, const unsigned int nodes_length)
{
	for (int i = 0; i < nodes_length; i++)
	{
		IRNode* node = nodes[i];

		if (node == NULL)
		{
			continue;
		}

		dump_node(node, 0);
	}
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

			break;
		}

		case IR_NODE_PARAM:
		{
			free(node->param.name);
			free_type(node->param.type);

			break;
		}
		
		case IR_NODE_CLASS:
		{
			if (node->class.constructor != NULL)
			{
				free_node(node->class.constructor);
			}

			const unsigned int fields_length = node->class.fields->length;

			for (int i = 0; i < fields_length; i++)
			{
				IRNode* field = node->class.fields->elements[i];

				if (field == NULL)
				{
					continue;
				}

				free_node(field);
			}

			const unsigned int methods_length = node->class.methods->length;

			for (int i = 0; i < fields_length; i++)
			{
				IRNode* method = node->class.methods->elements[i];

				if (method == NULL)
				{
					continue;
				}

				free_node(method);
			}

			break;
		}

		case IR_NODE_SUPER:
		{
			break;
		}

		case IR_NODE_THIS:
		{
			break;
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
