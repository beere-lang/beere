#include <stdlib.h>

#include "codegen-class.h"
#include "../../../analyzer/analyzer.h"

extern char* get_literal_value(LiteralNode* literal);

static void generate_class_static_field_declaration(CodeGen* codegen, char* class_name, Node* node, AsmArea* area)
{
	char buff[64];

	if (node->declare_node.declare.default_value->type != NODE_LITERAL)
	{
		printf("[Codegen] Static class fields requires compile-time known values for initializer...\n");
		exit(1);
	}
	
	Symbol* symbol = analyzer_find_symbol_from_scope(node->declare_node.declare.identifier, codegen->scope, 1, 0, 0, 0);
		
	snprintf(buff, 64, ".%s_static_%s:	%s", class_name, node->declare_node.declare.identifier, get_literal_value(&node->declare_node.declare.default_value->literal_node.literal));
	add_line_to_area(data_section, buff);
}

static void generate_vtable_method(CodeGen* codegen, MethodEntry* method_entry, AsmArea* table_area)
{
	char buff[64];
	snprintf(buff, 64, "	dq	.%s_fn_%s", method_entry->class_name, method_entry->method_name);

	add_line_to_area(table_area, buff);
}

static AsmArea* generate_class_vtable(CodeGen* codegen, Symbol* symbol_class, ClassVTable* vtable) 
{
	AsmArea* table_area = create_area();
	
	char buff[64];
	snprintf(buff, 64, ".%s_v-table:", symbol_class->symbol_class->identifier);

	add_line_to_area(table_area, buff);

	for (int i = 0; i < vtable->entries_count; i++) 
	{
		generate_vtable_method(codegen, vtable->entries[i], table_area);
	}

	return table_area;
}

static void generate_class_field(CodeGen* codegen, Node* field)
{

}

static void generate_class_fields(CodeGen* codegen, Node** fields, int fields_count) 
{
	for (int i = 0; i < fields_count; i++)
	{
		Node* field = fields[i];

		if (field->declare_node.declare.is_static)
		{
			generate_class_field(codegen, field);
		}
	}
}

static void generate_class_methods(CodeGen* codegen, Node** methods, int methods_count, AsmArea* area) 
{

}

static void merge_class_vtable(AsmArea* table_area)
{
	for (int i = 0; i < table_area->lines_length; i++)
	{
		char* line = table_area->lines[i];

		add_line_to_area(data_section, line);
	}
}

void generate_class(CodeGen* codegen, Node* node, AsmArea* area) 
{
	Symbol* class_symbol = analyzer_find_symbol_from_scope(node->class_node.class_node.identifer, codegen->scope, 0, 0, 1, 0);
	AsmArea* table = generate_class_vtable(codegen, class_symbol, class_symbol->symbol_class->class_v_table);

	merge_class_vtable(table);

	generate_class_fields(codegen, node->class_node.class_node.var_declare_list, node->class_node.class_node.var_count);
}