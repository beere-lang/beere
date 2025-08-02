#include <string.h>
#include <stdlib.h>

#include "codegen-create-instance.h"
#include "../../../../../frontend/semantic/analyzer/analyzer.h"
#include "../../../../../frontend/structure/parser/parser.h"
#include "../../expressions/codegen-expr.h"
#include "../codegen-class.h"

extern char* field_get_mov_op_code_access(CodeGen* codegen, Type* type);
extern char* field_get_reference_access_size(CodeGen* codegen, Type* type);
extern Symbol* find_method_owner(SymbolTable* scope);
extern int get_method_stack_size(Symbol* owner_method);
extern void generate_method_args(CodeGen* codegen, NodeList* args, AsmArea* area, int* stack_ref);

void generate_method_constructor_call(CodeGen* codegen, Node* node, AsmArea* area)
{
	char buff[64];

	Symbol* owner_method = find_method_owner(codegen->scope);
	Type* type = owner_method->symbol_function->return_type;
	int stack_size = get_method_stack_size(owner_method);

	int padding = 0;

	if ((stack_size + 8) % 16 != 0) // +8 pro return point do call
	{
		snprintf(buff, 64, "	sub	rsp, 8");
		add_line_to_area(area, buff);

		padding = 1;
	}

	if (node->create_instance_node.create_instance.constructor_args != NULL)
	{
		generate_method_args(codegen, node->create_instance_node.create_instance.constructor_args, area, &stack_size);
	}

	snprintf(buff, 64, "	call	.%s_ctr", node->create_instance_node.create_instance.class_name);
	add_line_to_area(area, buff);

	if (padding)
	{
		add_line_to_area(area, "	add	rsp, 8");
	}
}

int get_class_total_offset(CodeGen* codegen, Symbol* symbol)
{
	int total_offset = 0;
	Symbol* symbol_class = symbol;

	while (symbol_class != NULL)
	{
		total_offset += symbol_class->symbol_class->total_offset;
		symbol_class = symbol_class->symbol_class->super;
	}

	return total_offset;
}

void setup_instance_memory_alloc(CodeGen* codegen, Symbol* symbol, AsmArea* area)
{
	char buff[64];

	/* ------------------------------------ */

	snprintf(buff, 64, "	mov	rcx, %d", get_class_total_offset(codegen, symbol));
	add_line_to_area(area, buff);

	/* ------------------------------------ */

	ExternEntry* entry = create_extern_entry("malloc");
	add_extern_entry_to_table(entry);

	/* ------------------------------------ */

	add_line_to_area(area, "	call	malloc");
	add_line_to_area(area, "	mov	r8, rax");

	/* ------------------------------------ */
}

static void generate_field_initialization(CodeGen* codegen, Node* node, AsmArea* area, int offset)
{
	if (node->declare_node.declare.default_value == NULL)
	{
		return;
	}

	Type* type = node->declare_node.declare.var_type;

	char buff[64];

	char* mov_opcode = field_get_mov_op_code_access(codegen, type);
	char* destiny = NULL;

	char* access_size = field_get_reference_access_size(codegen, type);

	snprintf(buff, 64, "%s [r8+%d]", access_size, offset);
	destiny = strdup(buff);

	/* ---------------------------------------------- */
	
	AsmReturn* ret = generate_expression(codegen, node->declare_node.declare.default_value, area, 1, 0, 0);

	snprintf(buff, 64, "	%s	%s, %s", mov_opcode, destiny, ret->result);
	add_line_to_area(area, buff);

	/* ---------------------------------------------- */
}

static void generate_class_fields(CodeGen* codegen, Symbol* symbol, AsmArea* area)
{
	for (int i = 0; i < symbol->symbol_class->field_count; i++)
	{
		Node* curr = symbol->symbol_class->fields[i];

		if (curr->declare_node.declare.is_static)
		{
			continue;
		}
		
		ClassOffsets* offsets = find_class_offsets(class_offsets_table, (char*) symbol->symbol_class->identifier);
		int offset = find_field_offset(offsets, curr->declare_node.declare.identifier);

		generate_field_initialization(codegen, curr, area, offset);
	}
}

AsmReturn* generate_create_class_instance(CodeGen* codegen, Node* node, AsmArea* area)
{
	char* identifier = node->create_instance_node.create_instance.class_name;
	
	Symbol* symbol = analyzer_find_symbol_from_scope(identifier, codegen->scope, 0, 0, 1, 0);
	setup_instance_memory_alloc(codegen, symbol, area); // output reg é o 'R8', movido de 'RAX' pra 'R8', ja que 'RAX' é muito usado.

	generate_class_fields(codegen, symbol, area);
	generate_method_constructor_call(codegen, node, area);

	Type* type = create_type(TYPE_CLASS, identifier);
	AsmReturn* ret = create_asm_return("r8", type);
	ret->is_reg = 1;

	return ret;
}