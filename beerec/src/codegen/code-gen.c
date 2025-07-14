#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "../analyzer/analyzer.h"
#include "code-gen.h"
#include "stdlib.h"

AsmArea* data_section;
AsmArea* bss_section;
AsmArea* rodata_section;

ConstantTable* constant_table;

/**
 * TODO: 
 * - Gerar valores pra rodata (local constants)
 * - Implementar arrays melhor no code gen.
 */

static void code_gen_node(CodeGen* code_gen, Node* node, int scope_depth, AsmArea* area);
static void add_line_to_area(AsmArea* area, AsmLine* line);
static AsmArea* create_area();
static AsmLine* create_line();

static void setup_data()
{
	data_section = create_area();
	AsmLine* data_line = create_line();

	data_line->line = ".section  .data";
	add_line_to_area(data_section, data_line);
}

static void setup_bss()
{
	bss_section = create_area();

	AsmLine* bss_line = create_line();

	bss_line->line = ".section  .bss";
	add_line_to_area(data_section, bss_line);
}

static void setup_rodata()
{
	rodata_section = create_area();

	AsmLine* rodata_line = create_line();

	rodata_line->line = ".section  .rodata";
	add_line_to_area(rodata_section, rodata_line);
}

static void setup_constant_table()
{
	constant_table = malloc(sizeof(ConstantTable));

	constant_table->constants = malloc(sizeof(Constant*) * 4);
	constant_table->constants_capacity = 4;
	constant_table->constants_count = 0;
}

static void setup_code_gen(CodeGen* code_gen)
{
	setup_data();
	setup_bss();
	setup_rodata();
	setup_constant_table();
}

static AsmLine* create_line()
{
	AsmLine* line = malloc(sizeof(AsmLine));

	line->line = NULL;

	return line;
}

static AsmLine* generate_label(char* name)
{
	AsmLine* line = create_line();
	
	char buff[50];
	sprintf(buff, "%s:", name);
	
	line->line = strdup(buff);

	return line;
}

static AsmArea* create_area()
{
	AsmArea* area = malloc(sizeof(AsmArea));

	area->lines = malloc(sizeof(AsmLine*) * 4);
	area->lines_count = 0;
	area->lines_capacity = 4;

	area->lines[0] = NULL;

	return area;
}

static Constant* create_constant()
{
	Constant* constant = malloc(sizeof(Constant));
	constant->number = constant_table->constants_count - 1;
	
	AsmArea* area = create_area();

	char buff[64];
	snprintf(buff, 64, "LC%d:", constant->number);

	add_line_to_area(area, generate_label(buff));
	
	constant->area = area;

	return constant;
}

static void add_constant_to_table(Constant* constant)
{
	if (constant_table->constants_capacity <= constant_table->constants_count + 1)
	{
		constant_table->constants_capacity *= 2;

		constant_table->constants = realloc(constant_table->constants, sizeof(Constant*) * constant_table->constants_capacity);

		if (constant_table->constants == NULL)
		{
			printf("[CodeGen] [Debug] Failed to realloc memory for constant table...\n");
			exit(1);
		}
	}

	constant_table->constants[constant_table->constants_count] = constant;
	constant_table->constants_count++;
	constant_table->constants[constant_table->constants_count] = NULL;
}

static void add_line_to_area(AsmArea* area, AsmLine* line)
{
	if (area->lines_capacity <= area->lines_count + 1)
	{
		area->lines_capacity *= 2;

		area->lines = realloc(area->lines, sizeof(AsmLine*) * area->lines_capacity);

		if (area->lines == NULL)
		{
			printf("[CodeGen] [Debug] Failed to realloc memory for area...\n");
			exit(1);
		}
	}

	area->lines[area->lines_count] = line;
	area->lines_count++;
	area->lines[area->lines_count] = NULL;
}

static char* repeat_tab(int times)
{
	if (times == 0)
	{
		char* str = malloc(1);
		str[0] = '\0';

		return str;
	}

	int size = times + 1;
	char* str = malloc(size);

	int i = 0;

	while (i < times)
	{
		str[i] = '\t';

		i++;
	}

	str[i] = '\0';

	return str;
}

static void generate_local_variable_declaration(CodeGen* code_gen, Node* node, int scope_depth, Symbol* symbol, AsmArea* area)
{
	int offset = symbol->symbol_variable->offset;
	
	code_gen_node(code_gen, node->declare_node.declare.default_value, scope_depth, area);

	char* indent = repeat_tab(scope_depth);

	AsmLine* line = create_line();
	
	char buffer[64];
	snprintf(buffer, 64, "%smov [rbp %+d], eax", indent, offset);

	line->line = strdup(buffer);

	free(indent);
	
	add_line_to_area(area, line);
}

char* get_type_size(VarType type)
{
	switch (type)
	{
		case TYPE_BOOL:
		{
			return "db";
		}
		
		case TYPE_CHAR:
		{
			return "db";
		}

		case TYPE_INT:
		{
			return "dd";
		}

		case TYPE_FLOAT:
		{
			return "dd";
		}

		case TYPE_DOUBLE:
		{
			return "dq";
		}

		case TYPE_PTR:
		{
			return "dq";
		}

		case TYPE_ARRAY:
		{
			return NULL;
		}

		default:
		{
			exit(1);
		}
	}
}

static uint32_t float_to_bits(float number)
{
	uint32_t bits;
    memcpy(&bits, &number, sizeof(bits));

    return bits;
}

static uint64_t double_to_bits(double number)
{
	uint64_t bits;
    memcpy(&bits, &number, sizeof(bits));

    return bits;
}

static char* get_global_literal_value(Node* node)
{
	LiteralNode* literal = &node->literal_node.literal;
	
	char buff[128];
	
	switch (node->literal_node.literal.literal_type->type)
	{
		case TYPE_BOOL:
		{
			return literal->bool_value ? "1" : "0";
		}

		case TYPE_INT:
		{
			snprintf(buff, 64, "%d", literal->int_value);
			return strdup(buff);
		}

		case TYPE_CHAR:
		{
			snprintf(buff, 64, "%d", (int) literal->char_value);
			
			return strdup(buff);
		}

		case TYPE_FLOAT:
		{
			uint32_t bits = float_to_bits(literal->float_value);
			snprintf(buff, 64, "0x%08X", bits);
			
			return strdup(buff);
		}

		case TYPE_DOUBLE:
		{
			uint64_t bits = double_to_bits(literal->double_value);
			snprintf(buff, 64, "0x%016llX", (unsigned long long) bits);
			
			return strdup(buff);
		}

		default:
		{
			exit(1);
		}
	}
}

/**
 * TODO: Terminar isso...
 */
static Constant* generate_global_array_constant(CodeGen* code_gen, Node* node)
{
	return NULL;
}

/**
 * TODO: Terminar isso...
 */
static void generate_global_array_variable(CodeGen* code_gen, Node* node, int scope_depth, Symbol* symbol, AsmArea* area)
{
	
}

static void generate_global_bss_variable(CodeGen* code_gen, Node* node, int scope_depth, Symbol* symbol, AsmArea* area)
{
	AsmLine* label = generate_label(node->declare_node.declare.identifier);
	AsmLine* line = create_line();

	char* type_size = get_type_size(node->declare_node.declare.var_type->type);

	char buffer[64];
	snprintf(buffer, 64, "	.%s %s", type_size, "0");

	line->line = strdup(buffer);

	add_line_to_area(area, label);
	add_line_to_area(area, line);
}

static void generate_global_data_variable(CodeGen* code_gen, Node* node, int scope_depth, Symbol* symbol, AsmArea* area)
{
	AsmLine* label = generate_label(node->declare_node.declare.identifier);
	AsmLine* line = create_line();

	char* type_size = get_type_size(node->declare_node.declare.var_type->type);

	const char* value = get_global_literal_value(node->declare_node.declare.default_value);

	if (value == NULL)
	{
		exit(1);
	}

	char buffer[64];
	snprintf(buffer, 64, "	.%s %s", type_size, value);

	line->line = strdup(buffer);

	add_line_to_area(area, label);
	add_line_to_area(area, line);
}

static void generate_global_variable_declaration(CodeGen* code_gen, Node* node, int scope_depth, Symbol* symbol, AsmArea* area)
{
	if (node->declare_node.declare.default_value != NULL)
	{
		generate_global_data_variable(code_gen, node, scope_depth, symbol, area);
	}
	else
	{
		generate_global_bss_variable(code_gen, node, scope_depth, symbol, area);
	}
}

static void generate_variable_declaration(CodeGen* code_gen, Node* node, int scope_depth, AsmArea* area)
{
	char* identifier = node->declare_node.declare.identifier;
	Symbol* symbol = analyzer_find_symbol_from_scope(identifier, code_gen->scope, 1, 0, 0, 0);

	if (!symbol->symbol_variable->is_global)
	{
		generate_local_variable_declaration(code_gen, node, scope_depth, symbol, area);
	}
	else
	{
		generate_global_variable_declaration(code_gen, node, scope_depth, symbol, area);
	}
}

static void generate__function_declaration(CodeGen* code_gen, Node* node, int scope_depth)
{

}

static void code_gen_node(CodeGen* code_gen, Node* node, int scope_depth, AsmArea* area)
{
	switch (node->type)
	{
		case NODE_DECLARATION:
		{
			generate_variable_declaration(code_gen, node, scope_depth, area);

			return;
		}

		case NODE_FUNCTION:
		{
			
		}

		default:
		{
			printf("[CodeGen] [Debug] Node not implemented: %d...\n", node->type);
			exit(1);
		}
	}
}