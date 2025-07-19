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

ConstantTable* constant_table = NULL;
ClassTable* asm_class_table = NULL;

int actual_offset;

int flag_assign = 0;
int constructor_flag = 0;
int member_access_flag = 0;

char* class_func_flag = NULL;

char* arg_registers[] = { "rdi", "rsi", "rdx", "rcx", "r8", "r9" };

int ifs_count = 0;

AsmArea* ref_post_area = NULL;

/**
 * TODO - Completo:
 *  - Finalizar arrays e dar suporte pra arrays em variaveis globais (alocar no entry point)
 *  - Finalizar OOP (chamada de funções, v-table).
 *  - Adicionar loops (while e for)
 *  - Adicionar strings (dinamicas)
 *  - Adicionar o array push e o array pop no analyzer (só substituir prototype call quando for push ou pop em alguma array pela node respective)
 *  - Implementar modulos e fazer a linkagem correta
 *  - Adicionar interoperabilidade com C
 *  - Adicionar o package manager (Beeagle)
 *  - Adicionar o print (só mover a array de char da string pro rdi e dar syscall)
 *  - Criar a lib 'std'
 */

 /**
  * TODO - Pre Release:
  *  - Logs melhores
  *  - Adicionar o optimizer nas nodes
  */

static AsmReturn* generate_expression(CodeGen* code_gen, Node* node, int scope_depth, AsmArea* area, int force_reg, int prefer_secondary, int arg);
static AsmReturn* generate_expression(CodeGen* code_gen, Node* node, int scope_depth, AsmArea* area, int force_reg, int prefer_secondary, int arg);
static AsmReturn* generate_function_call(CodeGen* code_gen, Node* node, AsmArea* area, int prefer_second, int arg);
static int generate_setup_arguments(CodeGen* code_gen, Node* head, AsmArea* area, int jump_size);
static void code_gen_node(CodeGen* code_gen, Node* node, int scope_depth, AsmArea* area);
static void add_line_to_area(AsmArea* area, AsmLine* line);
static int elements_count(NodeList* list);
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

static void setup_class_table()
{
	asm_class_table = malloc(sizeof(ClassTable));

	asm_class_table->classes = malloc(sizeof(AsmClassInfo*) * 4);
	asm_class_table->class_capacity = 4;
	asm_class_table->class_count = 0;
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
	setup_class_table();
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
	
	char buff[64];
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

static void add_class_to_table(AsmClassInfo* class)
{
	if (asm_class_table->class_capacity <= asm_class_table->class_count + 1)
	{
		asm_class_table->class_capacity *= 2;

		asm_class_table->classes = realloc(asm_class_table->classes, sizeof(Constant*) * asm_class_table->class_capacity);

		if (asm_class_table->classes == NULL)
		{
			printf("[CodeGen] [Debug] Failed to realloc memory for constant table...\n");
			exit(1);
		}
	}

	asm_class_table->classes[asm_class_table->class_count] = class;
	asm_class_table->class_count++;
	asm_class_table->classes[asm_class_table->class_count] = NULL;
}

static int check_class(char* identifier, int has_constructor, int has_vtable)
{
	for (int i = 0; i < asm_class_table->class_count; i++)
	{
		AsmClassInfo* curr = asm_class_table->classes[i];
		
		if (strcmp(identifier, curr->class_name) == 0)
		{
			if (has_constructor)
			{
				return curr->has_constructor;
			}

			if (has_vtable)
			{
				return curr->has_v_table;
			}
		}
	}

	return 0;
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
	
	if (node->declare_node.declare.default_value != NULL)
	{
		Type* type = analyzer_return_type_of_expression(NULL, node->declare_node.declare.default_value, code_gen->scope, NULL, 0, 0);
	
		char* _temp = "mov";
	
		if (type->type == TYPE_FLOAT || type->type == TYPE_DOUBLE)
		{
			_temp = type->type == TYPE_DOUBLE ? "movsd" : "movss";
		}

		int force_reg = 1;

		if (type->type == TYPE_INT)
		{
			force_reg = 0;
		}
		
		AsmReturn* ret = generate_expression(code_gen, node->declare_node.declare.default_value, scope_depth, area, force_reg, 0, 0);
	
		AsmLine* line = create_line();
		
		char buffer[64];
		snprintf(buffer, 64, "	%s	[rbp%+d], %s", _temp, offset, ret->reg);
	
		line->line = strdup(buffer);
	
		add_line_to_area(area, line);
	}
	else
	{
		if (node->declare_node.declare.var_type->type == TYPE_ARRAY)
		{
			char buff[64];
			
			AsmLine* _line = create_line();
			
			snprintf(buff, 64, "	mov	[rbp%+d], 0", offset);

			_line->line = strdup(buff);

			add_line_to_area(area, _line);
		}
	}
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

static char* get_literal_value(Node* node, AsmArea* area, int force_reg, int prefer_second, int arg)
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

				if (arg)
				{
					temp = prefer_second ? "rbx" : "rax";
				}

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

				if (arg)
				{
					temp = prefer_second ? "rbx" : "rax";
				}

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

				if (arg)
				{
					temp = prefer_second ? "rbx" : "rax";
				}

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
			
			snprintf(buff, 64, "[rip + .LC%d]", constant->number);

			AsmLine* line = create_line();

			char _buff[64];
			snprintf(_buff, 64, "	movss	%s, [rip + .LC%d]", temp, constant->number);

			line->line = strdup(_buff);

			add_line_to_area(area, line);
			
			return strdup(temp);
		}

		case TYPE_DOUBLE:
		{
			Constant* constant = generate_constant(node);
			
			char* temp = prefer_second ? "xmm1" : "xmm0";
			
			snprintf(buff, 64, "[rip + .LC%d]", constant->number);

			AsmLine* line = create_line();
			
			char _buff[64];
			snprintf(_buff, 64, "	movsd	%s, [rip + .LC%d]", temp, constant->number);

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

static void generate_block_code(CodeGen* code_gen, Node* node, int scope_depth, AsmArea** area, int offset)
{
	Node* next = node->block_node.block.statements->head;

	while (next != NULL)
	{
		code_gen_node(code_gen, next, scope_depth, *area);
		next = next->next;

		if (ref_post_area != NULL)
		{
			add_function_area_to_text_section(*area);

			*area = ref_post_area;
			ref_post_area = NULL;
		}
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
	AsmLine* leave_unsetup = create_line();
	AsmLine* ret = create_line();
	
	// Usado pra deixar o codigo mais limpo.
	// - Equivale a:
	// 	mov rsp, rbp
	// 	pop rbp
	leave_unsetup->line = strdup("	leave");
	ret->line = strdup("	ret");

	add_line_to_area(area, leave_unsetup);
	add_line_to_area(area, ret);
}

static void generate_stack_space(int offset, AsmArea* area)
{
	if (offset == 0)
	{
		return;
	}
	
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

	char* identifier = (class_func_flag != NULL) ? class_func_flag : node->function_node.function.identifier;

	Symbol* symbol = (!constructor_flag) ? analyzer_find_symbol_from_scope(identifier, code_gen->scope, 0, 1, 0, 0) : code_gen->scope->owner_statement->symbol_class->constructor;
	
	add_line_to_area(function_area, label_line);

	generate_function_setup(function_area);

	generate_stack_space(symbol->symbol_function->total_offset * (-1), function_area);

	Node* block = node->function_node.function.block_node;

	SymbolTable* temp = code_gen->scope;
	
	code_gen->scope = symbol->symbol_function->scope;

	actual_offset = symbol->symbol_function->total_offset * (-1) + 8; // +8 == push
	generate_block_code(code_gen, block, scope_depth + 1, &function_area, actual_offset);

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

	Symbol* symbol = analyzer_find_symbol_from_scope(identifier, code_gen->scope, 1, 0, 0, 0);

	char buff[64];

	char* temp = prefer_secondary ? "rbx" : "rax";
	char* _temp = "mov";

	/**
	 * Lidar com classes.
	 */	
	Type* type = symbol->symbol_variable->type;

	if (type->type == TYPE_FLOAT || type->type == TYPE_DOUBLE)
	{
		temp = prefer_secondary ? "xmm1" : "xmm0";
		_temp = type->type == TYPE_DOUBLE ? "movsd" : "movss";
	}

	snprintf(buff, 64, "	%s	%s, [%s]", _temp, temp, ref->reg);

	AsmLine* reg_mov_line = create_line();
	reg_mov_line->line = strdup(buff);

	free(ref);

	add_line_to_area(area, reg_mov_line);

	AsmReturn* asm_return = create_asm_return(temp, ref->type);

	return asm_return;
}

static AsmReturn* generate_load_var_address_to_reg(CodeGen* code_gen, char* identifier, int scope_depth, AsmArea* area, int prefer_secondary)
{
	AsmReturn* ref = get_variable_reference(identifier, code_gen->scope);

	Symbol* symbol = analyzer_find_symbol_from_scope(identifier, code_gen->scope, 1, 0, 0, 0);

	char buff[64];

	char* temp = prefer_secondary ? "rbx" : "rax";
	
	snprintf(buff, 64, "	mov	%s, %s", temp, ref->reg);

	AsmLine* reg_mov_line = create_line();
	reg_mov_line->line = strdup(buff);

	free(ref);

	add_line_to_area(area, reg_mov_line);

	AsmReturn* asm_return = create_asm_return(temp, ref->type);

	return asm_return;
}

static AsmReturn* get_value(CodeGen* code_gen, Node* node, int scope_depth, AsmArea* area, int force_reg, int prefer_second, int arg)
{
	AsmReturn* value = NULL;
	
	if (node->type == NODE_LITERAL)
	{
		char* temp = get_literal_value(node, area, force_reg, prefer_second, arg);

		value = create_asm_return(temp, node->literal_node.literal.literal_type);
	}

	return value;
}

static void generate_local_variable_assign(CodeGen* code_gen, Node* node, int scope_depth, AsmArea* area)
{
	Node* ptr = node->variable_assign_node.variable_assign.left;

	int i = 0;
	
	while (ptr->type == NODE_DEREFERENCE)
	{
		ptr = ptr->dereference_node.dereference.ptr;
		i++;
	}

	flag_assign = 1;

	char* ref = generate_expression(code_gen, ptr, 0, area, 0, 0, 0)->reg;

	flag_assign = 0;
	
	while (i > 0)
	{
		AsmLine* line = create_line();

		char buff[64];
		snprintf(buff, 64, "	mov	rax, [%s]", ref);
		ref = "rax";
		
		line->line = strdup(buff);
		add_line_to_area(area, line);

		i--;
	}

	Type* type = analyzer_return_type_of_expression(NULL, node->variable_assign_node.variable_assign.assign_value, code_gen->scope, NULL, 0, 0);
	
	int force_reg = 1;

	if (type->type == TYPE_INT)
	{
		force_reg = 0;
	}

	AsmReturn* ret = generate_expression(code_gen, node->variable_assign_node.variable_assign.assign_value, scope_depth, area, force_reg, 0, 0);

	char* _temp = "mov";

	if (type->type == TYPE_FLOAT || type->type == TYPE_DOUBLE)
	{
		_temp = type->type == TYPE_DOUBLE ? "movsd" : "movss";
	}

	char buff[64];
	snprintf(buff, 64, "	%s	%s, %s", _temp, ref, ret->reg);

	AsmLine* line = create_line();
	line->line = strdup(buff);

	add_line_to_area(area, line);
}

static void generate_global_variable_assign(CodeGen* code_gen, Node* node, int scope_depth, AsmArea* area)
{
	Node* ptr = node->variable_assign_node.variable_assign.left;

	int i = 0;
	
	while (ptr->type == NODE_DEREFERENCE)
	{
		ptr = ptr->dereference_node.dereference.ptr;
		i++;
	}

	char _buff[64];

	char* ref = get_variable_reference(ptr->variable_node.variable.identifier, code_gen->scope)->reg;
	snprintf(_buff, 64, "rip + %s", ref);

	ref = strdup(_buff);
	
	while (i > 0)
	{
		AsmLine* line = create_line();

		char buff[64];
		snprintf(buff, 64, "	mov	rax, [%s]", ref);
		ref = "rax";
		
		line->line = strdup(buff);
		add_line_to_area(area, line);

		i--;
	}

	AsmReturn* ret = generate_expression(code_gen, node->variable_assign_node.variable_assign.assign_value, scope_depth, area, 1, 0, 0);

	Type* type = analyzer_return_type_of_expression(NULL, node->variable_assign_node.variable_assign.assign_value, code_gen->scope, NULL, 0, 0);

	char* _temp = "mov";

	if (type->type == TYPE_FLOAT || type->type == TYPE_DOUBLE)
	{
		_temp = type->type == TYPE_DOUBLE ? "movsd" : "movss";
	}

	char buff[64];
	snprintf(buff, 64, "	%s	[%s], %s", _temp, ref, ret->reg);

	AsmLine* line = create_line();
	line->line = strdup(buff);

	add_line_to_area(area, line);
}

static void generate_variable_assign(CodeGen* code_gen, Node* node, int scope_depth, AsmArea* area)
{
	Node* ptr = node->variable_assign_node.variable_assign.left;
	
	while (ptr->type == NODE_DEREFERENCE)
	{
		ptr = ptr->dereference_node.dereference.ptr;
	}

	if (ptr->type == NODE_IDENTIFIER)
	{
		char* identifier = ptr->variable_node.variable.identifier;
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
	else
	{
		generate_local_variable_assign(code_gen, node, scope_depth, area);
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

static AsmReturn* generate_and(CodeGen* code_gen, AsmReturn* lreg, AsmReturn* rreg,  Node* node, AsmArea* area, int prefer_second)
{
	AsmLine* line = create_line();

	char* temp = (lreg->reg[0] == 'a' && lreg->reg[1] == 'l') ? "eax" : "ebx";
	char* _temp = (rreg->reg[0] == 'a' && rreg->reg[1] == 'l') ? "eax" : "ebx";

	char buff[64];
	snprintf(buff, 64, "	or	%s, %s", temp, _temp);

	line->line = strdup(buff);

	AsmReturn* ret = create_asm_return(lreg->reg, create_type(TYPE_BOOL, NULL));

	add_line_to_area(area, line);
	
	return ret;
}

static AsmReturn* generate_or(CodeGen* code_gen, AsmReturn* lreg, AsmReturn* rreg,  Node* node, AsmArea* area, int prefer_second)
{
	AsmLine* line = create_line();

	char* temp = (lreg->reg[0] == 'a' && lreg->reg[1] == 'l') ? "eax" : "ebx";
	char* _temp = (rreg->reg[0] == 'a' && rreg->reg[1] == 'l') ? "eax" : "ebx";

	char buff[64];
	snprintf(buff, 64, "	or	%s, %s", temp, _temp);

	line->line = strdup(buff);

	AsmReturn* ret = create_asm_return(lreg->reg, create_type(TYPE_BOOL, NULL));

	add_line_to_area(area, line);
	
	return ret;
}

static AsmReturn* generate_equals(CodeGen* code_gen, AsmReturn* lreg, AsmReturn* rreg,  Node* node, AsmArea* area, int prefer_second)
{
	char* temp = prefer_second ? "bl" : "al";
	char* _temp = prefer_second ? "ebx" : "eax";
	
	AsmLine* cmp_line = create_line();
	AsmLine* set_line = create_line();
	AsmLine* mov_line = create_line();

	char buff[64];

	snprintf(buff, 64, "	cmp	%s, %s", lreg->reg, rreg->reg);
	cmp_line->line = strdup(buff);

	snprintf(buff, 64, "	sete	%s", temp);
	set_line->line = strdup(buff);

	snprintf(buff, 64, "	movzx	%s, %s", _temp, temp);
	mov_line->line = strdup(buff);

	add_line_to_area(area, cmp_line);
	add_line_to_area(area, set_line);
	add_line_to_area(area, mov_line);

	AsmReturn* ret = create_asm_return(temp, create_type(TYPE_BOOL, NULL));
	
	return ret;
}

static AsmReturn* generate_not_equals(CodeGen* code_gen, AsmReturn* lreg, AsmReturn* rreg,  Node* node, AsmArea* area, int prefer_second)
{
	char* temp = prefer_second ? "bl" : "al";
	char* _temp = prefer_second ? "ebx" : "eax";
	
	AsmLine* cmp_line = create_line();
	AsmLine* set_line = create_line();
	AsmLine* mov_line = create_line();

	char buff[64];

	snprintf(buff, 64, "	cmp	%s, %s", lreg->reg, rreg->reg);
	cmp_line->line = strdup(buff);

	snprintf(buff, 64, "	setne	%s", temp);
	set_line->line = strdup(buff);

	snprintf(buff, 64, "	movzx	%s, %s", _temp, temp);
	mov_line->line = strdup(buff);

	add_line_to_area(area, cmp_line);
	add_line_to_area(area, set_line);
	add_line_to_area(area, mov_line);

	AsmReturn* ret = create_asm_return(temp, create_type(TYPE_BOOL, NULL));
	
	return ret;
}

static AsmReturn* generate_less(CodeGen* code_gen, AsmReturn* lreg, AsmReturn* rreg,  Node* node, AsmArea* area, int prefer_second)
{
	char* temp = prefer_second ? "bl" : "al";
	char* _temp = prefer_second ? "ebx" : "eax";
	
	AsmLine* cmp_line = create_line();
	AsmLine* set_line = create_line();
	AsmLine* mov_line = create_line();

	char buff[64];

	snprintf(buff, 64, "	cmp	%s, %s", lreg->reg, rreg->reg);
	cmp_line->line = strdup(buff);

	snprintf(buff, 64, "	setl	%s", temp);
	set_line->line = strdup(buff);

	snprintf(buff, 64, "	movzx	%s, %s", _temp, temp);
	mov_line->line = strdup(buff);

	add_line_to_area(area, cmp_line);
	add_line_to_area(area, set_line);
	add_line_to_area(area, mov_line);

	AsmReturn* ret = create_asm_return(temp, create_type(TYPE_BOOL, NULL));
	
	return ret;
}

static AsmReturn* generate_greater(CodeGen* code_gen, AsmReturn* lreg, AsmReturn* rreg,  Node* node, AsmArea* area, int prefer_second)
{
	char* temp = prefer_second ? "bl" : "al";
	char* _temp = prefer_second ? "ebx" : "eax";
	
	AsmLine* cmp_line = create_line();
	AsmLine* set_line = create_line();
	AsmLine* mov_line = create_line();

	char buff[64];

	snprintf(buff, 64, "	cmp	%s, %s", lreg->reg, rreg->reg);
	cmp_line->line = strdup(buff);

	snprintf(buff, 64, "	setg	%s", temp);
	set_line->line = strdup(buff);

	snprintf(buff, 64, "	movzx	%s, %s", _temp, temp);
	mov_line->line = strdup(buff);
	
	add_line_to_area(area, cmp_line);
	add_line_to_area(area, set_line);
	add_line_to_area(area, mov_line);

	AsmReturn* ret = create_asm_return(temp, create_type(TYPE_BOOL, NULL));
	
	return ret;
}

static AsmReturn* generate_operation(CodeGen* code_gen, Node* node, int scope_depth, AsmArea* area, int prefer_second, int arg)
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
		lreg = generate_expression(code_gen, node->operation_node.operation.left, scope_depth, area, 1, 0, arg);
	}

	AsmReturn* rreg = generate_expression(code_gen, node->operation_node.operation.right, scope_depth, area, 0, 1, arg);

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

		case TOKEN_OPERATOR_OR:
		{
			AsmReturn* r = generate_or(code_gen, lreg, rreg, node, area, prefer_second);

			free(lreg);
			free(rreg);

			return r;
		}

		case TOKEN_OPERATOR_AND:
		{
			AsmReturn* r = generate_and(code_gen, lreg, rreg, node, area, prefer_second);

			free(lreg);
			free(rreg);

			return r;
		}

		case TOKEN_OPERATOR_GREATER:
		{
			AsmReturn* r = generate_greater(code_gen, lreg, rreg, node, area, prefer_second);

			free(lreg);
			free(rreg);

			return r;
		}

		case TOKEN_OPERATOR_LESS:
		{
			AsmReturn* r = generate_less(code_gen, lreg, rreg, node, area, prefer_second);

			free(lreg);
			free(rreg);

			return r;
		}

		case TOKEN_OPERATOR_EQUALS:
		{
			AsmReturn* r = generate_equals(code_gen, lreg, rreg, node, area, prefer_second);

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

static AsmReturn* generate_cast(CodeGen* code_gen, Node* node, int scope_depth, AsmArea* area, int force_reg, int prefer_second, int arg)
{
	/**
	 * TODO: Adicionar o modulo na struct do CodeGen.
	 */
	AsmReturn* expr = generate_expression(code_gen, node->cast_statement_node.cast_node.expression, scope_depth, area, force_reg, 0, arg);
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

static AsmReturn* generate_dereference(CodeGen* code_gen, Node* node, AsmArea* area, int prefer_secondary)
{
	Node* ptr = node;
	
	int i = 0;

	while (ptr->type == NODE_DEREFERENCE)
	{
		ptr = ptr->dereference_node.dereference.ptr;
		i++;
	}
	
	AsmReturn* ret = generate_expression(code_gen, ptr, 0, area, 1, 0, 0);

	char* reg = prefer_secondary ? "rbx" : "rax";

	char* curr = ret->reg;

	if (flag_assign)
	{
		i--;
	}
	
	while (i > 0)
	{
		AsmLine* line = create_line();

		char buff[64];
		snprintf(buff, 64, "	mov	%s, [%s]", reg, curr);
		curr = reg;

		line->line = strdup(buff);
		add_line_to_area(area, line);

		i--;
	}

	Type* type = analyzer_return_type_of_expression(NULL, ptr, code_gen->scope, NULL, 0, NULL);
	
	if (flag_assign)
	{
		char buff[32];
		snprintf(buff, 32, member_access_flag ? "%s" : "[%s]", curr);
		
		AsmReturn* final = create_asm_return(buff, type);

		return final;
	}

	AsmReturn* final = create_asm_return(curr, type);

	return final;
}

static AsmReturn* generate_adress_of(CodeGen* code_gen, Node* node, AsmArea* area, int prefer_secondary)
{
	Node* expr = node->adress_of_node.adress_of.expression;

	AsmLine* line = create_line();
		
	char buff[64];

	char* _temp = prefer_secondary ? "rbx" : "rbx";
		
  	Type* type = analyzer_return_type_of_expression(NULL, expr, code_gen->scope, NULL, 0, NULL);

	char* temp = get_variable_reference(expr->variable_node.variable.identifier, code_gen->scope)->reg;

	snprintf(buff, 64, "	lea	%s, [%s]", _temp, temp);
	line->line = strdup(buff);

	add_line_to_area(area, line);

	AsmReturn* _ret = create_asm_return(strdup(_temp), type);

	return _ret;
}

static int elements_count(NodeList* list)
{
	int count = 0;

	Node* curr = list->head;

	while (curr != NULL)
	{
		curr = curr->next;
		count++;
	}

	return count;
}

static void generate_array_value(CodeGen* code_gen, Node* expr, AsmArea* area, char* adress_reg, int offset, int type_size)
{
	AsmLine* line = create_line();

 	int force_reg = 0;
	int force_sec = 0;
	
	if (expr->type == NODE_IDENTIFIER)
	{
		force_reg = 1;
		force_sec = 1;
	}

	AsmReturn* ret = generate_expression(code_gen, expr, 0, area, force_reg, force_sec, 0);
	char buff[128];

	int mem_offset = 8 + (offset * type_size);

	snprintf(buff, 64, "	mov	[%s + %d], %s", adress_reg, mem_offset, ret->reg);

	line->line = strdup(buff);

	add_line_to_area(area, line);
}

static void setup_array_count(AsmArea* area, char* adress_reg, int count)
{
	AsmLine* line = create_line();

	char buff[32];
	snprintf(buff, 32, "	mov	[%s], %d", adress_reg, count);

	line->line = strdup(buff);

	add_line_to_area(area, line);
}

static void generate_array_values(CodeGen* code_gen, Node* node, AsmArea* area, char* adress_reg, int elements_size, int type_size)
{
	NodeList* values = node->array_literal_node.array_literal.values;
	Node* curr = values->head;
	
	setup_array_count(area, adress_reg, elements_size);
	int i = 0;

	while (i < elements_size)
	{
		if (i == 0)
		{
			generate_array_value(code_gen, curr, area, adress_reg, i, type_size);

			curr = curr->next;
			i++;

			continue;
		}
		
		generate_array_value(code_gen, curr, area, adress_reg, i, type_size);

		curr = curr->next;
		i++;
	}
}

static int generate_malloc_align_stack_to_call(AsmArea* area, int count, int type_size)
{
	int res = 0;
	
	if ((actual_offset + 8) % 16 == 0) // adiciona 8 pra instruçao call (sub rsp, 8 implicitamente)
	{
		AsmLine* line = create_line();
		line->line = strdup("	sub	rsp, 8");
		
		add_line_to_area(area, line);
		res = 1;
	}

	return res;
}

static int arr_generate_malloc_align_stack_to_call(AsmArea* area, int count, int type_size)
{
	int res = 0;
	
	if ((actual_offset + 8) % 16 == 0) // adiciona 8 pra instruçao call (sub rsp, 8 implicitamente)
	{
		AsmLine* line = create_line();
		line->line = strdup("	sub	rsp, 8");
		
		add_line_to_area(area, line);
		res = 1;
	}

	char buff[64];
	snprintf(buff, 64, "	mov	rcx, %d", 8 + (count * type_size));

	AsmLine* line = create_line();
	line->line = strdup(buff);

 	add_line_to_area(area, line);

	return res;
}

static AsmReturn* generate_array_literal(CodeGen* code_gen, Node* node, int scope_depth, AsmArea* area, int force_reg, int prefer_secondary, int arg)
{
	Type* type = node->array_literal_node.array_literal.array_type;
	
	int count = elements_count(node->array_literal_node.array_literal.values);
	int type_size = analyzer_get_type_size(type->base, code_gen->scope);

	int align = arr_generate_malloc_align_stack_to_call(area, count, type_size);

	AsmLine* line = create_line();
	line->line = strdup("	call	malloc");

 	add_line_to_area(area, line);

	if (align)
	{
		AsmLine* line = create_line();
		line->line = strdup("	add	rsp, 8");
		
		add_line_to_area(area, line);
	}

	generate_array_values(code_gen, node, area, strdup("rax"), count, type_size);

	AsmReturn* ret = create_asm_return("rax", node->array_literal_node.array_literal.array_type);

	return ret;
}

/**
 * TODO: Implementar melhor prototype pra isso no analyzer e parser (nodes)
 */
static AsmReturn* generate_array_push(CodeGen* code_gen, Node* node, int scope_depth, AsmArea* area)
{

}

/**
 * TODO: Implementar melhor prototype pra isso no analyzer e parser (nodes)
 */
static AsmReturn* generate_array_pop(CodeGen* code_gen, Node* node, int scope_depth, AsmArea* area)
{

}

/**
 * TODO: Implementar melhor prototype pra isso no analyzer e parser (nodes)
 */
static AsmReturn* generate_array_length(CodeGen* code_gen, Node* node, int scope_depth, AsmArea* area, int prefer_second)
{
	char* temp = prefer_second ? "ebx" : "eax";

	Node* arr = node->array_length_node.array_length.array;
	AsmReturn* ret = generate_expression(code_gen, arr, 0, area, 1, 0, 0);

	AsmLine* line = create_line();

	char buff[50];
	snprintf(buff, 50, "	mov	%s, [%s]", temp, ret->reg);

	line->line = strdup(buff);
	add_line_to_area(area, line);
	
	AsmReturn* res = create_asm_return(temp, create_type(TYPE_INT, NULL));

	return res;
}

/**
 * TODO: Caso o tipo for float ou double, o mov do elemento pro registrador vai dar merda, arrumar
 */
static AsmReturn* generate_array_access(CodeGen* code_gen, Node* node, AsmArea* area)
{
	Node* arr = node->acess_array_node.acess_array.array;
	Node* index_expr = node->acess_array_node.acess_array.index_expr;
	
	AsmReturn* ret = generate_expression(code_gen, arr, 0, area, 1, 0, 0);
	AsmReturn* expr_ret = generate_expression(code_gen, index_expr, 0, area, 1, 1, 0);

	AsmLine* line = create_line();
	char buff[64];

	char* index = expr_ret->reg;

	AsmLine* _line = create_line();
	
	Type* element_type = ret->type->base;
	int element_size = analyzer_get_type_size(ret->type->base, code_gen->scope);

	snprintf(buff, 64, "	imul	%s, %d", index, element_size);
	_line->line = strdup(buff);

	add_line_to_area(area, _line);

	AsmLine* __line = create_line();

	snprintf(buff, 64, "	add	%s, 8", index);
	__line->line = strdup(buff);

	add_line_to_area(area, __line);
	
	if (flag_assign)
	{
		snprintf(buff, 64, "[%s+%s]", ret->reg, index);
		
		AsmReturn* res = create_asm_return(buff, element_type);
	
		return res;
	}
	
	snprintf(buff, 64, "	mov	rdi, [%s+%s]", ret->reg, index);
	line->line = strdup(buff);

	add_line_to_area(area, line);

	AsmReturn* res = create_asm_return("rdi", element_type);

	return res;
}

static AsmReturn* generate_member_access(CodeGen* code_gen, Node* node, AsmArea* area, int prefer_secondary)
{
	int pointer_access = node->member_access_node.member_access.ptr_acess;
	char* member_name = node->member_access_node.member_access.member_name;
	
	flag_assign = 1;
	member_access_flag = 1;
	
	AsmReturn* object = generate_expression(code_gen, node->member_access_node.member_access.object, 0, area, 1, 1, 0);
	
	member_access_flag = 0;
	flag_assign = 0;

	Type* type = object->type;
	char* reg = object->reg;

	AsmLine* line = create_line();

	int offset = 16;

	if (type->type == TYPE_PTR)
	{
		type = type->base;
	}

	Symbol* symbol_class = analyzer_find_symbol_from_scope(type->class_name, code_gen->scope, 0, 0, 1, 0);
	
	SymbolTable* scope = symbol_class->symbol_class->class_scope;

	Symbol* symbol = analyzer_find_symbol_from_scope(member_name, scope, 1, 0, 0, 0);

	offset += symbol->symbol_variable->offset;

	char buff[50];
	snprintf(buff, 50, "	mov	rdi, [%s+%d]", reg, offset);

	line->line = strdup(buff);
	add_line_to_area(area, line);

	AsmReturn* ret = create_asm_return("rdi", symbol->symbol_variable->type);

	return ret;
}

static AsmReturn* generate_create_instance(CodeGen* code_gen, Node* node, AsmArea* area, int prefer_secondary)
{
	char* class_name = node->create_instance_node.create_instance.class_name;
	Type* type = create_type(TYPE_CLASS, class_name);

	/*==-------------------------------------------------==*/

	AsmLine* size_line = create_line();

	int size = analyzer_get_type_size(type, code_gen->scope);

	char buff[64];

	// Porque do + 16: o objeto da classe é formado por: ponteiro pra vtable: (8 bytes), 
	// id da classe (4 bytes do int + 4 bytes de alinhamento) 
	// e o resto que são as fields.
	snprintf(buff, 64, "	mov	rdi, %d", size + 16);

	size_line->line = strdup(buff);

	add_line_to_area(area, size_line);

	/*==-------------------------------------------------==*/
	
	int align = generate_malloc_align_stack_to_call(area, 0, size + 8);
	
	AsmLine* line = create_line();

	snprintf(buff, 64, "	call	malloc");

	line->line = strdup(buff);

	add_line_to_area(area, line);

	if (align)
	{
		AsmLine* line = create_line();
		line->line = strdup("	add	rsp, 8");
		
		add_line_to_area(area, line);
	}

	/*==-------------------------------------------------==*/

	AsmLine* v_mov_line = create_line();

	if (check_class(class_name, 0, 1))
	{
		snprintf(buff, 64, "	mov	[rax], .%s_vtable", class_name);
	}
	else
	{
		snprintf(buff, 64, "	mov	[rax], 0");
	}

	v_mov_line->line = strdup(buff);

	add_line_to_area(area, v_mov_line);

	/*==-------------------------------------------------==*/

	AsmLine* mov_line = create_line();

	mov_line->line = strdup("	mov	rdi, rax");

	NodeList* args = node->create_instance_node.create_instance.constructor_args;

	if (args != NULL)
	{
		generate_setup_arguments(code_gen, args->head, area, 1);
	}
	
	add_line_to_area(area, mov_line);

	/*==-------------------------------------------------==*/

	if (check_class(class_name, 1, 0))
	{
		AsmLine* constructor_line = create_line();
	
		snprintf(buff, 64, "	call	.%s_::ctr", class_name);
		constructor_line->line = strdup(buff);
	
		add_line_to_area(area, constructor_line);
	}

	/*==-------------------------------------------------==*/

	AsmReturn* ret = create_asm_return("rax", type);

	return ret;
}

static AsmReturn* generate_expression(CodeGen* code_gen, Node* node, int scope_depth, AsmArea* area, int force_reg, int prefer_secondary, int arg)
{
	switch (node->type)
	{
		case NODE_IDENTIFIER:
		{
			if (force_reg)
			{
				AsmReturn* ret = generate_load_var_to_reg(code_gen, node->variable_node.variable.identifier, scope_depth, area, prefer_secondary);
				
				AsmReturn* asm_return = create_asm_return(ret->reg, ret->type);
				
				return asm_return;
			}
			else
			{
				AsmReturn* ret = get_variable_reference(node->variable_node.variable.identifier, code_gen->scope);
				
				char* ref = ret->reg;
			
				Symbol* symbol = analyzer_find_symbol_from_scope(node->variable_node.variable.identifier, code_gen->scope, 1, 0, 0, 0);
				
				char* ref_str = NULL;

				if (symbol->symbol_variable->is_global)
				{
					char buff[64];
					snprintf(buff, sizeof(buff), "rip + %s", node->variable_node.variable.identifier);
					ref_str = strdup(buff);  // Aloca string dinamicamente
				}
				else
				{
					if (flag_assign)
						ref_str = strdup(ret->reg);
					else
					{
						char buff[64];
						snprintf(buff, sizeof(buff), "[%s]", ret->reg);
						ref_str = strdup(buff);
					}
				}

				AsmReturn* asm_return = create_asm_return(ref_str, ret->type);
				return asm_return;
			}
		}

		case NODE_FUNCTION_CALL:
		{
			AsmReturn* ret = generate_function_call(code_gen, node, area, prefer_secondary, arg);

			AsmReturn* asm_return = create_asm_return(ret->reg, ret->type);
			
			return asm_return;
		}
		
		case NODE_LITERAL:
		{
			return get_value(code_gen, node, scope_depth, area, force_reg, prefer_secondary, arg);
		}

		case NODE_ARRAY_LITERAL:
		{
			return generate_array_literal(code_gen, node, scope_depth, area, force_reg, prefer_secondary, arg);
		}

		case NODE_OPERATION:
		{
			return generate_operation(code_gen, node, scope_depth, area, prefer_secondary, arg);
		}

		case NODE_CAST:
		{
			return generate_cast(code_gen, node, scope_depth, area, force_reg, prefer_secondary, arg);
		}

		case NODE_DEREFERENCE:
		{
			return generate_dereference(code_gen, node, area, prefer_secondary);
		}

		case NODE_ADRESS_OF:
		{
			return generate_adress_of(code_gen, node, area, prefer_secondary);
		}

		case NODE_ARRAY_ACCESS:
		{
			return generate_array_access(code_gen, node, area);
		}

		case NODE_MEMBER_ACCESS:
		{
			return generate_member_access(code_gen, node, area, prefer_secondary);
		}

		case NODE_CREATE_INSTANCE:
		{
			return generate_create_instance(code_gen, node, area, prefer_secondary);
		}

		default:
		{
			printf("[CodeGen] [Debug] Expression node not implemented: %d...\n", node->type);
			exit(1);
		}
	}
}

static void handle_float_double_argument(CodeGen* code_gen, Node* node, AsmArea* area, int doub)
{
	AsmLine* line = create_line();
	AsmLine* _line = create_line();

	char* temp = doub ? "movsd" : "movss";
	char buff[64];

	AsmReturn* ret = generate_expression(code_gen, node->argument_node.argument.value, 0, area, 1, 0, 1);

	snprintf(buff, 64, "	%s	[rsp], %s", temp, ret->reg);

	line->line = strdup("	sub	rsp, 8");
	_line->line = strdup(buff);

	add_line_to_area(area, line);
	add_line_to_area(area, _line);
}

static void generate_stack_args(CodeGen* code_gen, Node* head, Node* arg, AsmArea* area, int floating, int doub)
{
	if (floating)
	{
		handle_float_double_argument(code_gen, arg, area, doub);

		return;
	}
	
	AsmLine* line = create_line();
	
	char buff[32];
	
	AsmReturn* ret = generate_expression(code_gen, arg->argument_node.argument.value, 0, area, 1, 0, 1);
	
	snprintf(buff, 32, "	push	%s", ret->reg);
	line->line = strdup(buff);
	
	add_line_to_area(area, line);
}

static void generate_register_args(CodeGen* code_gen, Node* head, Node* arg, AsmArea* area, int count, int floating)
{
	if (floating)
	{
		AsmReturn* ret = generate_expression(code_gen, arg->argument_node.argument.value, 0, area, 0, 0, 1);
			
		AsmLine* line = create_line();
		
		char buff[32];	
		snprintf(buff, 32, "	mov	xmm%d, %s", count, ret->reg);
		
		line->line = strdup(buff);
		
		add_line_to_area(area, line);

		return;
	}
	
	AsmReturn* ret = generate_expression(code_gen, arg->argument_node.argument.value, 0, area, 0, 0, 1);
			
	AsmLine* line = create_line();
	
	char buff[32];	
	snprintf(buff, 32, "	mov	%s, %s", arg_registers[count], ret->reg);
	
	line->line = strdup(buff);
	
	add_line_to_area(area, line);
}

static int generate_setup_arguments(CodeGen* code_gen, Node* head, AsmArea* area, int jump_size)
{
	Node* next = head;

	int final_soffset = 0;
	int count = 0;

	while (jump_size > 0)
	{
		count++;
		jump_size--;
	}
	
	while (next != NULL)
	{
		Type* type = analyzer_return_type_of_expression(NULL, next->argument_node.argument.value, code_gen->scope, NULL, 0, 0);
		int floating = (type->type == TYPE_FLOAT || type->type == TYPE_DOUBLE);

		if (count <= 6)
		{
			generate_register_args(code_gen, head, next, area, count, floating);
		}
		else
		{
			generate_stack_args(code_gen, head, next, area, floating, (type->type == TYPE_DOUBLE));

			final_soffset += 8;
		}
		
		next = next->next;
		count++;
	}

	return final_soffset;
}

static char* get_callee_identifier(Node* callee)
{
	if (callee->type == NODE_MEMBER_ACCESS)
	{
		return callee->member_access_node.member_access.member_name;
	}

	if (callee->type == NODE_IDENTIFIER)
	{
		return callee->variable_node.variable.identifier;
	}

	return NULL;
}

static AsmReturn* find_function_reg(Type* type, AsmArea* area, int prefer_second, int arg)
{
	switch (type->type)
	{
		case TYPE_BOOL:
		{
			char* temp = prefer_second ? "bl" : "al";

			if (arg)
			{	
				temp = prefer_second ? "rbx" : "rax";
			}

			if (prefer_second && !arg)
			{
				AsmLine* line = create_line();
				
				char buff[50];
				snprintf(buff, 50, "	mov	%s, al", temp);
				
				line->line = strdup(buff);
				
				add_line_to_area(area, line);
			}

			AsmReturn* ret = create_asm_return(temp, type);
			
			return ret;
		}
		
		case TYPE_CHAR:
		{
			char* temp = prefer_second ? "bl" : "al";

			if (arg)
			{
				temp = prefer_second ? "rbx" : "rax";
			}

			if (prefer_second && !arg)
			{	
				AsmLine* line = create_line();
				
				char buff[50];
				snprintf(buff, 50, "	mov	%s, al", temp);
				
				line->line = strdup(buff);
				
				add_line_to_area(area, line);
			}

			AsmReturn* ret = create_asm_return(temp, type);
			
			return ret;
		}
		
		case TYPE_INT:
		{
			char* temp = prefer_second ? "ebx" : "eax";

			if (arg)
			{
				temp = prefer_second ? "rbx" : "rax";
			}

			if (prefer_second && !arg)
			{
				AsmLine* line = create_line();
			
				char buff[50];
				snprintf(buff, 50, "	mov	%s, eax", temp);
			
				line->line = strdup(buff);
			
				add_line_to_area(area, line);
			}

			AsmReturn* ret = create_asm_return(temp, type);
			
			return ret;
		}

		case TYPE_FLOAT:
		case TYPE_DOUBLE:
		{
			char* temp = prefer_second ? "xmm1" : "xmm0";

			if (prefer_second)
			{
				AsmLine* line = create_line();
			
				char buff[50];
				snprintf(buff, 50, "	mov	%s, xmm0", temp);
				
				line->line = strdup(buff);
				
				add_line_to_area(area, line);
			}

			AsmReturn* ret = create_asm_return(temp, type);

			return ret;
		}

		default:
		{
			exit(1);
		}
	}
}

/**
 * TODO: Remover 'actual_offset', ele é inutil agora.
 */
static AsmReturn* generate_function_call(CodeGen* code_gen, Node* node, AsmArea* area, int prefer_second, int arg)
{
	int has_params = 0;
	int s_offset = 0;

	if (node->function_call_node.function_call.arguments != NULL)
	{
		has_params = 1;
		
		Node* params_head = node->function_call_node.function_call.arguments->head;
	
		s_offset = generate_setup_arguments(code_gen, params_head, area, 0);
		int offset = s_offset + actual_offset;
	
		if ((offset + 8) % 16 == 0)
		{	
			AsmLine* line = create_line();
			line->line = strdup("	sub	rsp, 8");
			s_offset += 8;
	
			add_line_to_area(area, line);
		}
	}

	char buff[80];

	AsmLine* line = create_line();
	
	char* identifier = get_callee_identifier(node->function_call_node.function_call.callee);
	snprintf(buff, 32, "	call	%s", identifier);

	line->line = strdup(buff);

	add_line_to_area(area, line);

	char _buff[80];

	AsmLine* _line = create_line();
	
	if (has_params)
	{
		snprintf(_buff, 32, "	add	rsp, %d", s_offset);

		_line->line = strdup(_buff);

		add_line_to_area(area, _line);
	}
	
	Symbol* symbol = analyzer_find_symbol_from_scope(identifier, code_gen->scope, 0, 1, 0, 0);
	Type* type = symbol->symbol_function->return_type;

	if (type->type == TYPE_VOID)
	{
		return NULL;
	}

	return find_function_reg(type, area, prefer_second, arg);
}

static char* get_return_type_reg(Type* type)
{
	switch (type->type)
	{
		case TYPE_BOOL:
		{
			return strdup("al");
		}
		
		case TYPE_CHAR:
		{
			return strdup("al");
		}
		
		case TYPE_INT:
		{
			return strdup("eax");
		}

		case TYPE_FLOAT:
		case TYPE_DOUBLE:
		{
			return strdup("xmm0");
		}

		case TYPE_ARRAY:
		case TYPE_PTR:
		{
			return strdup("rax");
		}

		default:
		{
			exit(1);
		}
	}
}

static int already_in_reg(const char* reg1, const char* reg2)
{
	for (int i = 0; all_families[i] != NULL; i++) 
	{
		const char** family = all_families[i];
		
		int found1 = 0;
		int found2 = 0;

		for (int j = 0; family[j] != NULL; j++) 
		{
			if (strcmp(reg1, family[j]) == 0)
			{
				found1 = 1;
			}

			if (strcmp(reg2, family[j]) == 0)
			{
				found2 = 1;
			}
		}

		if (found1 && found2)
		{
			return 1;
		}
	}

	return 0;
}

static void generate_return(CodeGen* code_gen, Node* node, int scope_depth, AsmArea* area)
{
	Node* expr = node->return_statement_node.return_statement.return_value;
	
	if (expr == NULL)
	{
		return;
	}
	
	AsmReturn* ret = generate_expression(code_gen, expr, scope_depth, area, 0, 0, 0);
	
	Type* type = analyzer_return_type_of_expression(NULL, expr, code_gen->scope, NULL, 0, 0);

	char* temp = "mov";

	if (type->type == TYPE_FLOAT || type->type == TYPE_DOUBLE)
	{
		temp = type->type == TYPE_DOUBLE ? "movsd" : "movss";
	}

	char* reg = get_return_type_reg(type);

	if (already_in_reg(reg, ret->reg))
	{
		return;
	}

	char buff[64];
	snprintf(buff, 64, "	%s	%s, %s", temp, reg, ret->reg);

	AsmLine* line = create_line();
	line->line = strdup(buff);

	add_line_to_area(area, line);
}

static void add_method_to_v_table(CodeGen* code_gen, Node* method, AsmArea* v_table, char* class_identifier)
{
	AsmLine* line = create_line();

	char _buff[64];
	snprintf(_buff, 64, ".%s_%s", class_identifier, method->function_node.function.identifier);
	
	char buff[64];
	snprintf(buff, 64, "	dq %s", _buff);

	line->line = strdup(buff);
	add_line_to_area(v_table, line);
}

static void add_v_table_to_data(AsmArea* v_table)
{
	for (int i = 0; i < v_table->lines_count; i++)
	{
		AsmLine* curr = v_table->lines[i];
		add_line_to_area(data_section, curr);
	}
}

static AsmArea* generate_class_v_table(CodeGen* code_gen, char* class_name)
{
	AsmArea* area = create_area();

	char buff[64];
	snprintf(buff, 64, ".%s_vtable:", class_name);
	AsmLine* label = generate_label(buff);

	label->line = strdup(buff);
	add_line_to_area(area, label);

	return area;
}

static void generate_class_method(CodeGen* code_gen, Node* method, char* class_identifier, int constructor)
{
	char* identifier = strdup(method->function_node.function.identifier);
	
	char buff[64];
	snprintf(buff, 64, ".%s_%s", class_identifier, constructor ? "::ctr" : identifier);
	
	class_func_flag = method->function_node.function.identifier;

	method->function_node.function.identifier = strdup(buff);
	
	if (constructor)
	{
		constructor_flag = 1;
	}
	
	generate_function_declaration(code_gen, method, 0, text_section);

	if (constructor)
	{
		constructor_flag = 0;
	}

	class_func_flag = NULL;

	free(method->function_node.function.identifier);
	method->function_node.function.identifier = identifier;
}

static void generate_class_methods(CodeGen* code_gen, Node** methods, int method_count, AsmArea* v_table, char* class_identifier)
{
	for (int i = 0; i < method_count; i++)
	{
		Node* curr = methods[i];

		if (curr == NULL)
		{
			continue; // constructor
		}
		
		if (curr->function_node.function.is_override || curr->function_node.function.is_virtual)
		{
			add_method_to_v_table(code_gen, curr, v_table, class_identifier);
		}

		generate_class_method(code_gen, curr, class_identifier, 0);
	}
}

static void generate_class_field(CodeGen* code_gen, Node* field, char* class_identifier)
{
	char* identifier = field->declare_node.declare.identifier;
	int bss = field->declare_node.declare.default_value == NULL;
	
	AsmArea* data = bss ? bss_section : data_section;
	
	char buff[64];
	snprintf(buff, 64, ".%s_%s", class_identifier, identifier);
	AsmLine* label = generate_label(buff);

	label->line = strdup(buff);
	add_line_to_area(data, label);

	AsmLine* line = create_line();
	
	Type* type = field->declare_node.declare.var_type;

	char* size_def = bss ? strdup("resb") : get_type_size(type->type);
	
	if (bss)
	{
		int size = analyzer_get_type_size(type, code_gen->scope);
		snprintf(buff, 64, "	%s %d", size_def, size);
	}
	else
	{
		char* ret = get_global_literal_value(field->declare_node.declare.default_value);
		snprintf(buff, 64, "	%s %s", size_def, ret);

		free(ret);
	}

	line->line = strdup(buff);

	add_line_to_area(data, line);
}

/**
 * Só fields 'static' vão estar aqui, o resto é associado na instancia da classe
 */
static void generate_class_fields(CodeGen* code_gen, Node** fields, int field_count, char* class_identifier)
{
	for (int i = 0; i < field_count; i++)
	{
		Node* curr = fields[i];
		
		if (!curr->declare_node.declare.is_static)
		{
			continue;
		}

		generate_class_field(code_gen, curr, class_identifier);
	}
}

static void generate_class(CodeGen* code_gen, Node* node, AsmArea* area)
{
	char* identifier = node->class_node.class_node.identifer;

	SymbolTable* temp = code_gen->scope;
	Node* constructor = node->class_node.class_node.constructor;
	
	Symbol* symbol = analyzer_find_symbol_from_scope(identifier, code_gen->scope, 0, 0, 1, 0);
	code_gen->scope = symbol->symbol_class->class_scope;

	AsmArea* v_table = generate_class_v_table(code_gen, identifier);

	Node** fields = node->class_node.class_node.var_declare_list;
	int field_count = node->class_node.class_node.var_count;

	generate_class_fields(code_gen, fields, field_count, identifier);

	Node** methods = node->class_node.class_node.func_declare_list;
	int method_count = node->class_node.class_node.func_count;

	AsmClassInfo* info = malloc(sizeof(AsmClassInfo));
	info->class_name = strdup(identifier);
	info->has_constructor = 0;
	info->has_v_table = 0;
	
	if (constructor != NULL)
	{
		generate_class_method(code_gen, constructor, identifier, 1);
	}
	
	generate_class_methods(code_gen, methods, method_count, v_table, identifier);

	code_gen->scope = temp;
	
	if (v_table->lines_count == 1)
	{
		add_class_to_table(info);

		return;
	}

	info->has_v_table = 1;
	add_v_table_to_data(v_table);

	add_class_to_table(info);
}

static void generate_post(CodeGen* code_gen, Node* then, AsmArea* area, int id)
{
	AsmArea* post_area = create_area();
	
	char buff[50];
	
	/* ------------------------------------------------------------- */
	
	char _buff[64];
	snprintf(_buff, 64, ".if_post_%d", id);
	
	AsmLine* label = generate_label(_buff);
	add_line_to_area(post_area, label);

	/* ------------------------------------------------------------- */

	ref_post_area = post_area;
}

static AsmArea* generate_then(CodeGen* code_gen, Node* expr, Node* then, AsmArea* area, int id)
{
	AsmArea* then_area = create_area();
	AsmReturn* ret = generate_expression(code_gen, expr, 0, area, 0, 0, 0);

	char buff[50];
	
	/* ------------------------------------------------------------- */
	
	char _buff[64];
	snprintf(_buff, 64, ".if_then_%d", id);
	
	AsmLine* label = generate_label(_buff);
	add_line_to_area(then_area, label);

	/* ------------------------------------------------------------- */
	
	AsmLine* test_line = create_line();
	
	snprintf(buff, 50, "	test	%s, %s", ret->reg, ret->reg);
	test_line->line = strdup(buff);

	add_line_to_area(area, test_line);

	/* ------------------------------------------------------------- */
	
	AsmLine* jmp_line = create_line();

	snprintf(buff, 50, "	jnz	%s", _buff);
	jmp_line->line = strdup(buff);

	add_line_to_area(area, jmp_line);

	/* ------------------------------------------------------------- */

	Node* next = then->block_node.block.statements->head;

	while (next != NULL)
	{
		code_gen_node(code_gen, next, 0, then_area);
		next = next->next;
	}

	AsmLine* ret_line = create_line();

	snprintf(buff, 50, "	jmp	.if_post_%d", ifs_count);
	ret_line->line = strdup(buff);

	add_line_to_area(then_area, ret_line);

	return then_area;
}

static void merge_area(AsmArea* area, AsmArea* _area)
{
	for (int i = 0; i < _area->lines_count; i++)
	{
		AsmLine* curr = _area->lines[i];

		add_line_to_area(area, curr);
	}
}

static AsmArea* generate_else(CodeGen* code_gen, Node* else_, AsmArea* area, int if_, int id)
{
	AsmArea* else_area = create_area();
	
	char buff[50];
	char _buff[64];

	Node* next = NULL;

	if (else_->type == NODE_IF)
	{
		IfNode* nested_if = &else_->if_statement_node.if_statement;
		
		code_gen->scope = nested_if->then_scope;

		AsmArea* then_area = generate_then(code_gen, nested_if->condition_top, nested_if->then_branch, area, id + 1);
		merge_area(text_section, then_area);

		if (nested_if->else_branch != NULL)
		{
			code_gen->scope = nested_if->else_scope;
			
			AsmArea* _else_area = generate_else(code_gen, nested_if->else_branch, area, 0, id);
			merge_area(text_section, _else_area);

			return else_area;
		}
	}
	else
	{
		if (!if_)
		{
			snprintf(_buff, 64, ".if_else_%d", id);
			
			AsmLine* label = generate_label(_buff);
			add_line_to_area(else_area, label);
		}
		
		next = else_->block_node.block.statements->head;

		while (next != NULL)
		{
			code_gen_node(code_gen, next, 0, else_area);
			next = next->next;
		}

		if (!if_)
		{
			AsmLine* jmp_line = create_line();

			snprintf(buff, 50, "	jmp	%s", _buff);
			jmp_line->line = strdup(buff);

			add_line_to_area(area, jmp_line);
		}
	}

	AsmLine* ret_line = create_line();

	snprintf(buff, 50, "	jmp	.if_post_%d", ifs_count);
	ret_line->line = strdup(buff);

	add_line_to_area(else_area, ret_line);

	return else_area;
}

static void generate_if(CodeGen* code_gen, Node* node, AsmArea* area)
{
	int id = ifs_count;
	
	IfNode* if_node = &node->if_statement_node.if_statement;
	
	SymbolTable* temp = code_gen->scope = if_node->then_scope;
	
	code_gen->scope = if_node->then_scope;
	AsmArea* then_area = generate_then(code_gen, if_node->condition_top, if_node->then_branch, area, id);

	merge_area(text_section, then_area);
	
	if (if_node->else_branch != NULL)
	{
		code_gen->scope = if_node->else_scope;
		AsmArea* else_area = generate_else(code_gen, if_node->else_branch, area, 0, id);

		merge_area(text_section, else_area);
	}
	
	code_gen->scope = temp;
	generate_post(code_gen, if_node->else_branch, area, id);

	ifs_count++;
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

		case NODE_FUNCTION_CALL:
		{
			generate_function_call(code_gen, node, area, 0, 0);

			return;
		}

		case NODE_OPERATION:
		{
			generate_operation(code_gen, node, scope_depth, area, 0, 0);

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

		case NODE_RETURN:
		{
			generate_return(code_gen, node, scope_depth, area);

			return;
		}
		
		case NODE_CLASS:
		{
			generate_class(code_gen, node, area);

			return;
		}

		case NODE_IF:
		{
			generate_if(code_gen, node, area);

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