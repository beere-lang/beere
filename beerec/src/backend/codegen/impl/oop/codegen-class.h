#ifndef CODEGEN_CLASS_H
#define CODEGEN_CLASS_H

#include "../../codegen.h"

typedef struct ClassOffsets ClassOffsets;
typedef struct FieldEntry FieldEntry;

struct FieldEntry
{
	char* field_name;
	int field_size;
	int field_offset;
};

struct ClassOffsets
{
	FieldEntry* entry;

	int fields_length;
	int fields_capacity;
};

struct ClassOffsetsTable
{
	char* class_name;
	
	ClassOffsets* offsets;
};

void generate_class(CodeGen* codegen, Node* node, AsmArea* area);

#endif