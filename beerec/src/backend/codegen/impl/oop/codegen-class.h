#ifndef CODEGEN_CLASS_H
#define CODEGEN_CLASS_H

#include "../../codegen.h"

typedef struct ClassOffsets ClassOffsets;
typedef struct FieldEntry FieldEntry;
typedef struct ClassOffsetsTable ClassOffsetsTable;

struct FieldEntry
{
	char* field_name;

	int field_size;
	int field_offset;
};

struct ClassOffsets
{
	ClassOffsets* parent;
	
	char* class_name;

	int start_offset;
	int end_offset;
	
	FieldEntry** fields;

	int fields_length;
	int fields_capacity;
};

struct ClassOffsetsTable
{
	
	ClassOffsets** class_offsets;

	int class_offsets_capacity;
	int class_offsets_length;
};

FieldEntry* create_field_entry(CodeGen* codegen, char* field_name, int size, int offset, Type* field_type);
void setup_instance_memory_alloc(CodeGen* codegen, Symbol* symbol, AsmArea* area);
ClassOffsets* find_class_offsets(ClassOffsetsTable* table, char* class_name);
void add_offsets_to_table(ClassOffsetsTable* table, ClassOffsets* offsets);
void add_entry_to_offsets(ClassOffsets* offsets, FieldEntry* entry);
void generate_class(CodeGen* codegen, Node* node, AsmArea* area);
ClassOffsets* create_class_offsets(char* class_name, int offset);
int find_field_offset(ClassOffsets* offsets, char* field_name);
ClassOffsetsTable* create_class_offsets_table();

#endif