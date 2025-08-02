#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "codegen-method-call.h"
#include "../../expressions/codegen-expr.h"
#include "../../../../../frontend/semantic/analyzer/analyzer.h"

extern Type* create_type(VarType type, char* class_name);

Symbol* find_method_owner(SymbolTable* scope)
{
	if (scope == NULL)
	{
		return NULL;
	}

	if (scope->scope_kind == SYMBOL_FUNCTION)
	{
		return scope->owner_statement;
	}

	return find_method_owner(scope->parent);
}

int get_method_stack_size(Symbol* method)
{
	int stack_size = method->symbol_function->total_offset + 8; // +8 == 'push rbp' no setup da função
	return stack_size;
}

static int align_to_eight(int value)
{
	if (value % 8 == 0)
	{
		return value;
	}
	
	return ((value / 8) + 1) * 8;
}

static void handle_float_argument(CodeGen* codegen, AsmReturn* reg, AsmArea* area, int is_double)
{
	char* temp = is_double ? "movsd" : "movss";
	char* _temp = is_double ? "qword" : "dword";
	char buff[64];
	
	add_line_to_area(area, "	sub	rsp, 8");
	snprintf(buff, 64, "	%s	%s [rsp], %s", temp, _temp, reg->result);
	add_line_to_area(area, buff);
}

void generate_method_args(CodeGen* codegen, NodeList* args, AsmArea* area, int* stack_size_ref)
{
	char buff[64];
	Node* next = args->head;

	while (next != NULL)
	{
		Type* type = analyzer_return_type_of_expression(NULL, next->argument_node.argument.value, codegen->scope, 0, 0, NULL);
		AsmReturn* ret = generate_expression(codegen, next->argument_node.argument.value, area, 1, 0, 1);
		
		if (ret->type->type == TYPE_FLOAT || ret->type->type == TYPE_DOUBLE)
		{
			handle_float_argument(codegen, ret, area, ret->type->type == TYPE_DOUBLE);
		}
		else
		{
			snprintf(buff, 64, "	push	%s", ret->result);
			add_line_to_area(area, buff);
		}

		*stack_size_ref += align_to_eight(analyzer_get_type_size(type, codegen->scope));

		next = next->next;
	}
}

static char* get_method_name_by_callee(Node* callee)
{
	if (callee->type == NODE_IDENTIFIER)
	{
		return callee->variable_node.variable.identifier;
	}

	if (callee->type == NODE_MEMBER_ACCESS)
	{
		return callee->member_access_node.member_access.member_name;
	}

	return NULL;
}

/**
 * @param area: Usado pra caso seja necessario mover o valor pra outro register.
 */
char* call_get_return_register(Type* type, int argument_flag, AsmArea* area)
{
	printf("Type: %d\n", type->type);
	
	switch (type->type)
	{
		case TYPE_BOOL:
		{
			if (argument_flag)
			{
				add_line_to_area(area, "	mov	rax, al");

				return strdup("rax");
			}
			
			return strdup("al");
		}

		case TYPE_CHAR:
		{
			if (argument_flag)
			{
				add_line_to_area(area, "	mov	rax, al");

				return strdup("rax");
			}
			
			return strdup("al");
		}
		
		case TYPE_INT:
		{
			if (argument_flag)
			{
				add_line_to_area(area, "	mov	rax, eax");

				return strdup("rax");
			}
			
			return strdup("eax");
		}

		case TYPE_FLOAT:
		{
			return strdup("xmm0");
		}

		case TYPE_DOUBLE:
		{
			return strdup("xmm0");
		}

		case TYPE_CLASS: // classes são ponteiros implicitos
		{
			return strdup("rax");
		}

		case TYPE_PTR:
		{
			return strdup("rax");
		}

		case TYPE_STRING:
		{
			return strdup("rax");
		}

		case TYPE_VOID:
		{
			return strdup("");
		}

		default:
		{
			exit(1);
		}
	}
}

AsmReturn* generate_method_call(CodeGen* codegen, Node* node, AsmArea* area, int prefer_second, int argument_flag)
{
	char buff[64];

	Node* callee = node->function_call_node.function_call.callee;
	char* method_name = get_method_name_by_callee(callee);

	Symbol* symbol_method = NULL;
	
	if (callee->type == NODE_IDENTIFIER)
	{
		symbol_method = analyzer_find_symbol_from_scope(method_name, codegen->scope, 0, 1, 0, 0);
	}

	Symbol* owner_method = find_method_owner(codegen->scope);
	Type* type = owner_method->symbol_function->return_type;
	int stack_size = get_method_stack_size(owner_method);

	int padding = 0;
	int backup_size = 0;

	if ((stack_size + 8) % 16 != 0) // +8 pro return point do call
	{
		snprintf(buff, 64, "	sub	rsp, 8");
		add_line_to_area(area, buff);

		padding = 1;
	}

	if (padding)
	{
		backup_size = 8;
	}

	NodeList* constructor_args = node->create_instance_node.create_instance.constructor_args;
	
	if (constructor_args != NULL)
	{
		generate_method_args(codegen, constructor_args, area, &stack_size);
	}

	if (callee->type != NODE_IDENTIFIER && callee->type != NODE_SUPER) // mover a instance pra algum registrador nao utilizado pra methods de classes.
	{
		AsmReturn* ret = generate_expression(codegen, callee->member_access_node.member_access.object, area, 1, prefer_second, 0);
		
		// Move o ponteiro da class pro 'R8' (usado como referencia pra instancia)
		snprintf(buff, 64, "	mov	r8, %s", ret->result);
		add_line_to_area(area, buff);
		
		// Move o ponteiro da vtable (offset 0 da class)
		snprintf(buff, 64, "	mov	r9, qword [%s]", ret->result); 
		add_line_to_area(area, buff);

		snprintf(buff, 64, "	mov	r9, qword [r9+%d]", symbol_method->symbol_function->method_id * 8);
		add_line_to_area(area, buff);

		add_line_to_area(area, "	call	r9");
	}
	else if (callee->type == NODE_IDENTIFIER)
	{
		snprintf(buff, 64, "	call	%s", method_name);
		add_line_to_area(area, buff);
	}
	else if (callee->type == NODE_SUPER)
	{
		AsmReturn* ret = generate_expression(codegen, callee, area, 0, 0, 0);

		// Ponteiro da instancia ja ta no 'R8'
		// snprintf(buff, 64, "	mov	r8, %s", ret->result);
		// add_line_to_area(area, buff);

		snprintf(buff, 64, "	call	.%s_ctr", ret->type->class_name);
		add_line_to_area(area, buff);
	}

	snprintf(buff, 64, "	call	.%s_ctr", node->create_instance_node.create_instance.class_name);
	add_line_to_area(area, buff);

	if (constructor_args != NULL)
	{
		int args_size = analyzer_get_list_size(node->create_instance_node.create_instance.constructor_args->head);
		backup_size += args_size * 8;
	}

	snprintf(buff, 64, "	add	rsp, %d", backup_size);
	add_line_to_area(area, buff);

	if (callee->type == NODE_SUPER)
	{
		return create_asm_return("", create_type(TYPE_VOID, NULL));
	}

	char* output_reg = call_get_return_register(type, argument_flag, area);

	return create_asm_return(output_reg, type);
}