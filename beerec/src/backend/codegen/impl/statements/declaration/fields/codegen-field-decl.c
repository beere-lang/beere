#include <string.h>
#include <stdlib.h>

#include "codegen-field-decl.h"
#include "../../../expressions/codegen-expr.h"
#include "../../../../../../frontend/semantic/analyzer/analyzer.h"

char* field_get_reference_access_size(CodeGen* codegen, Type* type)
{
	switch (type->type)
	{
		case TYPE_INT:
		{
			return strdup("dword");
		}

		case TYPE_FLOAT:
		{
			return strdup("dword");
		}

		case TYPE_DOUBLE:
		{
			return strdup("qword");
		}

		case TYPE_PTR:
		{
			return strdup("qword");
		}

		case TYPE_STRING:
		{
			return strdup("qword");
		}

		case TYPE_CLASS:
		{
			return strdup("qword");
		}

		case TYPE_BOOL:
		{
			return strdup("byte");
		}

		default:
		{
			printf("Codegen debug fail #204...\n");
			exit(1);
		}
	}
}

char* field_get_mov_op_code_access(CodeGen* codegen, Type* type)
{
	switch (type->type)
	{
		case TYPE_INT:
		{
			return strdup("mov");
		}

		case TYPE_FLOAT:
		{
			return strdup("movss");
		}

		case TYPE_DOUBLE:
		{
			return strdup("movsd");
		}

		case TYPE_PTR:
		{
			return strdup("mov");
		}

		case TYPE_STRING:
		{
			return strdup("mov");
		}

		case TYPE_CLASS:
		{
			return strdup("mov");
		}

		case TYPE_BOOL:
		{
			return strdup("mov");
		}

		default:
		{
			printf("Codegen debug fail #2774...\n");
			exit(1);
		}
	}
}

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

char* get_literal_value(LiteralNode* literal)
{
	char buff[64];
	
	switch (literal->literal_type->type)
	{
		case TYPE_INT:
		{
			snprintf(buff, 64, "%s %d", generate_size_type(literal->literal_type->type), literal->int_value);
			
			return strdup(buff);
		}

		case TYPE_FLOAT:
		{
			snprintf(buff, 64, "%s %f", generate_size_type(literal->literal_type->type), literal->float_value);

			return strdup(buff);
		}

		case TYPE_DOUBLE:
		{
			snprintf(buff, 64, "%s %f", generate_size_type(literal->literal_type->type), literal->double_value);
			
			return strdup(buff);
		}

		case TYPE_STRING:
		{
			snprintf(buff, 64, "%s %s", generate_size_type(literal->literal_type->type), literal->string_value);
			return strdup(buff);
		}
		
		default:
		{
			printf("[Codegen] Invalid constant value type...\n");
			exit(1);
		}
	}
}

void generate_local_field_declaration(CodeGen* codegen, Node* node, AsmArea* area)
{
	char buff[64];

	Symbol* symbol = analyzer_find_symbol_from_scope(node->declare_node.declare.identifier, codegen->scope, 1, 0, 0, 0);
	
	AsmReturn* expr = generate_expression(codegen, node->declare_node.declare.default_value, area, 1, 0, 0, 0);
		
	snprintf(buff, 64, "	%s	%s [rbp%+d], %s", field_get_mov_op_code_access(codegen, expr->type), field_get_reference_access_size(codegen, symbol->symbol_variable->type), symbol->symbol_variable->offset, expr->result);
	add_line_to_area(area, buff);
}

void generate_global_field_declaration(CodeGen* codegen, Node* node, AsmArea* area)
{
	char buff[64];

	if (node->declare_node.declare.default_value->type != NODE_LITERAL)
	{
		printf("[Codegen] Global fields requires compile-time known values for initializer...\n");
		exit(1);
	}
	
	Symbol* symbol = analyzer_find_symbol_from_scope(node->declare_node.declare.identifier, codegen->scope, 1, 0, 0, 0);
		
	snprintf(buff, 64, "%s:	%s", node->declare_node.declare.identifier, get_literal_value(&node->declare_node.declare.default_value->literal_node.literal));
	add_line_to_area(data_section, buff);
}

void generate_static_field_declaration(CodeGen* codegen, Node* node, AsmArea* area)
{
	char buff[64];

	if (node->declare_node.declare.default_value->type != NODE_LITERAL)
	{
		printf("[Codegen] Static fields requires compile-time known values for initializer...\n");
		exit(1);
	}
	
	Symbol* symbol = analyzer_find_symbol_from_scope(node->declare_node.declare.identifier, codegen->scope, 1, 0, 0, 0);
		
	snprintf(buff, 64, ".static_%s:	%s", node->declare_node.declare.identifier, get_literal_value(&node->declare_node.declare.default_value->literal_node.literal));
	add_line_to_area(data_section, buff);
}

void generate_field_declaration(CodeGen* codegen, Node* node, AsmArea* area)
{
	if (node->declare_node.declare.is_static)
	{
		generate_static_field_declaration(codegen, node, area);
		
		return;
	}

	if (codegen->scope->scope_kind == GLOBAL_SCOPE)
	{
		generate_global_field_declaration(codegen, node, area);

		return;
	}

	if (node->declare_node.declare.default_value == NULL)
	{
		return;
	}

	generate_local_field_declaration(codegen, node, area);
}