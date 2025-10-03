#include "llvm-gen.h"
#include "llvm-c/Core.h"
#include "llvm-c/Types.h"

#include <string.h>

#include "../../../utils/logger/logger.h"

#include "../control-flow/control-flow.h"
#include "../control-flow/dominator-tree/dominator-tree.h"
#include "../control-flow/dominance-frontier/dominance-frontier.h"

#define ENTRY_NAME ".entry"
#define DEFAULT_PHI_START_CAPACITY 4
#define DEFAULT_BLOCK_PHIS_START_CAPACITY 3

static size_t expr_count = 0;
static size_t fload_count = 0;

static LLVMTypeRef get_type(Type* type);
static const char* generate_expr_label(int inc);
static int get_biggest(int* values, int length);
static char* get_field_label(char* label, int count);
static int defines_field(char* field_label, CFBlock* block);
static LLVMTypeRef* get_args_type(IRNode** args, unsigned int args_size);
static void setup_func_arg_names(LLVMValueRef* func, IRNode** args, unsigned int args_size);
static LLVMValueRef generate_expr(const LLVMModuleRef module, const LLVMBuilderRef llvm, IRNode* node);
static void generate_llvm_from_node(const LLVMModuleRef module, const LLVMBuilderRef llvm, IRNode* node);
static void handle_block_definitions(CFBlock* block, const unsigned int cf_blocks_length, int** field_count);
static void generate_func_blocks(const LLVMModuleRef module, const LLVMBuilderRef llvm, IRNode* func, const LLVMValueRef llvm_func);

// ==------------------------------------ Statements ------------------------------------== \\

static void generate_func(const LLVMModuleRef module, const LLVMBuilderRef llvm, IRNode* node)
{
	LLVMTypeRef return_type = get_type(node->func.type);

	unsigned int params_size = node->func.params_size;
	LLVMTypeRef* params_type = get_args_type(node->func.params, params_size);

	LLVMTypeRef type = LLVMFunctionType(return_type, params_type, params_size, 0);
	LLVMValueRef func = LLVMAddFunction(module, node->func.name, type);

	setup_func_arg_names(&func, node->func.params, params_size);

	generate_func_blocks(module, llvm, node, func);

	LLVMDumpModule(module);
}

static void generate_field(const LLVMModuleRef module, const LLVMBuilderRef llvm, IRNode* node)
{
	LLVMTypeRef type = get_type(node->field.type);
	LLVMValueRef field = LLVMBuildAlloca(llvm, type, node->field.name);

	// add_field_to_table(field, node->field.name, node->field.type);

	if (node->field.value == NULL)
	{
		return;
	}

	LLVMValueRef value = generate_expr(module, llvm, node->field.value);

	LLVMBuildStore(llvm, value, field);
}

static void generate_block(const LLVMModuleRef module, const LLVMBuilderRef llvm, IRNode* block)
{
        const int length = block->block.nodes->length;

        for (int i = 0; i < length; i++)
        {
                IRNode* instruction = block->block.nodes->elements[i];

                if (instruction == NULL)
                {
                        return;
                }

                generate_llvm_from_node(module, llvm, instruction);
        }
}

static void generate_ret(const LLVMModuleRef module, const LLVMBuilderRef llvm, IRNode* node)
{
        if (node->ret.value != NULL)
        {
                LLVMValueRef value = generate_expr(module, llvm, node->ret.value);
                LLVMBuildRet(llvm, value);
        }
        else
        {
                LLVMBuildRetVoid(llvm);
        }
}

// ==------------------------------------ Core ------------------------------------== \\

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

                        break;
		}

                case IR_NODE_RET:
                {
                        generate_ret(module, llvm, node);

                        break;
                }

		default:
		{
			break;
		}
	}

        println("Node with type id: %d, not implemented...", node->type);
        exit(1);
}

static void insert_func_phis(IRNode** fields, const unsigned int fields_length, CFBlock** blocks, const unsigned int blocks_length, DList** frontiers)
{
	int** has_phi = malloc(sizeof(int*) * fields_length);

	for (int i = 0; i < fields_length; i++)
	{
		has_phi[i] = malloc(sizeof(int) * blocks_length);
	}

	for (int i = 0; i < fields_length; i++)
	{
		IRNode* field = fields[i];

		if (field == NULL)
		{
			continue;
		}

		DList* work_list = create_list(8);

		for (int j = 0; j < blocks_length; j++)
		{
			CFBlock* block = blocks[j];

			if (block == NULL)
			{
				continue;
			}

			if (!defines_field(field->field.name, block))
			{
				continue;
			}

			int* index = malloc(sizeof(int));
			*index = block->dt_index; // frontiers usa dt_index

			add_element_to_list(work_list, index);
		}

		while (!is_empty_list(work_list))
		{
			int* index = pop_list(work_list);

			DList* frontier = frontiers[*index];
			const unsigned int length = frontier->length;

			for (int j = 0; j < length; j++)
			{
				CFBlock* block = frontier->elements[j];

				if (block == NULL)
				{
					continue;
				}

				if (has_phi[i][block->cf_index])
				{
					continue;
				}

				IRNode* phi = create_ir_node(IR_NODE_PHI);
				
				phi->phi.labels = create_list(4);
				
				phi->phi.field = fields[i];
				phi->phi.field_index = i;

				add_element_to_list(block->block->block.phis, phi);
				has_phi[i][block->cf_index] = 1;

				int* index = malloc(sizeof(int));
				*index = block->dt_index;

				add_element_to_list(work_list, index);
			}
		}

		free_list(work_list);
	}

	for (int i = 0; i < fields_length; i++)
	{
		free(has_phi[i]);
	}

	free(has_phi);
}

static void rename_block_phi(DTBlock* block, int* field_stack, const unsigned int fields_length, const unsigned int cf_blocks_length, int** field_count, int* has_renamed)
{
	if (has_renamed[block->block->cf_index])
	{
		return;
	}

	has_renamed[block->block->cf_index] = 1;

	const unsigned int preds_length = block->block->predecessors->length;
	const unsigned int phis_length = block->block->block->block.phis->length;

	for (int i = 0; i < phis_length; i++)
	{
		IRNode* phi = block->block->block->block.phis->elements[i];

		if (phi == NULL || phi->phi.field_index == -1)
		{
			continue;
		}

		char* name = phi->phi.field->field.name;

		for (int j = 0; j < preds_length; j++)
		{
			CFBlock* pred = block->block->predecessors->elements[j];

			if (pred == NULL)
			{
				continue;
			}

			char* label = get_field_label(name, field_count[phi->phi.field_index][pred->cf_index]);
			add_element_to_list(phi->phi.labels, label);
		}
	}

	for (int i = 0; i < fields_length; i++)
	{
		field_count[i][block->block->cf_index] = field_stack[i];
	}

	handle_block_definitions(block->block, cf_blocks_length, field_count);

	for (int i = 0; i < fields_length; i++)
	{
		field_stack[i] = field_count[i][block->block->cf_index];
	}

	const unsigned int childs_length = block->dominateds->length;

	for (int i = 0; i < childs_length; i++)
	{
		DTBlock* child = block->dominateds->elements[i];

		if (child == NULL)
		{
			continue;
		}

		rename_block_phi(child, field_stack, fields_length, cf_blocks_length, field_count, has_renamed);
	}
}

static void rename_func_phis(DTBlock* entry, CFBlock** cf_blocks, const unsigned int cf_blocks_length, IRNode** fields, const unsigned int fields_length)
{
	int** field_count = malloc(sizeof(int*) * fields_length);
	
	int* field_stack = malloc(sizeof(int) * fields_length);
	memset(field_stack, 0, sizeof(int) * fields_length);

	int has_renamed[cf_blocks_length];

	for (int i = 0; i < fields_length; i++)
	{
		field_count[i] = malloc(sizeof(int) * cf_blocks_length);
		memset(field_count[i], 0, sizeof(int) * cf_blocks_length);
	}

	memset(has_renamed, 0, sizeof(has_renamed));

	rename_block_phi(entry, field_stack, fields_length, cf_blocks_length, field_count, has_renamed);

	for (int i = 0; i < fields_length; i++)
	{
		free(field_count[i]);
	}

	free(field_count);
	free(field_stack);
}

static void generate_func_phi(IRNode* func)
{
	DList* cf = init_control_flow(func);
	DominatorTree* dt = generate_dominator_tree(cf->elements[0], cf->length);

	CFBlock** cf_list = (CFBlock**) cf->elements;
	
	DTBlock** dt_list = (DTBlock**) dt->blocks->elements;
	const int dt_length = dt->blocks->length;

	DList** frontiers = generate_dominance_frontier(cf_list, cf->length, dt->idominators, dt_list, dt_length);
	
	func->func.frontiers = frontiers;
	func->func.frontiers_length = cf->length;

	insert_func_phis(func->func.fields, func->func.fields_length, cf_list, cf->length, frontiers);
	rename_func_phis(dt_list[0], cf_list, cf->length, func->func.fields, func->func.fields_length);

	free_list(cf);
	
	free_list(dt->blocks);
	free(dt->idominators);
	free(dt);

	for (int i = 0; i < cf->length; i++)
	{
		free(frontiers[i]);
	}
}

static void generate_funcs_phi(IRNode** nodes, const int length)
{
	for (int i = 0; i < length; i++)
	{
		IRNode* node = nodes[i];

		if (node == NULL)
		{
			continue;
		}

		if (node->type != IR_NODE_FUNC)
		{
			continue;
		}

		generate_func_phi(node);
	}
}

/**
 * TODO: adicionar o sistema de modulos depois
 */
void generate_module_llvm(IRNode** nodes, IRNode** fields, const int length)
{
	const LLVMModuleRef module = LLVMModuleCreateWithName("test_module"); /** TODO: implementar o negocio certo nisso */
	const LLVMBuilderRef builder = LLVMCreateBuilder();

	generate_funcs_phi(nodes, length);

	for (int i = 0; i < length; i++)
	{
		IRNode* curr = nodes[i];

		if (curr == NULL)
		{
			continue;
		}

		generate_llvm_from_node(module, builder, curr);
	}
}

// ==------------------------------------ Expressions ------------------------------------== \\

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

static LLVMValueRef generate_fadd_operation(const LLVMModuleRef module, const LLVMBuilderRef llvm, const LLVMValueRef left, const LLVMValueRef right)
{
	const LLVMValueRef result = LLVMBuildFAdd(llvm, left, right, generate_expr_label(1));

	return result;
}

static LLVMValueRef generate_isub_operation(const LLVMModuleRef module, const LLVMBuilderRef llvm, const LLVMValueRef left, const LLVMValueRef right)
{
	const LLVMValueRef result = LLVMBuildSub(llvm, left, right, generate_expr_label(1));

	return result;
}

static LLVMValueRef generate_fsub_operation(const LLVMModuleRef module, const LLVMBuilderRef llvm, const LLVMValueRef left, const LLVMValueRef right)
{
	const LLVMValueRef result = LLVMBuildFSub(llvm, left, right, generate_expr_label(1));

	return result;
}

static LLVMValueRef generate_imul_operation(const LLVMModuleRef module, const LLVMBuilderRef llvm, const LLVMValueRef left, const LLVMValueRef right)
{
	const LLVMValueRef result = LLVMBuildMul(llvm, left, right, generate_expr_label(1));

	return result;
}

static LLVMValueRef generate_fmul_operation(const LLVMModuleRef module, const LLVMBuilderRef llvm, const LLVMValueRef left, const LLVMValueRef right)
{
	const LLVMValueRef result = LLVMBuildFMul(llvm, left, right, generate_expr_label(1));

	return result;
}

static LLVMValueRef generate_idiv_operation(const LLVMModuleRef module, const LLVMBuilderRef llvm, const LLVMValueRef left, const LLVMValueRef right)
{
	const LLVMValueRef result = LLVMBuildSDiv(llvm, left, right, generate_expr_label(1));

	return result;
}

static LLVMValueRef generate_fdiv_operation(const LLVMModuleRef module, const LLVMBuilderRef llvm, const LLVMValueRef left, const LLVMValueRef right)
{
	const LLVMValueRef result = LLVMBuildFDiv(llvm, left, right, generate_expr_label(1));

	return result;
}

static LLVMValueRef generate_and_operation(const LLVMModuleRef module, const LLVMBuilderRef llvm, const LLVMValueRef left, const LLVMValueRef right)
{
	const LLVMValueRef result = LLVMBuildAnd(llvm, left, right, generate_expr_label(1));

	return result;
}

static LLVMValueRef generate_or_operation(const LLVMModuleRef module, const LLVMBuilderRef llvm, const LLVMValueRef left, const LLVMValueRef right)
{
	const LLVMValueRef result = LLVMBuildOr(llvm, left, right, generate_expr_label(1));

	return result;
}

static LLVMValueRef generate_igt_operation(const LLVMModuleRef module, const LLVMBuilderRef llvm, const LLVMValueRef left, const LLVMValueRef right)
{
	const LLVMValueRef result = LLVMBuildICmp(llvm, LLVMIntSGT, left, right, generate_expr_label(1));

	return result;
}

static LLVMValueRef generate_fgt_operation(const LLVMModuleRef module, const LLVMBuilderRef llvm, const LLVMValueRef left, const LLVMValueRef right)
{
	const LLVMValueRef result = LLVMBuildFCmp(llvm, LLVMRealUGT, left, right, generate_expr_label(1));

	return result;
}

static LLVMValueRef generate_ige_operation(const LLVMModuleRef module, const LLVMBuilderRef llvm, const LLVMValueRef left, const LLVMValueRef right)
{
	const LLVMValueRef result = LLVMBuildICmp(llvm, LLVMIntSGE, left, right, generate_expr_label(1));

	return result;
}

static LLVMValueRef generate_fge_operation(const LLVMModuleRef module, const LLVMBuilderRef llvm, const LLVMValueRef left, const LLVMValueRef right)
{
	const LLVMValueRef result = LLVMBuildFCmp(llvm, LLVMRealUGE, left, right, generate_expr_label(1));

	return result;
}

static LLVMValueRef generate_ilt_operation(const LLVMModuleRef module, const LLVMBuilderRef llvm, const LLVMValueRef left, const LLVMValueRef right)
{
	const LLVMValueRef result = LLVMBuildICmp(llvm, LLVMIntSLT, left, right, generate_expr_label(1));

	return result;
}

static LLVMValueRef generate_flt_operation(const LLVMModuleRef module, const LLVMBuilderRef llvm, const LLVMValueRef left, const LLVMValueRef right)
{
	const LLVMValueRef result = LLVMBuildFCmp(llvm, LLVMRealULT, left, right, generate_expr_label(1));

	return result;
}

static LLVMValueRef generate_ile_operation(const LLVMModuleRef module, const LLVMBuilderRef llvm, const LLVMValueRef left, const LLVMValueRef right)
{
	const LLVMValueRef result = LLVMBuildICmp(llvm, LLVMIntSLE, left, right, generate_expr_label(1));

	return result;
}

static LLVMValueRef generate_fle_operation(const LLVMModuleRef module, const LLVMBuilderRef llvm, const LLVMValueRef left, const LLVMValueRef right)
{
	const LLVMValueRef result = LLVMBuildFCmp(llvm, LLVMRealULE, left, right, generate_expr_label(1));

	return result;
}

static LLVMValueRef generate_ie_operation(const LLVMModuleRef module, const LLVMBuilderRef llvm, const LLVMValueRef left, const LLVMValueRef right)
{
	const LLVMValueRef result = LLVMBuildFCmp(llvm, LLVMRealUEQ, left, right, generate_expr_label(1));

	return result;
}

static LLVMValueRef generate_fe_operation(const LLVMModuleRef module, const LLVMBuilderRef llvm, const LLVMValueRef left, const LLVMValueRef right)
{
	const LLVMValueRef result = LLVMBuildFCmp(llvm, LLVMRealUEQ, left, right, generate_expr_label(1));

	return result;
}

static LLVMValueRef generate_ine_operation(const LLVMModuleRef module, const LLVMBuilderRef llvm, const LLVMValueRef left, const LLVMValueRef right)
{
	const LLVMValueRef result = LLVMBuildFCmp(llvm, LLVMRealUNE, left, right, generate_expr_label(1));

	return result;
}

static LLVMValueRef generate_fne_operation(const LLVMModuleRef module, const LLVMBuilderRef llvm, const LLVMValueRef left, const LLVMValueRef right)
{
	const LLVMValueRef result = LLVMBuildFCmp(llvm, LLVMRealUNE, left, right, generate_expr_label(1));

	return result;
}

static LLVMValueRef generate_operation(const LLVMModuleRef module, const LLVMBuilderRef llvm, IRNode* node)
{
	Type* type = node->operation.type;

	const LLVMValueRef left = generate_expr(module, llvm, node->operation.left);
	const LLVMValueRef right = generate_expr(module, llvm, node->operation.right);

	switch (node->operation.operation)
	{
		case IR_OPERATION_ADD:
		{
			if (type->type == TYPE_INT)
			{
				return generate_iadd_operation(module, llvm, left, right);
			}
			
			return generate_fadd_operation(module, llvm, left, right);
		}

		case IR_OPERATION_SUB:
		{
			if (type->type == TYPE_INT)
			{
				return generate_isub_operation(module, llvm, left, right);
			}
			
			return generate_fsub_operation(module, llvm, left, right);
		}

		case IR_OPERATION_MUL:
		{
			if (type->type == TYPE_INT)
			{
				return generate_imul_operation(module, llvm, left, right);
			}
			
			return generate_fmul_operation(module, llvm, left, right);
		}

		case IR_OPERATION_DIV:
		{
			if (type->type == TYPE_INT)
			{
				return generate_idiv_operation(module, llvm, left, right);
			}
			
			return generate_fdiv_operation(module, llvm, left, right);
		}
		
		case IR_OPERATION_GREATER:
		{
			if (type->type == TYPE_INT)
			{
				return generate_igt_operation(module, llvm, left, right);
			}
			
			return generate_fgt_operation(module, llvm, left, right);
		}

		case IR_OPERATION_GREATER_EQUALS:
		{
			if (type->type == TYPE_INT)
			{
				return generate_ige_operation(module, llvm, left, right);
			}
			
			return generate_fge_operation(module, llvm, left, right);

		}

		case IR_OPERATION_LESS:
		{
			if (type->type == TYPE_INT)
			{
				return generate_ilt_operation(module, llvm, left, right);
			}
			
			return generate_flt_operation(module, llvm, left, right);
		}

		case IR_OPERATION_LESS_EQUALS:
		{
			if (type->type == TYPE_INT)
			{
				return generate_ile_operation(module, llvm, left, right);
			}
			
			return generate_fle_operation(module, llvm, left, right);
		}

		case IR_OPERATION_EQUALS:
		{
			if (type->type == TYPE_INT)
			{
				return generate_ie_operation(module, llvm, left, right);
			}
			
			return generate_fe_operation(module, llvm, left, right);
		}

		case IR_OPERATION_NOT_EQUALS:
		{
			if (type->type == TYPE_INT)
			{
				return generate_ine_operation(module, llvm, left, right);
			}
			
			return generate_fne_operation(module, llvm, left, right);
		}

		case IR_OPERATION_OR:
		{
			return generate_or_operation(module, llvm, left, right);
		}

		case IR_OPERATION_AND:
		{
			return generate_and_operation(module, llvm, left, right);
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

// ==------------------------------------ Utils ------------------------------------== \\

static void handle_block_definitions(CFBlock* block, const unsigned int cf_blocks_length, int** field_count)
{
	const unsigned int length = block->block->block.nodes->length;
	const unsigned int phis_length = block->block->block.phis->length;

	for (int i = 0; i < phis_length; i++)
	{
		IRNode* phi = block->block->block.phis->elements[i];

		if (phi == NULL)
		{
			continue;
		}

		field_count[phi->phi.field_index][block->cf_index]++;
	}

	for (int i = 0; i < length; i++)
	{
		IRNode* node = block->block->block.nodes->elements[i];

		if (node == NULL)
		{
			continue;
		}

		if (node->type != IR_NODE_STORE && node->type != IR_NODE_FIELD)
		{
			continue;
		}

		if (node->type == IR_NODE_FIELD)
		{
			/**
			 * TODO: adicionar coisa aqui depois.
			 */
		}
		else 
		{
			if (node->store.field_index == -1)
			{
				continue;
			}
			
			field_count[node->store.field_index][block->cf_index]++;
		}
	}
}

static int get_biggest(int* values, int length)
{
	int biggest = -100000;
	
	for (int i = 0; i < length; i++)
	{
		int value = values[i];

		if (value <= biggest)
		{
			continue;
		}

		biggest = value;
	}

	return biggest;
}

static char* get_field_label(char* label, int count)
{
	char buff[512];
	snprintf(buff, 512, ".%s_%d", label, count);

	return _strdup(buff);
}

static int defines_field(char* field_label, CFBlock* block)
{
	const unsigned int length = block->block->block.nodes->length;

	for (int i = 0; i < length; i++)
	{
		IRNode* node = block->block->block.nodes->elements[i];

		if (node == NULL)
		{
			continue;
		}

		if (node->type != IR_NODE_STORE && node->type != IR_NODE_FIELD)
		{
			continue;
		}

		if (node->type == IR_NODE_FIELD && node->field.value == NULL)
		{
			continue;
		}

		return 1;
	}

	return 0;
}

static void generate_func_blocks(const LLVMModuleRef module, const LLVMBuilderRef llvm, IRNode* func, const LLVMValueRef llvm_func)
{
        const int length = func->func.blocks->length;

        for (int i = 0; i < length; i++)
        {
                IRNode* block = func->func.blocks->elements[i];

                if (block == NULL)
                {
                        continue;
                }

                LLVMBasicBlockRef bb = LLVMAppendBasicBlock(llvm_func, block->block.label);
                LLVMPositionBuilderAtEnd(llvm, bb);

                generate_block(module, llvm, block);
        }
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
			return LLVMPointerType(LLVMInt8Type(), 0);
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
