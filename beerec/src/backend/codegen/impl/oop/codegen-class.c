#include <stdlib.h>
#include <string.h>

#include "codegen-class.h"
#include "../../../../frontend/semantic/analyzer/analyzer.h"
#include "../statements/declaration/methods/codegen-method-decl.h"

extern char* get_literal_value(LiteralNode* literal);

ClassOffsets* find_class_offsets(ClassOffsetsTable* table, char* class_name)
{
	for (int i = 0; i < table->class_offsets_length; i++)
	{
		ClassOffsets* offsets = table->class_offsets[i];
			
		if (strcmp(offsets->class_name, class_name) == 0)
		{
			return offsets;
		}
	}

	return NULL;
}

int find_field_offset(ClassOffsets* offsets, char* field_name)
{
	if (offsets == NULL)
	{
		return -1;
	}

	for (int i = 0; i < offsets->fields_length; i++)
	{
		FieldEntry* curr = offsets->fields[i];
		
		if (strcmp(curr->field_name, field_name) == 0)
		{
			return curr->field_offset;
		}
	}

	return find_field_offset(offsets->parent, field_name);
}

ClassOffsetsTable* create_class_offsets_table()
{
	ClassOffsetsTable* table = malloc(sizeof(ClassOffsetsTable));
	
	table->class_offsets = malloc(sizeof(ClassOffsets*) * 4);
	table->class_offsets_capacity = 4;

	table->class_offsets_length = 0;

	return table;
}

int check_offsets_table_cache(ClassOffsetsTable* table, char* owner_class_name)
{
	for (int i = 0; i < table->class_offsets_length; i++)
	{
		ClassOffsets* offsets = table->class_offsets[i];

		if (strcmp(offsets->class_name, owner_class_name) == 0)
		{
			return 1;
		}
	}

	return 0;
}

void add_offsets_to_table(ClassOffsetsTable* table, ClassOffsets* offsets)
{
	if (check_offsets_table_cache(table, offsets->class_name))
	{
		return;
	}

	if (table->class_offsets_capacity <= table->class_offsets_length)
	{
		table->class_offsets_capacity *= 2;

		table->class_offsets = realloc(table->class_offsets, sizeof(ClassOffsets*) * table->class_offsets_capacity);
	}

	table->class_offsets[table->class_offsets_length] = offsets;
	table->class_offsets_length++;
}

FieldEntry* create_field_entry(CodeGen* codegen, char* field_name, int size, int offset, Type* field_type)
{
	FieldEntry* entry = malloc(sizeof(FieldEntry));
	
	entry->field_name = strdup(field_name);
	entry->field_size = size;
	entry->field_offset = offset;

	return entry;
}

void add_entry_to_offsets(ClassOffsets* offsets, FieldEntry* entry)
{
	if (offsets->fields_capacity <= offsets->fields_length)
	{
		offsets->fields_capacity *= 2;

		offsets->fields = realloc(offsets->fields, sizeof(FieldEntry*) * offsets->fields_capacity);
	}

	offsets->fields[offsets->fields_length] = entry;
	offsets->fields_length++;
}

ClassOffsets* create_class_offsets(char* class_name, int offset)
{
	ClassOffsets* offsets = malloc(sizeof(ClassOffsets));
	
	offsets->class_name = strdup(class_name);
	
	offsets->fields = malloc(sizeof(FieldEntry*) * 4);
	offsets->fields_capacity = 4;
	offsets->fields_length = 0;

	offsets->start_offset = 0;
	offsets->end_offset = 0;

	offsets->parent = NULL;

	return offsets;
}

static void generate_class_static_field_declaration(CodeGen* codegen, char* class_name, Node* node)
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

static void generate_class_field(CodeGen* codegen, char* class_name, Node* field)
{
	generate_class_static_field_declaration(codegen, class_name, field);
}

static void generate_class_fields(CodeGen* codegen, char* class_name, Node** fields, int fields_count) 
{
	for (int i = 0; i < fields_count; i++)
	{
		Node* field = fields[i];

		if (field->declare_node.declare.is_static)
		{
			generate_class_field(codegen, class_name, field);
		}
	}
}

static void generate_class_method(CodeGen* codegen, char* class_name, Node* method)
{
	char buff[64];
	snprintf(buff, 64, ".%s_fn_%s", class_name, method->function_node.function.identifier);

	generate_method_declaration(codegen, method, text_section, 1, strdup(buff));
}

static void generate_class_constructor(CodeGen* codegen, char* class_name, Node* method)
{
	char buff[64];
	snprintf(buff, 64, ".%s_ctr", class_name);

	generate_method_declaration(codegen, method, text_section, 1, strdup(buff));
}

static void generate_class_methods(CodeGen* codegen, char* class_name, Node** methods, int methods_count) 
{
	for (int i = 0; i < methods_count; i++)
	{
		Node* method = methods[i];

		generate_class_method(codegen, class_name, method);
	}
}

static void merge_class_vtable(AsmArea* table_area)
{
	for (int i = 0; i < table_area->lines_length; i++)
	{
		char* line = table_area->lines[i];

		add_line_to_area(data_section, line);
	}
}

static void generate_class_offsets(CodeGen* codegen, char* identifier, ClassOffsets* offsets, Node** fields, int fields_count, int* offset)
{
	for (int i = 0; i < fields_count; i++)
	{
		Node* curr = fields[i];

		if (curr->declare_node.declare.is_static)
		{
			continue;
		}
		
		const int BYTES_ALIGNMENT_SIZE = 8;

		Type* type = curr->declare_node.declare.var_type;
		char* identifier = curr->declare_node.declare.identifier;
		
		int size = analyzer_get_type_size(type, codegen->scope);
		int aligned = (size % BYTES_ALIGNMENT_SIZE == 0) ? size : ((size / BYTES_ALIGNMENT_SIZE) + 1) * BYTES_ALIGNMENT_SIZE;

		FieldEntry* entry = create_field_entry(codegen, identifier, aligned, *offset, type);
		add_entry_to_offsets(offsets, entry);

		printf("%d, %s\n", entry->field_offset, entry->field_name);
		
		*offset += aligned;
	}
}

static void setup_class_offsets(CodeGen* codegen, char* identifier, Node* node)
{
	const int INITIAL_CLASS_OFFSET = 16; // 8 bytes do ptr pra vtable + 8 bytes do id da class pra checks em run-time
	int offset = INITIAL_CLASS_OFFSET;

	ClassOffsets* curr_offsets = NULL;
	Symbol* curr = analyzer_find_symbol_from_scope(identifier, codegen->scope, 0, 0, 1, 0);
	
	Symbol* arr[64];
	int count = 0;

	while (curr != NULL)
	{
		arr[count] = curr;
		
		curr = curr->symbol_class->super;
		count++;
	}

	for (int i = count - 1; i >= 0; i--)
	{
		Symbol* curr = arr[i];
		
		ClassOffsets* offsets = create_class_offsets((char*) curr->symbol_class->identifier, offset);

		generate_class_offsets(codegen, (char*) curr->symbol_class->identifier, offsets, curr->symbol_class->fields, curr->symbol_class->field_count, &offset);

		offsets->parent = curr_offsets;
		curr_offsets = offsets;

		if (i == 0)
		{
			add_offsets_to_table(class_offsets_table, offsets);
		}
	}
}

void generate_class(CodeGen* codegen, Node* node, AsmArea* area) 
{
	char* identifier = node->class_node.class_node.identifer;
	Symbol* class_symbol = analyzer_find_symbol_from_scope(identifier, codegen->scope, 0, 0, 1, 0);
	AsmArea* table = generate_class_vtable(codegen, class_symbol, class_symbol->symbol_class->class_v_table);

	merge_class_vtable(table);

	SymbolTable* temp = codegen->scope;
	codegen->scope = class_symbol->symbol_class->class_scope;
	
	setup_class_offsets(codegen, identifier, node);
	
	if (node->class_node.class_node.constructor != NULL)
	{
		generate_class_constructor(codegen, identifier, node->class_node.class_node.constructor);
	}

	generate_class_fields(codegen, identifier, node->class_node.class_node.var_declare_list, node->class_node.class_node.var_count);
	generate_class_methods(codegen, identifier, node->class_node.class_node.func_declare_list, node->class_node.class_node.func_count);

	codegen->scope = temp;
}