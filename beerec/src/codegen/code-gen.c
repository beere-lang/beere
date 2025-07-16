#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "../analyzer/analyzer.h"
#include "../parser/parser.h"
#include "code-gen.h"
#include "stdlib.h"

AsmArea* data_section;
AsmArea* bss_section;
AsmArea* rodata_section;
AsmArea* text_section;

ConstantTable* constant_table;

/**
 * TODO: 
 * - Adicionar suporte a parametros
 * - Adicionar suporte a function calls (com argumentos)
 * - Reservar espaço suficiente pra stack armazenar variaveis locais
 * - Implementar arrays melhor no code gen.
 * 
 * Estrategia pra arrays inicializadas globalmente:
 *  - Alocar a array no entry point antes de tudo, ja que não é possivel realocar na data
 */

static AsmReturn* generate_expression(CodeGen* code_gen, Node* node, int scope_depth, AsmArea* area, int force_reg, int prefer_secondary);
static void code_gen_node(CodeGen* code_gen, Node* node, int scope_depth, AsmArea* area);
static void add_line_to_area(AsmArea* area, AsmLine* line);
static AsmArea* create_area();
static AsmLine* create_line();

static AsmReturn* create_asm_return(char* reg, Type* content_type)
{
	AsmReturn* asm_return = malloc(sizeof(AsmReturn));

	asm_return->reg = strdup(reg);
	asm_return->type = content_type;

	return asm_return;
}

static void setup_data()
{
	data_section = create_area();
	AsmLine* data_line = create_line();

	data_line->line = "section	.data";
	add_line_to_area(data_section, data_line);
}

static void setup_bss()
{
	bss_section = create_area();

	AsmLine* bss_line = create_line();

	bss_line->line = "section	.bss";
	add_line_to_area(bss_section, bss_line);
}

static void setup_rodata()
{
	rodata_section = create_area();

	AsmLine* rodata_line = create_line();

	rodata_line->line = "section	.rodata";
	add_line_to_area(rodata_section, rodata_line);
}

static void setup_text()
{
	text_section = create_area();

	AsmLine* text_line = create_line();
	text_line->line = "section	.text";

	add_line_to_area(text_section, text_line);
}

static void setup_entry_point()
{
	AsmLine* entry_line = create_line();
	entry_line->line = "global	main";

	add_line_to_area(text_section, entry_line);
}

static void setup_constant_table()
{
	constant_table = malloc(sizeof(ConstantTable));

	constant_table->constants = malloc(sizeof(Constant*) * 4);
	constant_table->constants_capacity = 4;
	constant_table->constants_count = 0;
}

void setup_code_gen(CodeGen* code_gen, Module* module)
{
	code_gen->scope = module->global_scope;
	
	setup_data();
	setup_bss();
	setup_rodata();
	setup_text();
	setup_entry_point();
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

static void add_constant_area_to_rodata(Constant* constant)
{
	AsmArea* constant_area = constant->area;

	for (int i = 0; i < constant_area->lines_count; i++)
	{
		AsmLine* line = constant_area->lines[i];
		add_line_to_area(rodata_section, line);
	}
}

static void add_constants_to_rodata()
{
	for (int i = 0; i < constant_table->constants_count; i++)
	{
		Constant* constant = constant_table->constants[i];
		add_constant_area_to_rodata(constant);
	}
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

/**
 * TODO: Adicionar um check, pra caso o valor e o tipo seja o mesmo de uma ja existente, ela usa a ja existente
 */
static Constant* create_constant()
{
	Constant* constant = malloc(sizeof(Constant));
	constant->number = constant_table->constants_count;
	
	AsmArea* area = create_area();

	char buff[64];
	snprintf(buff, 64, ".LC%d", constant->number);

	add_line_to_area(area, generate_label(buff));
	
	constant->area = area;

	add_constant_to_table(constant);

	return constant;
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
	
	AsmReturn* ret = generate_expression(code_gen, node->declare_node.declare.default_value, scope_depth, area, 0, 0);

	AsmLine* line = create_line();
	
	char buffer[64];
	snprintf(buffer, 64, "	mov	[rbp %+d], %s", offset, ret->reg);

	line->line = strdup(buffer);

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
			return strdup(literal->bool_value ? "1" : "0");
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

static Constant* generate_constant(Node* node)
{
	LiteralNode* literal = &node->literal_node.literal;
	
	Constant* constant = create_constant();
	AsmArea* area = constant->area;

	char buff[64];

	char* value = get_global_literal_value(node);

	snprintf(buff, 64, "	%s %s", get_type_size(literal->literal_type->type), value);

	free(value);
	
	AsmLine* literal_line = create_line();
	literal_line->line = strdup(buff);

	add_line_to_area(area, literal_line);

	return constant;
}

static char* get_literal_value(Node* node, AsmArea* area, int force_reg, int prefer_second)
{
	LiteralNode* literal = &node->literal_node.literal;
	
	char buff[128];
	
	switch (node->literal_node.literal.literal_type->type)
	{
		case TYPE_BOOL:
		{
			if (force_reg)
			{
				AsmLine* line = create_line();

				char* temp = prefer_second ? "bl" : "al";

				char _buff[64];
				snprintf(_buff, 64, "	mov	%s, %s", temp, literal->bool_value ? "1" : "0");
			
				line->line = strdup(_buff);

				add_line_to_area(area, line);
				
				return strdup(temp);
			}
			
			return strdup(literal->bool_value ? "1" : "0");
		}

		case TYPE_INT:
		{
			if (force_reg)
			{
				AsmLine* line = create_line();

				char* temp = prefer_second ? "ebx" : "eax";

				char _buff[64];
				snprintf(_buff, 64, "	mov	%s, %d", temp, literal->int_value);
			
				line->line = strdup(_buff);

				add_line_to_area(area, line);
				
				return strdup(temp);
			}
			
			snprintf(buff, 64, "%d", literal->int_value);
			return strdup(buff);
		}

		case TYPE_CHAR:
		{
			if (force_reg)
			{
				AsmLine* line = create_line();

				char* temp = prefer_second ? "bl" : "al";

				char _buff[64];
				snprintf(_buff, 64, "	mov	%s, %d", temp, (int) literal->char_value);
			
				line->line = strdup(_buff);

				add_line_to_area(area, line);
				
				return strdup(temp);
			}

			snprintf(buff, 64, "%d", (int) literal->char_value);
			return strdup(buff);
		}

		case TYPE_FLOAT:
		{
			Constant* constant = generate_constant(node);
			
			char* temp = prefer_second ? "xmm1" : "xmm0";
			
			snprintf(buff, 64, "[.LC%d]", constant->number);

			AsmLine* line = create_line();

			char _buff[64];
			snprintf(_buff, 64, "	movss	%s, [.LC%d]", temp, constant->number);

			line->line = strdup(_buff);

			add_line_to_area(area, line);
			
			return strdup(temp);
		}

		case TYPE_DOUBLE:
		{
			Constant* constant = generate_constant(node);
			
			char* temp = prefer_second ? "xmm1" : "xmm0";
			
			snprintf(buff, 64, "[.LC%d]", constant->number);

			AsmLine* line = create_line();
			
			char _buff[64];
			snprintf(_buff, 64, "	movsd	%s, [.LC%d]", temp, constant->number);

			line->line = strdup(_buff);

			add_line_to_area(area, line);
			
			return strdup(temp);
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
static void generate_global_array_initialization(CodeGen* code_gen, Node* node)
{
	
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

	char buffer[64];
	snprintf(buffer, 64, "	resb %d", analyzer_get_type_size(node->declare_node.declare.var_type, code_gen->scope));

	line->line = strdup(buffer);

	add_line_to_area(bss_section, label);
	add_line_to_area(bss_section, line);
}

static void generate_global_data_variable(CodeGen* code_gen, Node* node, int scope_depth, Symbol* symbol, AsmArea* area)
{
	AsmLine* label = generate_label(node->declare_node.declare.identifier);
	AsmLine* line = create_line();

	char* type_size = get_type_size(node->declare_node.declare.var_type->type);

	char* value = get_global_literal_value(node->declare_node.declare.default_value);

	if (value == NULL)
	{
		exit(1);
	}

	char buffer[64];
	snprintf(buffer, 64, "	%s %s", type_size, value);

	line->line = strdup(buffer);

	free(value);

	add_line_to_area(data_section, label);
	add_line_to_area(data_section, line);
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

static void add_function_area_to_text_section(AsmArea* area)
{
	for (int i = 0; i < area->lines_count; i++)
	{
		AsmLine* line = area->lines[i];
		add_line_to_area(text_section, line);
	}
}

static void generate_block_code(CodeGen* code_gen, Node* node, int scope_depth, AsmArea* area)
{
	Node* next = node->block_node.block.statements->head;

	while (next != NULL)
	{
		code_gen_node(code_gen, next, scope_depth, area);

		next = next->next;
	}
}

static void generate_function_setup(AsmArea* area)
{
	AsmLine* base_setup = create_line();
	AsmLine* stack_setup = create_line();

	base_setup->line = "	push	rbp";
	stack_setup->line = "	mov	rbp, rsp";

	add_line_to_area(area, base_setup);
	add_line_to_area(area, stack_setup);
}

static void generate_function_unsetup(AsmArea* area)
{
	AsmLine* base_unsetup = create_line();
	AsmLine* stack_unsetup = create_line();
	AsmLine* ret = create_line();

	base_unsetup->line = strdup("	mov	rsp, rbp");
	stack_unsetup->line = strdup("	pop	rbp");
	ret->line = strdup("	ret");

	add_line_to_area(area, base_unsetup);
	add_line_to_area(area, stack_unsetup);
	add_line_to_area(area, ret);
}

/**
 * TODO: Terminar isso
 */
static void generate_param_setup(AsmArea* area)
{
	
}

static void generate_stack_space(int offset, AsmArea* area)
{
	AsmLine* line = create_line();
	
	char buff[50];
	
	snprintf(buff, 50, "	sub	rsp, %d", offset); // diminui o valor da memoria de rsp (pra "alocar" memoria pra stack)
	
	line->line = strdup(buff);

	add_line_to_area(area, line);
}

static void generate_function_declaration(CodeGen* code_gen, Node* node, int scope_depth, AsmArea* area)
{
	AsmArea* function_area = create_area();
	AsmLine* label_line = generate_label(strdup(node->function_node.function.identifier));

	char* identifier = node->function_node.function.identifier;
	Symbol* symbol = analyzer_find_symbol_from_scope(identifier, code_gen->scope, 0, 1, 0, 0);
	
	add_line_to_area(function_area, label_line);

	generate_function_setup(function_area);

	generate_stack_space(symbol->symbol_function->total_offset * (-1), function_area);

	generate_param_setup(function_area);

	Node* block = node->function_node.function.block_node;

	SymbolTable* temp = code_gen->scope;
	
	code_gen->scope = symbol->symbol_function->scope;

	generate_block_code(code_gen, block, scope_depth + 1, function_area);

	code_gen->scope = temp;

	generate_function_unsetup(function_area);

	add_function_area_to_text_section(function_area);
}

static char* get_local_variable_reference(Node* variable, SymbolTable* scope)
{
	char* identifier = variable->variable_node.variable.identifier;
	Symbol* symbol = analyzer_find_symbol_from_scope(identifier, scope, 1, 0, 0, 0);

	char buff[64];

	snprintf(buff, 64, "[rbp%+d]", symbol->symbol_variable->offset);

	return strdup(buff);
}

static AsmReturn* get_variable_reference(char* _identifier, SymbolTable* scope)
{
	char* identifier = _identifier;
	Symbol* symbol = analyzer_find_symbol_from_scope(identifier, scope, 1, 0, 0, 0);

	if (symbol->symbol_variable->is_global)
	{
		char buff[64];

		snprintf(buff, 64, "%s", identifier);

		AsmReturn* asm_return = create_asm_return(buff, symbol->symbol_variable->type);
		
		return asm_return;
	}
	else
	{
		char buff[64];

		snprintf(buff, 64, "rbp%+d", symbol->symbol_variable->offset);

		AsmReturn* asm_return = create_asm_return(buff, symbol->symbol_variable->type);

		return asm_return;
	}
}

static AsmReturn* generate_load_var_to_reg(CodeGen* code_gen, char* identifier, int scope_depth, AsmArea* area, int prefer_secondary)
{
	AsmReturn* ref = get_variable_reference(identifier, code_gen->scope);

	char buff[64];

	char* temp = prefer_secondary ? "rbx" : "rax";

	snprintf(buff, 64, "	mov	%s, [%s]", temp, ref->reg);

	AsmLine* reg_mov_line = create_line();
	reg_mov_line->line = strdup(buff);

	free(ref);

	add_line_to_area(area, reg_mov_line);

	AsmReturn* asm_return = create_asm_return(temp, ref->type);

	return asm_return;
}

static AsmReturn* get_value(CodeGen* code_gen, Node* node, int scope_depth, AsmArea* area, int force_reg, int prefer_second)
{
	AsmReturn* value = NULL;
	
	if (node->type == NODE_LITERAL)
	{
		char* temp = get_literal_value(node, area, force_reg, prefer_second);

		value = create_asm_return(temp, node->literal_node.literal.literal_type);
	}

	return value;
}

static void generate_local_variable_assign(CodeGen* code_gen, Node* node, int scope_depth, AsmArea* area)
{
	AsmReturn* ret = generate_expression(code_gen, node->variable_assign_node.variable_assign.assign_value, scope_depth, area, 0, 0);

	Node* left = node->variable_assign_node.variable_assign.left;
	char* reference = get_local_variable_reference(left, code_gen->scope);
	
	char buff[64];
	snprintf(buff, 64, "	mov	%s, %s", reference, ret->reg);

	free(ret);
	free(reference);

	AsmLine* line = create_line();
	line->line = strdup(buff);

	add_line_to_area(area, line);
}

static void generate_global_variable_assign(CodeGen* code_gen, Node* node, int scope_depth, AsmArea* area)
{
	AsmReturn* ret = generate_expression(code_gen, node->variable_assign_node.variable_assign.assign_value, scope_depth, area, 0, 0);

	Node* left = node->variable_assign_node.variable_assign.left;
	
	char buff[64];
	snprintf(buff, 64, "	mov	[%s], %s", left->variable_node.variable.identifier, ret->reg);

	AsmLine* line = create_line();
	line->line = strdup(buff);

	free(ret);

	add_line_to_area(area, line);
}

static void generate_variable_assign(CodeGen* code_gen, Node* node, int scope_depth, AsmArea* area)
{
	char* identifier = node->variable_assign_node.variable_assign.left->variable_node.variable.identifier;
	Symbol* symbol = analyzer_find_symbol_from_scope(identifier, code_gen->scope, 1, 0, 0, 0);

	if (!symbol->symbol_variable->is_global)
	{
		generate_local_variable_assign(code_gen, node, scope_depth, area);
	}
	else
	{
		generate_global_variable_assign(code_gen, node, scope_depth, area);
	}
}

void code_gen_global(CodeGen* code_gen, Node* node)
{
	code_gen_node(code_gen, node, 0, text_section);
}

void print_code_generated()
{
	add_constants_to_rodata();
	
	for (int i = 0; i < data_section->lines_count; i++)
	{
		printf("%s\n", data_section->lines[i]->line);
	}
	
	printf("\n");

	for (int i = 0; i < rodata_section->lines_count; i++)
	{
		printf("%s\n", rodata_section->lines[i]->line);
	}

	printf("\n");

	for (int i = 0; i < bss_section->lines_count; i++)
	{
		printf("%s\n", bss_section->lines[i]->line);
	}

	printf("\n");

	for (int i = 0; i < text_section->lines_count; i++)
	{
		printf("%s\n", text_section->lines[i]->line);
	}
}

//====------------------------------------------------------------------------------------------------------------------------------------------====//

static AsmReturn* generate_multiply(CodeGen* code_gen, char* lreg, char* rreg, Node* node, AsmArea* area)
{

}

static AsmReturn* generate_add(CodeGen* code_gen, AsmReturn* lreg, AsmReturn* rreg, Node* node, AsmArea* area)
{
	char* temp = "add";

	if (lreg->type->type == TYPE_FLOAT || lreg->type->type == TYPE_DOUBLE)
	{
		temp = (lreg->type->type == TYPE_DOUBLE) ? "addsd" : "addss";
	}
	
	AsmLine* line = create_line();

	char buff[64];
	
	snprintf(buff, 64, "	%s	%s, %s", temp, lreg->reg, rreg->reg);

	line->line = strdup(buff);

	add_line_to_area(area, line);

	Type* type = create_type(lreg->type->type, NULL);
	AsmReturn* ret = create_asm_return(lreg->reg, type);

	return ret;
}

static AsmReturn* generate_subtract(CodeGen* code_gen, AsmReturn* lreg, AsmReturn* rreg, Node* node, AsmArea* area)
{
	char* temp = "sub";

	if (lreg->type->type == TYPE_FLOAT || lreg->type->type == TYPE_DOUBLE)
	{
		temp = (lreg->type->type == TYPE_DOUBLE) ? "subsd" : "subss";
	}
	
	AsmLine* line = create_line();

	char buff[64];
	
	snprintf(buff, 64, "	%s	%s, %s", temp, lreg->reg, rreg->reg);

	line->line = strdup(buff);

	add_line_to_area(area, line);

	Type* type = create_type(lreg->type->type, NULL);
	AsmReturn* ret = create_asm_return(lreg->reg, type);

	return ret;
}

static char* generate_divided(CodeGen* code_gen, char* lreg, AsmReturn* rreg, Node* node, AsmArea* area)
{
	
}

static AsmReturn* generate_int_increment(AsmReturn* lreg, AsmArea* area, int prefer_second)
{
	char* temp = prefer_second ? "rbx" : "rax";
	
	AsmLine* _line = create_line();

	char _buff[64];
	snprintf(_buff, 64, "	mov	%s, [%s]", temp, lreg->reg);
	
	AsmLine* line = create_line();
	
	char buff[64];
	snprintf(buff, 64, "	inc	[%s]", lreg->reg);

	_line->line = strdup(_buff);
	line->line = strdup(buff);

	add_line_to_area(area, _line);
	add_line_to_area(area, line);

	Type* type = create_type(TYPE_INT, NULL);
	AsmReturn* ret = create_asm_return(temp, type);
		
	return ret;
}

static AsmReturn* generate_floating_increment(AsmReturn* lreg, AsmArea* area, int doub, int prefer_second)
{
	char* temp = doub ? "movsd" : "movss";
	char* _temp = doub ? "addsd" : "addss";

	char* __temp = prefer_second ? "xmm1" : "xmm0";
	
	AsmLine* _line = create_line();

	char _buff[64];
	snprintf(_buff, 64, "	%s	%s, [%s]", __temp, temp, lreg->reg);
	
	AsmLine* line = create_line();
	
	char buff[64];
	snprintf(buff, 64, "	%s	[%s]", _temp, lreg->reg);

	_line->line = strdup(_buff);
	line->line = strdup(buff);

	add_line_to_area(area, _line);
	add_line_to_area(area, line);

	Type* type = create_type((doub) ? TYPE_DOUBLE : TYPE_FLOAT, NULL);
	AsmReturn* ret = create_asm_return(__temp, type);
		
	return ret;
}

/**
 * RUST: Chupa meu pau rust, eu tenho e você não tem...
 */
static AsmReturn* generate_increment(CodeGen* code_gen, AsmReturn* lreg,  Node* node, AsmArea* area, int prefer_second)
{
	if (lreg->type->type == TYPE_INT)
	{
		return generate_int_increment(lreg, area, prefer_second);
	}
	else if (lreg->type->type == TYPE_FLOAT || lreg->type->type == TYPE_DOUBLE)
	{
		int doub = (lreg->type->type == TYPE_DOUBLE) ? 1 : 0;
		
		return generate_floating_increment(lreg, area, doub, prefer_second);
	}

	return NULL;
}

static AsmReturn* generate_int_decrement(AsmReturn* lreg, AsmArea* area, int prefer_second)
{
	char* temp = prefer_second ? "rbx" : "rax";
	
	AsmLine* _line = create_line();

	char _buff[64];
	snprintf(_buff, 64, "	mov	%s, [%s]", temp, lreg->reg);
	
	AsmLine* line = create_line();
	
	char buff[64];
	snprintf(buff, 64, "	dec	[%s]", lreg->reg);

	_line->line = strdup(_buff);
	line->line = strdup(buff);

	add_line_to_area(area, _line);
	add_line_to_area(area, line);

	Type* type = create_type(TYPE_INT, NULL);
	AsmReturn* ret = create_asm_return(temp, type);
		
	return ret;
}

static AsmReturn* generate_floating_decrement(AsmReturn* lreg, AsmArea* area, int doub, int prefer_second)
{
	char* temp = doub ? "movsd" : "movss";
	char* _temp = doub ? "subsd" : "subss";

	char* __temp = prefer_second ? "xmm1" : "xmm0";
	
	AsmLine* _line = create_line();

	char _buff[64];
	snprintf(_buff, 64, "	%s	%s, [%s]", __temp, temp, lreg->reg);
	
	AsmLine* line = create_line();
	
	char buff[64];
	snprintf(buff, 64, "	%s	[%s]", _temp, lreg->reg);

	_line->line = strdup(_buff);
	line->line = strdup(buff);

	add_line_to_area(area, _line);
	add_line_to_area(area, line);

	Type* type = create_type((doub) ? TYPE_DOUBLE : TYPE_FLOAT, NULL);
	AsmReturn* ret = create_asm_return(__temp, type);
		
	return ret;
}

/**
 * RUST: Chupa meu pau rust, eu tenho e você não tem...
 */
static AsmReturn* generate_decrement(CodeGen* code_gen, AsmReturn* lreg,  Node* node, AsmArea* area, int prefer_second)
{
	if (lreg->type->type == TYPE_INT)
	{
		return generate_int_decrement(lreg, area, prefer_second);
	}
	else if (lreg->type->type == TYPE_FLOAT || lreg->type->type == TYPE_DOUBLE)
	{
		int doub = (lreg->type->type == TYPE_DOUBLE) ? 1 : 0;
		
		return generate_floating_decrement(lreg, area, doub, prefer_second);
	}

	return NULL;
}

static AsmReturn* generate_operation(CodeGen* code_gen, Node* node, int scope_depth, AsmArea* area, int prefer_second)
{
	Node* left = node->operation_node.operation.left;
	Node* right = node->operation_node.operation.right;
	
	Type* ltype = analyzer_return_type_of_expression(NULL, left, code_gen->scope, NULL, 0, 0);
	Type* rtype = analyzer_return_type_of_expression(NULL, right, code_gen->scope, NULL, 0, 0);

	AsmReturn* lreg = NULL;
	
	if (node->operation_node.operation.op == TOKEN_OPERATOR_INCREMENT || node->operation_node.operation.op == TOKEN_OPERATOR_DECREMENT)
	{
		lreg = get_variable_reference(node->operation_node.operation.left->variable_node.variable.identifier, code_gen->scope);
	}
	else
	{
		lreg = generate_expression(code_gen, node->operation_node.operation.left, scope_depth, area, 1, 0);
	}

	AsmReturn* rreg = generate_expression(code_gen, node->operation_node.operation.right, scope_depth, area, 0, 1);

	switch (node->operation_node.operation.op)
	{
		case TOKEN_OPERATOR_PLUS:
		{
			AsmReturn* r = generate_add(code_gen, lreg, rreg, node, area);
			
			free(lreg);
			free(rreg);
			
			return r;
		}

		case TOKEN_OPERATOR_MINUS:
		{
			AsmReturn* r = generate_subtract(code_gen, lreg, rreg,  node, area);
			
			free(lreg);
			free(rreg);
			
			return r;
		}

		case TOKEN_OPERATOR_INCREMENT:
		{
			AsmReturn* r = generate_increment(code_gen, lreg, node, area, prefer_second);
			
			free(lreg);
			free(rreg);
			
			return r;
		}

		case TOKEN_OPERATOR_DECREMENT:
		{	
			AsmReturn* r = generate_decrement(code_gen, lreg, node, area, prefer_second);

			free(lreg);
			free(rreg);

			return r;
		}

		default:
		{
			exit(1);
		}
	}

	free(lreg);
	free(rreg);
}

static AsmReturn* generate_cast(CodeGen* code_gen, Node* node, int scope_depth, AsmArea* area, int force_reg, int prefer_second)
{
	/**
	 * TODO: Adicionar o modulo na struct do CodeGen.
	 */
	AsmReturn* expr = generate_expression(code_gen, node->cast_statement_node.cast_node.expression, scope_depth, area, force_reg, 0);
	Type* type = expr->type;

	char* reg = expr->reg;

	if (type->type == TYPE_DOUBLE && node->cast_statement_node.cast_node.cast_type->type == TYPE_INT)
	{
		char* temp = prefer_second ? "ebx" : "eax";
		
		AsmLine* line = create_line();
		
		char buff[64];
		snprintf(buff, 64, "	cvtsd2si	%s, %s", temp, reg);
					
		line->line = strdup(buff);
		
		add_line_to_area(area, line);

		Type* type = create_type(TYPE_INT, NULL);
		AsmReturn* ret = create_asm_return(temp, type);
		
		return ret;
	} 
	else if (type->type == TYPE_DOUBLE && node->cast_statement_node.cast_node.cast_type->type == TYPE_FLOAT)
	{
		char* temp = prefer_second ? "xmm1" : "xmm0";
		
		AsmLine* line = create_line();
		
		char buff[64];
		snprintf(buff, 64, "	cvtsd2ss	%s, %s", temp, reg);
					
		line->line = strdup(buff);
		
		add_line_to_area(area, line);
		
		Type* type = create_type(TYPE_FLOAT, NULL);
		AsmReturn* ret = create_asm_return(temp, type);
		
		return ret;
	} 
	else if (type->type == TYPE_FLOAT && node->cast_statement_node.cast_node.cast_type->type == TYPE_INT)
	{
		char* temp = prefer_second ? "ebx" : "eax";
		
		AsmLine* line = create_line();
		
		char buff[64];
		snprintf(buff, 64, "	cvtss2si	%s, %s", temp, reg);
					
		line->line = strdup(buff);
		
		add_line_to_area(area, line);
		
		Type* type = create_type(TYPE_INT, NULL);
		AsmReturn* ret = create_asm_return(temp, type);
		
		return ret;
	}
	else if (type->type == TYPE_FLOAT && node->cast_statement_node.cast_node.cast_type->type == TYPE_DOUBLE)
	{
		char* temp = prefer_second ? "xmm1" : "xmm0";
		
		AsmLine* line = create_line();
		
		char buff[64];
		snprintf(buff, 64, "	cvtss2sd	%s, %s", temp, reg);
					
		line->line = strdup(buff);
		
		add_line_to_area(area, line);
		
		Type* type = create_type(TYPE_DOUBLE, NULL);
		AsmReturn* ret = create_asm_return(temp, type);
		
		return ret;
	} 
	else if (type->type == TYPE_INT && node->cast_statement_node.cast_node.cast_type->type == TYPE_FLOAT)
	{
		char* temp = prefer_second ? "xmm1" : "xmm0";
		
		AsmLine* line = create_line();
		
		char buff[64];
		snprintf(buff, 64, "	cvtsi2ss	%s, %s", temp, reg);
			
		line->line = strdup(buff);
		
		add_line_to_area(area, line);
		
		Type* type = create_type(TYPE_FLOAT, NULL);
		AsmReturn* ret = create_asm_return(temp, type);
		
		return ret;
	}
	else if (type->type == TYPE_INT && node->cast_statement_node.cast_node.cast_type->type == TYPE_DOUBLE)
	{
		char* temp = prefer_second ? "xmm1" : "xmm0";
		
		AsmLine* line = create_line();
		
		char buff[64];
		snprintf(buff, 64, "	cvtsi2sd	%s, %s", temp, reg);

		line->line = strdup(buff);
		
		add_line_to_area(area, line);
		
		Type* type = create_type(TYPE_DOUBLE, NULL);
		AsmReturn* ret = create_asm_return(temp, type);
		
		return ret;
	}

	return NULL;
}

static AsmReturn* generate_expression(CodeGen* code_gen, Node* node, int scope_depth, AsmArea* area, int force_reg, int prefer_secondary)
{
	switch (node->type)
	{
		case NODE_IDENTIFIER:
		{
			AsmReturn* ret = generate_load_var_to_reg(code_gen, node->variable_node.variable.identifier, scope_depth, area, prefer_secondary);
			
			char* reg = prefer_secondary ? "rbx" :"rax";
			
			AsmReturn* asm_return = create_asm_return(reg, ret->type);
			
			return asm_return;
		}
		
		case NODE_LITERAL:
		{
			return get_value(code_gen, node, scope_depth, area, force_reg, prefer_secondary);
		}

		case NODE_OPERATION:
		{
			return generate_operation(code_gen, node, scope_depth, area, prefer_secondary);
		}

		case NODE_CAST:
		{
			return generate_cast(code_gen, node, scope_depth, area, force_reg, prefer_secondary);
		}

		default:
		{
			printf("[CodeGen] [Debug] Expression node not implemented: %d...\n", node->type);
			exit(1);
		}
	}
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

		case NODE_OPERATION:
		{
			generate_operation(code_gen, node, scope_depth, area, 0);

			return;
		}

		case NODE_FUNCTION:
		{
			generate_function_declaration(code_gen, node, scope_depth, area);

			return;
		}

		case NODE_VARIABLE_ASSIGN:
		{
			generate_variable_assign(code_gen, node, scope_depth, area);

			return;
		}

		case NODE_LITERAL:
		{
			return;
		}

		default:
		{
			printf("[CodeGen] [Debug] Node not implemented: %d...\n", node->type);
			exit(1);
		}
	}
}