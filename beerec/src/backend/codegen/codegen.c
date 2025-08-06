#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "codegen.h"
#include "impl/oop/codegen-class.h"
#include "impl/statements/assign/fields/codegen-field-assgn.h"
#include "impl/statements/calls/codegen-method-call.h"
#include "impl/statements/declaration/fields/codegen-field-decl.h"
#include "impl/statements/declaration/methods/codegen-method-decl.h"
#include "impl/statements/if/codegen-if.h"
#include "impl/statements/return/codegen-return.h"

extern char* get_literal_value(LiteralNode* literal);

ConstantTable* constant_table = NULL;
AsmArea* externs_section = NULL;
AsmArea* text_section = NULL;
AsmArea* data_section = NULL;
AsmArea* rodata_section = NULL;
AsmArea* bss_section = NULL;
ExternTable* extern_table = NULL;
ClassOffsetsTable* class_offsets_table = NULL;

AsmReturn* create_asm_return(char* value, Type* type)
{
	AsmReturn* asm_ret = malloc(asm_return_size);

	asm_ret->result = strdup(value);
	asm_ret->type = type;
	asm_ret->is_reg = 0;

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
	add_line_to_area(text_section_area, "");
	add_line_to_area(text_section_area, "global	main");

	text_section = text_section_area;
}

static void setup_bss_section()
{
	AsmArea* bss_section_area = create_area();
	add_line_to_area(bss_section_area, "section	.bss");

	bss_section = bss_section_area;
}

static void setup_data_section()
{
	AsmArea* data_section_area = create_area();
	add_line_to_area(data_section_area, "section	.data");

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

static int check_externs_cache(ExternEntry* entry)
{
	for (int i = 0; i < extern_table->externs_length; i++)
	{
		ExternEntry* curr = extern_table->externs[i];
		
		if (strcmp(entry->label, curr->label) == 0) 
		{
			return 1;
		}
	}

	return 0;
}

void add_extern_entry_to_table(ExternEntry* entry)
{
	if (check_externs_cache(entry))
	{
		return;
	}
	
	if (extern_table->externs_length <= extern_table->externs_capacity)
	{
		extern_table->externs_capacity *= 2;
		extern_table->externs = malloc(sizeof(ExternEntry*) * extern_table->externs_capacity);
	}

	extern_table->externs[extern_table->externs_length] = entry;
	extern_table->externs_length++;
}

ExternEntry* create_extern_entry(char* label)
{
	ExternEntry* entry = malloc(sizeof(ExternEntry));
	
	if (entry == NULL)
	{
		exit(1);
	}
	
	entry->label = strdup(label);

	return entry;
}

static void setup_extern_table()
{
	ExternTable* table = malloc(sizeof(ExternTable));
	
	table->externs = malloc(sizeof(ExternEntry*) * 4);
	table->externs_capacity = 4;
	table->externs_length = 0;

	extern_table = table;
}

static void setup_class_offsets_table()
{
	class_offsets_table = create_class_offsets_table();
}

void setup_codegen(Module* module, CodeGen* codegen)
{
	codegen->module = module;
	codegen->scope = module->global_scope;
	codegen->inner_class = 0;

	setup_class_offsets_table();
	setup_bss_section();
	setup_data_section();
	setup_rodata_section();
	setup_constant_table();
	setup_extern_table();
	setup_text_section();
	setup_externs();
}

static void print_constants()
{
	for (int i = 0; i < constant_table->constant_length; i++)
	{
		Constant* curr = constant_table->constants[i];

		char buff[64];
		snprintf(buff, 64, ".LC%d: %s", curr->id, curr->value);

		add_line_to_area(rodata_section, buff);
	}
}

static void print_area(AsmArea* area)
{
	for (int i = 0; i < area->lines_length; i++)
	{
		char* curr = area->lines[i];
		printf("%s\n", curr);
	}

	printf("\n");
}

void print_code_generated(CodeGen* codegen)
{
	print_constants();
	print_area(externs_section);
	print_area(bss_section);
	print_area(data_section);
	print_area(rodata_section);
	print_area(text_section);
}

void generate_node(CodeGen* codegen, Node* node, AsmArea* area)
{
	switch (node->type)
	{
		case NODE_FUNCTION:
		{
			generate_method_declaration(codegen, node, area, 0, NULL);

			return;
		}

		case NODE_DECLARATION:
		{
			generate_field_declaration(codegen, node, area);

			return;
		}

		case NODE_VARIABLE_ASSIGN:
		{
			generate_field_assign(codegen, node, area);

			return;
		}

		case NODE_RETURN:
		{
			generate_return(codegen, node, area);

			return;
		}

		case NODE_IF:
		{
			generate_if(codegen, node, area);

			return;
		}

		case NODE_FUNCTION_CALL:
		{
			generate_method_call(codegen, node, area, 0, 0);

			return;
		}

		case NODE_CLASS:
		{
			generate_class(codegen, node, area);

			return;
		}
		
		default:
		{
			printf("Invalid node while generating node: %d...\n", node->type);
			exit(1);
		}
	}
}