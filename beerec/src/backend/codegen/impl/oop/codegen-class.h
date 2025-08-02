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
	char* class_name;

	int offset;
	
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

void generate_class(CodeGen* codegen, Node* node, AsmArea* area);

#endif