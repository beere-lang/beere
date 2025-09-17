#include "llvm-gen.h"
#include "llvm-c/Core.h"
#include "llvm-c/Types.h"

#include <string.h>

#include "../../../utils/logger/logger.h"

static const char* ENTRY_NAME = ".entry";

static size_t expr_count = 0;

static LLVMValueRef generate_expr(const LLVMModuleRef module, const LLVMBuilderRef llvm, IRNode* node);

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

static const char* generate_expr_label(int inc)
{
	char buff[128];
	snprintf(buff, 128, ".expr_%d", (int) expr_count);

	expr_count += inc;

	return strdup(buff);
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
	
		default:
		{
			break;
		}
	}

	return NULL;
}

static void generate_llvm_from_node(const LLVMModuleRef module, const LLVMBuilderRef llvm, IRNode *node)
{
	switch (node->type)
	{
		case IR_NODE_FUNC:
		{
			generate_func(module, llvm, node);

			break;
		}

		default:
		{
			break;
		}
	}
}

void generate_module_llvm(IRNode* init) // adicionar o sistema de modulos depois
{
	const LLVMModuleRef module = LLVMModuleCreateWithName("test_module"); // implementar o bgl certo nisso
	const LLVMBuilderRef builder = LLVMCreateBuilder();

	generate_llvm_from_node(module, builder, init);
}