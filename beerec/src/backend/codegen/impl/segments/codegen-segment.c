#include <string.h>
#include <stdlib.h>

#include "codegen-segment.h"

static char* generate_segment_node(SegmentNode* node);

extern char* get_reference_access_size(CodeGen* codegen, Type* type);

static char* _generate_segment_literal(SegmentNode* node)
{
	char buff[64];

	if (node->literal->type == SEGMENT_LITERAL_INT)
	{
		snprintf(buff, 64, "%d", node->literal->integer);

		return strdup(buff);
	}

	return NULL;
}

static char* _generate_segment_operation(SegmentNode* node)
{
	char buff[64];
	char* operator = "";

	char* left = generate_segment_node(node->operation->left);
	char* right = generate_segment_node(node->operation->right);
	
	if (node->operation->op == SEGMENT_OPERATION_ADD)
	{
		operator = "+";
	}

	if (node->operation->op == SEGMENT_OPERATION_SUB)
	{
		operator = "-";
	}

	snprintf(buff, 64, "%s%s%s", left, operator, right);

	return strdup(buff);
}

static char* generate_segment_node(SegmentNode* node)
{
	char buff[64];

	if (node->type == SEGMENT_NODE_LITERAL)
	{
		return _generate_segment_literal(node);
	}

	if (node->type == SEGMENT_NODE_OPERATION)
	{
		return _generate_segment_operation(node);
	}

	return NULL;
}

char* generate_segment(CodeGen* codegen, SegmentNode* node, Type* type)
{
	char buff[64];

	char* segment = generate_segment_node(node);
	char* access_specifier = get_reference_access_size(codegen, type);

	snprintf(buff, 64, "%s [%s]", access_specifier, segment);

	free(segment);

	return strdup(buff);
}

void unuse_segment_registers(SegmentNode* node)
{
	if (node->type == SEGMENT_REGISTER)
	{
		unuse_register(node->reg->reg);
	}

	if (node->type == SEGMENT_NODE_OPERATION)
	{
		unuse_segment_registers(node->operation->left);
		unuse_segment_registers(node->operation->right);
	}
}