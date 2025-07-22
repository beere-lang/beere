#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "codegen.h"

static char* generate_size_type(VarType type)
{
	switch (type)
	{
		case TYPE_INT:
		{
			return strdup("dd");
		}

		case TYPE_FLOAT:
		{
			return strdup("dd");
		}

		case TYPE_DOUBLE:
		{
			return strdup("dq");
		}

		case TYPE_STRING:
		{
			return strdup("db");
		}
		
		default:
		{
			printf("[Codegen] Invalid constant value type...\n");
			exit(1);
		}
	}
}

static char* get_literal_value(LiteralNode* literal)
{
	char buff[64];
	
	switch (literal->literal_type->type)
	{
		case TYPE_INT:
		{
			snprintf(buff, 64, "%d", literal->int_value);
			return strdup(buff);
		}

		case TYPE_FLOAT:
		{
			union
			{
				float flt;
				uint32_t hex;
			} hex;

			hex.flt = literal->float_value;
			
			snprintf(buff, 64, "0x%x", hex.hex);
			return strdup(buff);
		}

		case TYPE_DOUBLE:
		{
			union
			{
				double dbl;
				uint64_t hex;
			} hex;

			hex.dbl = literal->double_value;
			
			snprintf(buff, 64, "0x%x", (unsigned int) hex.hex);
			return strdup(buff);
		}

		case TYPE_STRING:
		{
			return strdup(literal->string_value);
		}
		
		default:
		{
			printf("[Codegen] Invalid constant value type...\n");
			exit(1);
		}
	}
}

AsmReturn* create_asm_return(char* value, Type* type)
{
	AsmReturn* asm_ret = malloc(asm_return_size);

	asm_ret->result = strdup(value);
	asm_ret->type = type;

	return asm_ret;
}

AsmArea* create_area()
{
	AsmArea* area = malloc(asm_area_size);

	area->lines = malloc(lines_default_size);
	area->lines_capacity = 10;
	area->lines_length = 0;

	return area;
}

void add_line_to_area(AsmArea* area, char* line)
{
	if (area->lines_capacity <= area->lines_length)
	{
		area->lines_capacity *= 2;
		area->lines = realloc(area->lines, sizeof(char*) * area->lines_capacity);

		if (area->lines == NULL)
		{
			printf("[Codegen] Failed to realloc memory for area...\n");
			exit(1);
		}
	}

	area->lines[area->lines_length] = strdup(line);
	area->lines_length++;
}

AsmArea* create_area_with_label(char* label)
{
	AsmArea* area_with_label = create_area();

	char buff[64];
	snprintf(buff, 64, "%s:", label);

	add_line_to_area(area_with_label, buff);

	return area_with_label;
}

static void setup_externs()
{
	AsmArea* externs_area = create_area();

	externs_section = externs_area;
}

static void setup_text_section()
{
	AsmArea* text_section_area = create_area();
	add_line_to_area(text_section_area, "section	.text");

	text_section = text_section_area;
}

static void setup_bss_section()
{
	AsmArea* bss_section_area = create_area();
	add_line_to_area(bss_section, "section	.bss");

	bss_section = bss_section_area;
}

static void setup_data_section()
{
	AsmArea* data_section_area = create_area();
	add_line_to_area(data_section, "section	.data");

	data_section = data_section_area;
}

static void setup_rodata_section()
{
	AsmArea* rodata_section_area = create_area();
	add_line_to_area(rodata_section_area, "section	.rodata");

	rodata_section = rodata_section_area;
}

static void setup_constant_table()
{
	ConstantTable* table = malloc(sizeof(ConstantTable));

	table->constants = malloc(sizeof(Constant*) * 4);
	table->constants_capacity = 4;
	table->constant_length = 0;

	constant_table = table;
}

static void add_to_constant_table(Constant* constant)
{
	if (constant_table->constants_capacity <= constant_table->constant_length)
	{
		constant_table->constants_capacity *= 2;
		constant_table->constants = realloc(constant_table->constants, sizeof(Constant*) * constant_table->constant_length);
	}

	constant_table->constants[constant_table->constant_length] = constant;
	constant_table->constant_length++;
}

static Constant* check_constants_cache(Constant* constant)
{
	for (int i = 0; i < constant_table->constant_length; i++)
	{
		Constant* curr = constant_table->constants[i];

		if (strcmp(curr->value, constant->value) == 0)
		{
			return curr;
		}
	}

	return NULL;
}

static Constant* create_constant()
{
	Constant* constant = malloc(sizeof(Constant));
	
	constant->id = constant_table->constant_length;
	constant->value = NULL;

	return constant;
}

Constant* generate_constant(Node* literal)
{
	Constant* constant = create_constant();
	constant->value = get_literal_value(&literal->literal_node.literal);

	Constant* cache = check_constants_cache(constant);

	if (cache != NULL)
	{
		free(constant->value);
		free(constant);

		constant = cache;
	}
	else
	{
		add_to_constant_table(constant);
	}

	return constant;
}