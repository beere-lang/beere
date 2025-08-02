#include <string.h>

#include "codegen-create-instance.h"
#include "../../../../../frontend/semantic/analyzer/analyzer.h"
#include "../../../../../frontend/structure/parser/parser.h"
#include "../../expressions/codegen-expr.h"

extern char* field_get_mov_op_code_access(CodeGen* codegen, Type* type);
extern char* field_get_reference_access_size(CodeGen* codegen, Type* type);

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

	if (node->declare_node.declare.is_static)
	{
		return;
	}

	char* access_size = field_get_reference_access_size(codegen, type);

	snprintf(buff, 64, "%s [r8+%d]", access_size, offset);
	destiny = strdup(buff);

	/* ---------------------------------------------- */
	
	AsmReturn* ret = generate_expression(codegen, node->declare_node.declare.default_value, area, 1, 0, 0);

	snprintf(buff, 64, "	%s	%s, %s", mov_opcode, destiny, ret->result);
	add_line_to_area(area, buff);

	/* ---------------------------------------------- */
}

static void generate_class_fields_initialization(CodeGen* codegen, Symbol* symbol, AsmArea* area, int* offset)
{
	for (int i = 0; i < symbol->symbol_class->field_count; i++)
	{
		Node* curr = symbol->symbol_class->fields[i];

		const int BYTES_ALIGNMENT_SIZE = 8;
		
		int size = analyzer_get_type_size(curr->declare_node.declare.var_type, codegen->scope);
		int aligned = (size % BYTES_ALIGNMENT_SIZE == 0) ? size : ((size / BYTES_ALIGNMENT_SIZE) + 1) * BYTES_ALIGNMENT_SIZE;

		generate_field_initialization(codegen, curr, area, *offset);

		*offset += aligned;
	}
}

static void generate_class_fields(CodeGen* codegen, Symbol* symbol, AsmArea* area)
{
	int offset = 0;
	
	while (symbol != NULL)
	{
		generate_class_fields_initialization(codegen, symbol, area, &offset);

		symbol = symbol->symbol_class->super;
	}
}

/**
 * TODO: 
 * - Terminar a chamada de constructors na instanciação.
 * - Implementar class offsets table.
 * - Implementar super no codegen.
 */
AsmReturn* generate_create_class_instance(CodeGen* codegen, Node* node, AsmArea* area)
{
	char* identifier = node->create_instance_node.create_instance.class_name;
	
	Symbol* symbol = analyzer_find_symbol_from_scope(identifier, codegen->scope, 0, 0, 1, 0);
	setup_instance_memory_alloc(codegen, symbol, area); // output reg é o 'R8', movido de 'RAX' pra 'R8', pra evitar override

	generate_class_fields(codegen, symbol, area);

	Type* type = create_type(TYPE_CLASS, node->class_node.class_node.identifer);
	AsmReturn* ret = create_asm_return("r8", type);

	return ret;
}