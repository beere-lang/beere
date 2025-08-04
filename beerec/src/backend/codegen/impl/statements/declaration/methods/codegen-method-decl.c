#include "codegen-method-decl.h"
#include "../../../../../../frontend/semantic/analyzer/analyzer.h"

void generate_method_setup(AsmArea* area, Symbol* symbol, int is_class, char* prefered_name)
{
	char buff[64];

	snprintf(buff, 64, "%s:", (!is_class) ? symbol->symbol_function->identifier : prefered_name);
	add_line_to_area(area, buff);

	add_line_to_area(area, "	push	rbp");
	add_line_to_area(area, "	mov	rbp, rsp");

	if (symbol->symbol_function->total_offset + 8 != 0)
	{
		// +8 por causa dos 8 bytes do backup do rbp
		snprintf(buff, 64, "	sub	rsp, %d", (symbol->symbol_function->total_offset + 8) * -1);
		add_line_to_area(area, buff);
	}
}

void generate_method_unsetup(AsmArea* area)
{
	add_line_to_area(area, "	leave");
	add_line_to_area(area, "	ret");
}

void generate_method_statements(CodeGen* codegen, Node* block_node, AsmArea* area)
{
	Node* next = block_node->block_node.block.statements->head;

	while (next != NULL)
	{
		generate_node(codegen, next, area);
		next = next->next;
	}
}

void merge_method_area(AsmArea* area)
{
	for (int i = 0; i < area->lines_length; i++)
	{
		char* curr = area->lines[i];
		add_line_to_area(text_section, curr);
	}
}

void generate_method_declaration(CodeGen* codegen, Node* node, AsmArea* area, int is_class, char* class_method_name)
{
	AsmArea* method_area = create_area();

	Symbol* symbol = analyzer_find_symbol_from_scope(node->function_node.function.identifier, codegen->scope, 0, 1, 0, 0);
	SymbolTable* temp = codegen->scope;
	
	codegen->scope = symbol->symbol_function->scope;
	
	generate_method_setup(method_area, symbol, is_class, class_method_name);

	generate_method_statements(codegen, node->function_node.function.block_node, method_area);

	generate_method_unsetup(method_area);

	codegen->scope = temp;

	merge_method_area(method_area);
}