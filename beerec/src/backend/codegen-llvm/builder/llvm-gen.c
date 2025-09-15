#include <cstdlib>
#include <string.h>

#include "llvm-gen.h"
#include "llvm-c/Core.h"
#include "llvm-c/Types.h"

static size_t expr_count = 0;

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
	LLVMTypeRef* args_type = malloc(sizeof(LLVMTypeRef*) * args_size);

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
	
	LLVMDumpModule(module);
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