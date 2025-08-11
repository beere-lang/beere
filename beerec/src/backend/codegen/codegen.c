#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "codegen.h"
#include "impl/expressions/operations/codegen-op.h"
#include "impl/oop/codegen-class.h"
#include "impl/segments/codegen-segment.h"
#include "impl/statements/assign/fields/codegen-field-assgn.h"
#include "impl/statements/break/codegen-break.h"
#include "impl/statements/calls/codegen-method-call.h"
#include "impl/statements/continue/codegen-continue.h"
#include "impl/statements/declaration/fields/codegen-field-decl.h"
#include "impl/statements/declaration/methods/codegen-method-decl.h"
#include "impl/statements/for-loop/codegen-for.h"
#include "impl/statements/if/codegen-if.h"
#include "impl/statements/return/codegen-return.h"
#include "impl/statements/switch/codegen-switch.h"
#include "impl/statements/while-loop/codegen-while.h"

extern char* get_literal_value(LiteralNode* literal);

ConstantTable* constant_table = NULL;
AsmArea* externs_section = NULL;
AsmArea* text_section = NULL;
AsmArea* data_section = NULL;
AsmArea* rodata_section = NULL;
AsmArea* bss_section = NULL;
ExternTable* extern_table = NULL;
ClassOffsetsTable* class_offsets_table = NULL;
RegistersTable* registers_table = NULL;
MethodRegisterStackTable* register_stack_table = NULL;

static SegmentNode* generate_segment_node(SegmentNodeType type)
{
	SegmentNode* node = malloc(sizeof(SegmentNode));
	
	node->type = type;

	return node;
}

SegmentNode* generate_segment_register(Register* reg)
{
	SegmentRegister* segment_reg = malloc(sizeof(SegmentRegister));

	segment_reg->reg = reg;

	SegmentNode* node = generate_segment_node(SEGMENT_REGISTER);
	node->reg = segment_reg;

	return node;
}

SegmentNode* generate_segment_literal(SegmentLiteralType type, int integer) // value needs to be the same from type
{
	SegmentLiteral* literal = malloc(sizeof(SegmentLiteral));

	literal->type = type;
	literal->integer = integer;

	SegmentNode* node = generate_segment_node(SEGMENT_NODE_LITERAL);
	node->literal = literal;

	return node;
}

SegmentNode* generate_segment_operation(SegmentNode* left, SegmentNode* right, SegmentOperationType type)
{
	SegmentOperation* operation = malloc(sizeof(SegmentOperation));

	operation->op = type;
	operation->left = left;
	operation->right = right;

	SegmentNode* node = generate_segment_node(SEGMENT_NODE_OPERATION);
	node->operation = operation;

	return node;
}

static void setup_register_stack_table()
{
	MethodRegisterStackTable* stack_table = malloc(sizeof(MethodRegisterStackTable));

	stack_table->stacks = malloc(sizeof(MethodRegisterStack*) * 4);
	stack_table->stacks_capacity = 4;
	stack_table->stacks_length = 0;

	register_stack_table = stack_table;
}

static void add_stack_to_table(MethodRegisterStack* stack)
{
	if (register_stack_table->stacks_length >= register_stack_table->stacks_capacity)
	{
		register_stack_table->stacks_capacity *= 2;

		register_stack_table->stacks = realloc(register_stack_table->stacks, sizeof(RegistersClass*) * register_stack_table->stacks_capacity);
	}

	register_stack_table->stacks[register_stack_table->stacks_length] = stack;
	register_stack_table->stacks_length++;
}

static void setup_registers_table()
{
	RegistersTable* table = malloc(sizeof(RegistersTable));

	table->registers_classes = malloc(sizeof(RegistersClass*) * 4);

	table->registers_classes_capacity = 4;
	table->registers_classes_length = 0;
}

static void add_class_to_table(RegistersClass* class)
{
	if (registers_table->registers_classes_length >= registers_table->registers_classes_capacity)
	{
		registers_table->registers_classes_capacity *= 2;

		registers_table->registers_classes = realloc(registers_table->registers_classes, sizeof(RegistersClass*) * registers_table->registers_classes_capacity);
	}

	registers_table->registers_classes[registers_table->registers_classes_length] = class;
	registers_table->registers_classes_length++;
}

static RegistersClass* setup_registers_class(RegistersClassType type)
{
	RegistersClass* class = malloc(sizeof(RegistersClass));

	class->registers = malloc(sizeof(Register*) * 4);

	class->class = type;

	class->registers_capacity = 4;
	class->registers_length = 0;

	return class;
}

static void add_register_to_class(Register* reg, RegistersClass* class)
{
	if (class->registers_length >= class->registers_capacity)
	{
		class->registers_capacity *= 2;

		class->registers = realloc(class->registers, sizeof(Register*) * class->registers_capacity);
	}

	class->registers[class->registers_length] = reg;
	class->registers_length++;
}

static Register* setup_register(BitsSize size, char* name, Register* parent)
{
	Register* reg = malloc(sizeof(Register));

	reg->in_use = 0;
	reg->reg = strdup(name);
	reg->parent = parent;
	
	return reg;
}

static RegistersClass* find_registers_class(RegistersClassType type)
{
	for (int i = 0; i < registers_table->registers_classes_length; i++)
	{
		RegistersClass* class = registers_table->registers_classes[i];
		
		if (class->class == type)
		{
			return class;
		}
	}

	return NULL;
}

static void init_registers_table()
{
	{
		RegistersClass* floats_class = setup_registers_class(CLASS_FLOATS);

		Register* xmm0 = setup_register(BITS_SIZE_64, "xmm0", NULL);
		Register* xmm1 = setup_register(BITS_SIZE_64, "xmm1", NULL);
		Register* xmm2 = setup_register(BITS_SIZE_64, "xmm2", NULL);
		Register* xmm3 = setup_register(BITS_SIZE_64, "xmm3", NULL);
		Register* xmm4 = setup_register(BITS_SIZE_64, "xmm4", NULL);
		Register* xmm5 = setup_register(BITS_SIZE_64, "xmm5", NULL);
		Register* xmm6 = setup_register(BITS_SIZE_64, "xmm6", NULL);
		Register* xmm7 = setup_register(BITS_SIZE_64, "xmm7", NULL);
		Register* xmm8 = setup_register(BITS_SIZE_64, "xmm8", NULL);
		Register* xmm9 = setup_register(BITS_SIZE_64, "xmm9", NULL);
		Register* xmm10 = setup_register(BITS_SIZE_64, "xmm10", NULL);
		Register* xmm11 = setup_register(BITS_SIZE_64, "xmm11", NULL);
		Register* xmm12 = setup_register(BITS_SIZE_64, "xmm12", NULL);
		Register* xmm13 = setup_register(BITS_SIZE_64, "xmm13", NULL);
		Register* xmm14 = setup_register(BITS_SIZE_64, "xmm14", NULL);
		Register* xmm15 = setup_register(BITS_SIZE_64, "xmm15", NULL);

		add_register_to_class(xmm0, floats_class);
		add_register_to_class(xmm1, floats_class);
		add_register_to_class(xmm2, floats_class);
		add_register_to_class(xmm3, floats_class);
		add_register_to_class(xmm4, floats_class);
		add_register_to_class(xmm5, floats_class);
		add_register_to_class(xmm6, floats_class);
		add_register_to_class(xmm7, floats_class);
		add_register_to_class(xmm8, floats_class);
		add_register_to_class(xmm9, floats_class);
		add_register_to_class(xmm10, floats_class);
		add_register_to_class(xmm11, floats_class);
		add_register_to_class(xmm12, floats_class);
		add_register_to_class(xmm13, floats_class);
		add_register_to_class(xmm14, floats_class);
		add_register_to_class(xmm15, floats_class);

		add_class_to_table(floats_class);
	}

	{
		RegistersClass* generals_class = setup_registers_class(CLASS_GENERALS);

		Register* rax = setup_register(BITS_SIZE_64, "rax", NULL);
		Register* eax = setup_register(BITS_SIZE_32, "eax", rax);
		Register* ax = setup_register(BITS_SIZE_16, "ax", eax);
		Register* al = setup_register(BITS_SIZE_8, "al", ax);

		rax->child = eax;
		eax->child = ax;
		ax->child = al;
		al->child = NULL;

		Register* rbx = setup_register(BITS_SIZE_64, "rbx", NULL);
		Register* ebx = setup_register(BITS_SIZE_32, "ebx", rbx);
		Register* bx = setup_register(BITS_SIZE_16, "bx", ebx);
		Register* bl = setup_register(BITS_SIZE_8, "bl", bx);

		rbx->child = ebx;
		ebx->child = bx;
		bx->child = bl;
		bl->child = NULL;

		Register* rcx = setup_register(BITS_SIZE_64, "rcx", NULL);
		Register* ecx = setup_register(BITS_SIZE_32, "ecx", rcx);
		Register* cx = setup_register(BITS_SIZE_16, "cx", ecx);
		Register* cl = setup_register(BITS_SIZE_8, "cl", cx);

		rcx->child = ecx;
		ecx->child = cx;
		cx->child = cl;
		cl->child = NULL;

		Register* rdx = setup_register(BITS_SIZE_64, "rdx", NULL);
		Register* edx = setup_register(BITS_SIZE_32, "edx", rdx);
		Register* dx = setup_register(BITS_SIZE_16, "dx", edx);
		Register* dl = setup_register(BITS_SIZE_8, "dl", dx);

		rdx->child = edx;
		edx->child = dx;
		dx->child = dl;
		dl->child = NULL;

		Register* rdi = setup_register(BITS_SIZE_64, "rdi", NULL);
		Register* edi = setup_register(BITS_SIZE_32, "edi", rdi);
		Register* di = setup_register(BITS_SIZE_16, "di", edi);
		Register* dil = setup_register(BITS_SIZE_8, "dil", di);

		rdi->child = edi;
		edi->child = di;
		di->child = dil;
		dil->child = NULL;

		Register* rsi = setup_register(BITS_SIZE_64, "rsi", NULL);
		Register* esi = setup_register(BITS_SIZE_32, "esi", rsi);
		Register* si = setup_register(BITS_SIZE_16, "si", esi);
		Register* sil = setup_register(BITS_SIZE_8, "sil", si);

		rsi->child = esi;
		esi->child = si;
		si->child = sil;
		sil->child = NULL;

		Register* r8 = setup_register(BITS_SIZE_64, "r8", NULL);
		Register* r8d = setup_register(BITS_SIZE_32, "r8d", r8);
		Register* r8w = setup_register(BITS_SIZE_16, "r8w", r8d);
		Register* r8b = setup_register(BITS_SIZE_8, "r8b", r8w);

		r8->child = r8d;
		r8d->child = r8w;
		r8w->child = r8b;
		r8b->child = NULL;

		Register* r9 = setup_register(BITS_SIZE_64, "r9", NULL);
		Register* r9d = setup_register(BITS_SIZE_32, "r9d", r9);
		Register* r9w = setup_register(BITS_SIZE_16, "r9w", r9d);
		Register* r9b = setup_register(BITS_SIZE_8, "r9b", r9w);

		r9->child = r9d;
		r9d->child = r9w;
		r9w->child = r9b;
		r9b->child = NULL;

		Register* r10 = setup_register(BITS_SIZE_64, "r10", NULL);
		Register* r10d = setup_register(BITS_SIZE_32, "r10d", r10);
		Register* r10w = setup_register(BITS_SIZE_16, "r10w", r10d);
		Register* r10b = setup_register(BITS_SIZE_8, "r10b", r10w);

		r10->child = r10d;
		r10d->child = r10w;
		r10w->child = r10b;
		r10b->child = NULL;
		
		Register* r11 = setup_register(BITS_SIZE_64, "r11", NULL);
		Register* r11d = setup_register(BITS_SIZE_32, "r11d", r11);
		Register* r11w = setup_register(BITS_SIZE_16, "r11w", r11d);
		Register* r11b = setup_register(BITS_SIZE_8, "r11b", r11w);

		r11->child = r11d;
		r11d->child = r11w;
		r11w->child = r11b;
		r11b->child = NULL;
		
		Register* r12 = setup_register(BITS_SIZE_64, "r12", NULL);
		Register* r12d = setup_register(BITS_SIZE_32, "r12d", r12);
		Register* r12w = setup_register(BITS_SIZE_16, "r12w", r12d);
		Register* r12b = setup_register(BITS_SIZE_8, "r12b", r12w);

		r12->child = r12d;
		r12d->child = r12w;
		r12w->child = r12b;
		r12b->child = NULL;
		
		Register* r13 = setup_register(BITS_SIZE_64, "r13", NULL);
		Register* r13d = setup_register(BITS_SIZE_32, "r13d", r13);
		Register* r13w = setup_register(BITS_SIZE_16, "r13w", r13d);
		Register* r13b = setup_register(BITS_SIZE_8, "r13b", r13w);

		r13->child = r13d;
		r13d->child = r13w;
		r13w->child = r13b;
		r13b->child = NULL;
		
		Register* r14 = setup_register(BITS_SIZE_64, "r14", NULL);
		Register* r14d = setup_register(BITS_SIZE_32, "r14d", r14);
		Register* r14w = setup_register(BITS_SIZE_16, "r14w", r14d);
		Register* r14b = setup_register(BITS_SIZE_8, "r14b", r14w);

		r14->child = r14d;
		r14d->child = r14w;
		r14w->child = r14b;
		r14b->child = NULL;
		
		Register* r15 = setup_register(BITS_SIZE_64, "r15", NULL);
		Register* r15d = setup_register(BITS_SIZE_32, "r15d", r15);
		Register* r15w = setup_register(BITS_SIZE_16, "r15w", r15d);
		Register* r15b = setup_register(BITS_SIZE_8, "r15b", r15w);

		r15->child = r15d;
		r15d->child = r15w;
		r15w->child = r15b;
		r15b->child = NULL;

		add_register_to_class(rax, generals_class);
		add_register_to_class(eax, generals_class);
		add_register_to_class(ax, generals_class);
		add_register_to_class(al, generals_class);

		add_register_to_class(rbx, generals_class);
		add_register_to_class(ebx, generals_class);
		add_register_to_class(bx, generals_class);
		add_register_to_class(bl, generals_class);

		add_register_to_class(rcx, generals_class);
		add_register_to_class(ecx, generals_class);
		add_register_to_class(cx, generals_class);
		add_register_to_class(cl, generals_class);

		add_register_to_class(rdx, generals_class);
		add_register_to_class(edx, generals_class);
		add_register_to_class(dx, generals_class);
		add_register_to_class(dl, generals_class);

		add_register_to_class(rdi, generals_class);
		add_register_to_class(edi, generals_class);
		add_register_to_class(di, generals_class);
		add_register_to_class(dil, generals_class);

		add_register_to_class(rsi, generals_class);
		add_register_to_class(esi, generals_class);
		add_register_to_class(si, generals_class);
		add_register_to_class(sil, generals_class);

		add_register_to_class(r8, generals_class);
		add_register_to_class(r8d, generals_class);
		add_register_to_class(r8w, generals_class);
		add_register_to_class(r8b, generals_class);

		add_register_to_class(r9, generals_class);
		add_register_to_class(r9d, generals_class);
		add_register_to_class(r9w, generals_class);
		add_register_to_class(r9b, generals_class);

		add_register_to_class(r10, generals_class);
		add_register_to_class(r10d, generals_class);
		add_register_to_class(r10w, generals_class);
		add_register_to_class(r10b, generals_class);

		add_register_to_class(r11, generals_class);
		add_register_to_class(r11d, generals_class);
		add_register_to_class(r11w, generals_class);
		add_register_to_class(r11b, generals_class);

		add_register_to_class(r12, generals_class);
		add_register_to_class(r12d, generals_class);
		add_register_to_class(r12w, generals_class);
		add_register_to_class(r12b, generals_class);

		add_register_to_class(r13, generals_class);
		add_register_to_class(r13d, generals_class);
		add_register_to_class(r13w, generals_class);
		add_register_to_class(r13b, generals_class);

		add_register_to_class(r14, generals_class);
		add_register_to_class(r14d, generals_class);
		add_register_to_class(r14w, generals_class);
		add_register_to_class(r14b, generals_class);

		add_register_to_class(r15, generals_class);
		add_register_to_class(r15d, generals_class);
		add_register_to_class(r15w, generals_class);
		add_register_to_class(r15b, generals_class);

		add_class_to_table(generals_class);
	}
}

Register* find_register_by_name(char* name, Type* type)
{
	RegistersClass* class = find_registers_class(CLASS_GENERALS);

	if (type->type == TYPE_DOUBLE || type->type == TYPE_FLOAT)
	{
		class = find_registers_class(CLASS_FLOATS);
	}

	for (int i = 0; i < class->registers_length; i++)
	{
		Register* curr = class->registers[i];

		if (strcmp(name, curr->reg) == 0)
		{
			return curr;
		}
	}

	return NULL;
}

Register* find_register_piece_with_size(Register* reg, BitsSize size)
{
	if (reg == NULL)
	{
		return NULL;
	}

	if (reg->size == size)
	{
		return reg;
	}

	if (size > reg->size) // * a ordem declarada no enum importa *
	{
		return find_register_piece_with_size(reg->parent, size);
	}

	return find_register_piece_with_size(reg->child, size);
}

static void set_all_family_registers(int in_use, Register* reg)
{
	Register* parent_reg = reg->parent;
	
	while (parent_reg != NULL)
	{
		parent_reg->in_use = in_use;

		parent_reg = parent_reg->parent;
	}

	Register* child_reg = reg->child;

	while (child_reg != NULL)
	{
		child_reg->in_use = in_use;

		child_reg = child_reg->child;
	}

	reg->in_use = in_use;
}

MethodRegisterStack* find_method_stack(Symbol* method)
{
	for (int i = 0; i < register_stack_table->stacks_length; i++)
	{
		MethodRegisterStack* curr = register_stack_table->stacks[i];

		if (curr->method == method)
		{
			return curr;
		}
	}

	return NULL;
}

Register* find_and_use_register(Type* type, BitsSize size, Symbol* method)
{
	RegistersClass* class = find_registers_class(CLASS_GENERALS);

	if (type->type == TYPE_DOUBLE || type->type == TYPE_FLOAT)
	{
		class = find_registers_class(CLASS_FLOATS);
	}

	if (class == NULL)
	{
		exit(1);
	}

	for (int i = 0; i < class->registers_length; i++)
	{
		Register* curr = class->registers[i];

		if (curr->in_use)
		{
			continue;
		}

		if (curr->size == size)
		{
			set_all_family_registers(1, curr);

			return curr;
		}
	}

	return NULL;
}

void unuse_register(Register* reg)
{
	set_all_family_registers(0, reg);
}

MethodRegisterStack* setup_method_register_stack(Symbol* method)
{
	MethodRegisterStack* stack = malloc(sizeof(MethodRegisterStack));

	stack->method = method;
	
	stack->registers = malloc(sizeof(Register*) * 4);
	
	stack->registers_capacity = 4;
	stack->registers_length = 0;

	add_stack_to_table(stack);

	return stack;
}

Register* has_register_in_stack(MethodRegisterStack* stack, Register* reg)
{
	for (int i = 0; i < stack->registers_length; i++)
	{
		Register* curr = stack->registers[i];

		if (curr == reg) // todos os registers usados devem apontar pro mesmo endereço
		{
			return curr;
		}
	}

	return NULL;
}

char* get_asm_value(CodeGen* codegen, AsmReturnValue* value, Type* type)
{
	if (value->type == SEGMENT_RETURN_TYPE)
	{
		return generate_segment(codegen, value->segment, type);
	}

	if (value->type == REGISTER_RETURN_TYPE)
	{
		return strdup(value->reg->reg);
	}

	return NULL;
}

void unuse_asm_value(AsmReturnValue* value)
{
	if (value->type == SEGMENT_RETURN_TYPE)
	{
		unuse_segment_registers(value->segment);

		return;
	}

	if (value->type == REGISTER_RETURN_TYPE)
	{
		unuse_register(value->reg);

		return;
	}
}

void add_register_to_stack(MethodRegisterStack* stack, Register* reg)
{
	if (has_register_in_stack(stack, reg) != NULL)
	{
		return;
	}
	
	if (stack->registers_length >= stack->registers_capacity)
	{
		stack->registers_capacity *= 2;

		stack->registers = realloc(stack->registers, sizeof(Register*) * stack->registers_capacity);
	}

	stack->registers[stack->registers_length] = reg;
	stack->registers_length++;
}

AsmReturn* create_asm_return(SegmentNode* segment, Register* reg, Type* type, int is_reg)
{
	AsmReturn* asm_ret = malloc(asm_return_size);

	asm_ret->type = type;
	asm_ret->value->type = (is_reg) ? REGISTER_RETURN_TYPE : SEGMENT_RETURN_TYPE;

	asm_ret->value->segment = segment;
	asm_ret->value->reg = reg;
	
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

Constant* generate_directly_constant(double value, int is_double)
{
	Constant* constant = create_constant();
	char buff[64];

	char* size = (is_double) ? "dq" : "dd";
	snprintf(buff, 64, "%s %f", size, value);
	
	constant->value = strdup(buff);

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

	setup_registers_table();
	init_registers_table();

	setup_register_stack_table();
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

Symbol* find_owner_method(SymbolTable* scope)
{
	if (scope == NULL)
	{
		return NULL;
	}

	if (scope->scope_kind == SYMBOL_FUNCTION)
	{
		return scope->owner_statement;
	}

	return find_owner_method(scope->parent);
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

		case NODE_OPERATION:
		{
			generate_operation(codegen, node, area, 0);

			return;
		}

		case NODE_SWITCH_STATEMENT:
		{
			generate_switch(codegen, node, area);

			return;
		}

		case NODE_WHILE_LOOP:
		{
			generate_while(codegen, node, area);

			return;
		}

		case NODE_FOR_LOOP:
		{
			generate_for(codegen, node, area);

			return;
		}

		case NODE_CONTINUE:
		{
			generate_continue(codegen, node, area);

			return;
		}

		case NODE_BREAK:
		{
			generate_break(codegen, node, area);

			return;
		}
		
		default:
		{
			printf("Invalid node while generating node: %d...\n", node->type);
			exit(1);
		}
	}
}