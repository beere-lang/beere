#include "llvm-gen.h"
#include "llvm-c/Core.h"
#include "llvm-c/Types.h"

#include <string.h>

#include "../../../utils/logger/logger.h"

static const char* ENTRY_NAME = ".entry";
static const int FIELD_TABLE_DEFAULT_START_CAPACITY = 4;

static size_t expr_count = 0;
static size_t fload_count = 0;

static LLVMFieldTable* field_table = NULL;

static LLVMValueRef generate_expr(const LLVMModuleRef module, const LLVMBuilderRef llvm, IRNode* node);

static void setup_field_table()
{
	field_table = malloc(sizeof(LLVMFieldTable));

	field_table->length = 0;
	field_table->capacity = FIELD_TABLE_DEFAULT_START_CAPACITY;

	field_table->fields = malloc(sizeof(LLVMValueRef) * FIELD_TABLE_DEFAULT_START_CAPACITY);
	field_table->names = malloc(sizeof(char*) * FIELD_TABLE_DEFAULT_START_CAPACITY);
	field_table->types = malloc(sizeof(char*) * FIELD_TABLE_DEFAULT_START_CAPACITY);
}

static void add_field_to_table(const LLVMValueRef field, const char* name, Type* type)
{
	if (field_table == NULL)
	{
		return;
	}

	if (field_table->capacity <= field_table->length)
	{
		field_table->capacity *= 2;

		field_table->fields = realloc(field_table->fields, sizeof(LLVMValueRef) * field_table->capacity);
		field_table->names = realloc(field_table->names, sizeof(char*) * field_table->capacity);
		field_table->types = realloc(field_table->types, sizeof(Type*) * field_table->capacity);
	}

	field_table->fields[field_table->length] = field;
	field_table->names[field_table->length] = _strdup(name);
	field_table->types[field_table->length] = type;

	field_table->length++;
}

static LLVMValueRef get_field_from_table(const char* name)
{
	if (field_table == NULL)
	{
		return NULL;
	}

	for (int i = 0; i < field_table->length; i++)
	{
		char* field_name = field_table->names[i];
		LLVMValueRef field = field_table->fields[i];

		if (field == NULL || name == NULL)
		{
			continue;
		}

		if (strcmp(name, field_name) != 0)
		{
			continue;
		}

		return field;
	}

	return NULL;
}

static Type* get_field_type_from_table(const char* name)
{
	if (field_table == NULL)
	{
		return NULL;
	}

	for (int i = 0; i < field_table->length; i++)
	{
		const char* field_name = field_table->names[i];

		LLVMValueRef field = field_table->fields[i];
		Type* type = field_table->types[i];

		if (field == NULL || name == NULL)
		{
			continue;
		}

		if (strcmp(name, field_name) != 0)
		{
			continue;
		}

		return type;
	}

	return NULL;
}

static const char* generate_expr_label(int inc)
{
	char buff[128];
	snprintf(buff, 128, ".expr_%d", (int) expr_count);

	expr_count += inc;

	return _strdup(buff);
}

static const char* generate_field_load_label(int inc)
{
	char buff[128];
	snprintf(buff, 128, ".fieldl%d", (int) fload_count);

	fload_count += inc;

	return _strdup(buff);
}

static LLVMTypeRef get_type(Type* type)
{
	switch (type->type)
	{
		case TYPE_FLOAT:
		{
			return LLVMFloatType();
		}

		case TYPE_DOUBLE:
		{
			return LLVMDoubleType();
		}

		case TYPE_STRING:
		{
			return LLVMPointerType(LLVMInt8Type(), 0); // sei la porra, muda isso depois
		}

		case TYPE_PTR:
		{
			return LLVMPointerType(get_type(type->base), 0);
		}

		case TYPE_INT:
		{
			return LLVMInt32Type();
		}

		default:
		{
			return NULL;
		}
	}
}

static LLVMTypeRef* get_args_type(IRNode** args, unsigned int args_size)
{
	LLVMTypeRef* args_type = malloc(sizeof(LLVMTypeRef) * args_size);

	int j = 0;

	for (int i = 0; i < args_size; i++)
	{
		IRNode* arg = args[i];

		if (arg == NULL)
		{
			continue;
		}

		args_type[j] = get_type(arg->field.type);
		j++;
	}

	return args_type;
}

static void setup_func_arg_names(LLVMValueRef* func, IRNode** args, unsigned int args_size)
{
	for (int i = 0; i < args_size; i++)
	{
		IRNode* arg = args[i];
		LLVMValueRef arg_ref = LLVMGetParam(*func, i);

		if (arg == NULL || arg_ref)
		{
			continue;
		}

		LLVMSetValueName(arg_ref, arg->field.name);
	}
}

static void generate_func(const LLVMModuleRef module, const LLVMBuilderRef llvm, IRNode* node)
{
	LLVMTypeRef return_type = get_type(node->func.type);

	unsigned int args_size = node->func.args_size;
	LLVMTypeRef* args_type = get_args_type(node->func.args, args_size);

	LLVMTypeRef type = LLVMFunctionType(return_type, args_type, args_size, 0);
	LLVMValueRef func = LLVMAddFunction(module, node->func.name, type);

	setup_func_arg_names(&func, node->func.args, args_size);

	LLVMBasicBlockRef entry = LLVMAppendBasicBlock(func, ENTRY_NAME);

	LLVMPositionBuilderAtEnd(llvm, entry);

	/**
	 * TODO: implementar os multiplos blocos em funções aqui
	 */
	/*
	int length = node->func->block.nodes->length;
	IRNode** nodes = node->func->block.nodes->elements;

	for (int i = 0; i < length; i++)
	{
		IRNode* node = nodes[i];

		if (node == NULL)
		{
			continue;
		}

		generate_llvm_from_node(module, llvm, node);
	}
	*/

	LLVMDumpModule(module);
}

static LLVMValueRef generate_literal_i32(const LLVMModuleRef module, const LLVMBuilderRef llvm, const int value)
{
	const LLVMValueRef result = LLVMConstInt(LLVMInt32Type(), value, 1); /** TODO: depois testar esse sign extend */

	return result;
}

static LLVMValueRef generate_literal_f32(const LLVMModuleRef module, const LLVMBuilderRef llvm, const float value)
{
	const LLVMValueRef result = LLVMConstReal(LLVMFloatType(), value);

	return result;
}

static LLVMValueRef generate_literal_d64(const LLVMModuleRef module, const LLVMBuilderRef llvm, const float value)
{
	const LLVMValueRef result = LLVMConstReal(LLVMDoubleType(), value);

	return result;
}

static LLVMValueRef generate_literal(const LLVMModuleRef module, const LLVMBuilderRef llvm, IRNode* node)
{
	switch (node->literal.type->type)
	{
		case TYPE_INT:
		{
			return generate_literal_i32(module, llvm, node->literal.int_val);
		}

		case TYPE_FLOAT:
		{
			return generate_literal_f32(module, llvm, node->literal.float_val);
		}

		case TYPE_DOUBLE:
		{
			return generate_literal_d64(module, llvm, node->literal.double_val);
		}

		default:
		{
			println("Found literal node with type not implemented: %d", node->literal.type->type);

			break;
		}
	}

	return NULL;
}

static LLVMValueRef generate_iadd_operation(const LLVMModuleRef module, const LLVMBuilderRef llvm, const LLVMValueRef left, const LLVMValueRef right)
{
	const LLVMValueRef result = LLVMBuildAdd(llvm, left, right, generate_expr_label(1));

	return result;
}

static LLVMValueRef generate_isub_operation(const LLVMModuleRef module, const LLVMBuilderRef llvm, const LLVMValueRef left, const LLVMValueRef right)
{
	const LLVMValueRef result = LLVMBuildSub(llvm, left, right, generate_expr_label(1));

	return result;
}

static LLVMValueRef generate_imul_operation(const LLVMModuleRef module, const LLVMBuilderRef llvm, const LLVMValueRef left, const LLVMValueRef right)
{
	const LLVMValueRef result = LLVMBuildMul(llvm, left, right, generate_expr_label(1));

	return result;
}

static LLVMValueRef generate_idiv_operation(const LLVMModuleRef module, const LLVMBuilderRef llvm, const LLVMValueRef left, const LLVMValueRef right)
{
	const LLVMValueRef result = LLVMBuildSDiv(llvm, left, right, generate_expr_label(1));

	return result;
}

static LLVMValueRef generate_operation(const LLVMModuleRef module, const LLVMBuilderRef llvm, IRNode* node)
{
	const LLVMValueRef left = generate_expr(module, llvm, node->operation.left);
	const LLVMValueRef right = generate_expr(module, llvm, node->operation.right);

	switch (node->operation.type) /** TODO: implementar coisa pra floating numbers (float, double) */
	{
		case IR_OPERATION_ADD:
		{
			return generate_iadd_operation(module, llvm, left, right);
		}

		case IR_OPERATION_SUB:
		{
			return generate_isub_operation(module, llvm, left, right);
		}

		case IR_OPERATION_MUL:
		{
			return generate_imul_operation(module, llvm, left, right);
		}

		case IR_OPERATION_DIV:
		{
			return generate_idiv_operation(module, llvm, left, right);
		}

		default:
		{
			println("Found operation node with type not implemented: %d", node->operation.type);

			break;
		}
	}

	return NULL;
}

static LLVMValueRef generate_field_literal(const LLVMModuleRef module, const LLVMBuilderRef llvm, IRNode* node)
{
	const LLVMValueRef field = get_field_from_table(node->field_literal.name);
	Type* type = get_field_type_from_table(node->field_literal.name);

	if (field == NULL || type == NULL)
	{
		return NULL;
	}

	const char* fload_name = generate_field_load_label(1);
	const LLVMTypeRef ftype = get_type(type);

	const LLVMValueRef value = LLVMBuildLoad2(llvm, ftype, field, fload_name);

	return value;
}

static LLVMValueRef generate_expr(const LLVMModuleRef module, const LLVMBuilderRef llvm, IRNode* node)
{
	switch (node->type)
	{
		case IR_NODE_OPERATION:
		{
			return generate_operation(module, llvm, node);
		}

		case IR_NODE_LITERAL:
		{
			return generate_literal(module, llvm, node);
		}

		case IR_NODE_FIELD_LITERAL:
		{
			return generate_field_literal(module, llvm, node);
		}

		default:
		{
			break;
		}
	}

	return NULL;
}

static void generate_field(const LLVMModuleRef module, const LLVMBuilderRef llvm, IRNode* node)
{
	LLVMTypeRef type = get_type(node->field.type);
	LLVMValueRef field = LLVMBuildAlloca(llvm, type, node->field.name);

	add_field_to_table(field, node->field.name, node->field.type);

	if (node->field.value == NULL)
	{
		return;
	}

	LLVMValueRef value = generate_expr(module, llvm, node->field.value);

	LLVMBuildStore(llvm, value, field);
}

static void generate_llvm_from_node(const LLVMModuleRef module, const LLVMBuilderRef llvm, IRNode* node)
{
	switch (node->type)
	{
		case IR_NODE_FUNC:
		{
			generate_func(module, llvm, node);

			break;
		}

		case IR_NODE_FIELD:
		{
			generate_field(module, llvm, node);
		}

		default:
		{
			break;
		}
	}
}

/**
 * TODO: adicionar o sistema de modulos depois
 */
void generate_module_llvm(IRNode* init)
{
	setup_field_table();

	const LLVMModuleRef module = LLVMModuleCreateWithName("test_module"); /** TODO: implementar o negocio certo nisso */
	const LLVMBuilderRef builder = LLVMCreateBuilder();

	generate_llvm_from_node(module, builder, init);
}
