#include "codegen-create-instance.h"
#include "../../../../../frontend/semantic/analyzer/analyzer.h"

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

	add_line_to_area(area, "	call	malloc");

	/* ------------------------------------ */
}

/**
 * TODO: Terminar a instanciação de classes.
 */
AsmReturn* generate_create_class_instance(CodeGen* codegen, Node* node, AsmArea* area)
{
	
	char* identifier = node->create_instance_node.create_instance.class_name;
	
	Symbol* symbol = analyzer_find_symbol_from_scope(identifier, codegen->scope, 0, 0, 1, 0);
	setup_instance_memory_alloc(codegen, symbol, area); // output reg é o 'RAX'

	return NULL;
}