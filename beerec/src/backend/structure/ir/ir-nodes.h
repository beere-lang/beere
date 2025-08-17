#ifndef IR_NODES_H 
#define IR_NODES_H

#include "../../../frontend/structure/ast/types/types.h"

typedef struct IRNode IRNode;
typedef struct IRNodeBlock IRNodeBlock;
typedef struct IRNodeOperation IRNodeOperation;
typedef struct IRNodeRet IRNodeRet;
typedef struct IRNodeList IRNodeList;
typedef struct IRNodeCall IRNodeCall;
typedef struct IRNodeStore IRNodeStore;
typedef struct IRNodeLiteral IRNodeLiteral;
typedef struct IRNodeGoto IRNodeGoto;
typedef struct IRNodeFunc IRNodeFunc;
typedef struct IRNodeField IRNodeField;
typedef struct IRType IRType;

typedef struct VirtualReg VirtualReg;

typedef enum
{
	IR_NODE_BLOCK,
	IR_NODE_STORE,
	IR_NODE_LITERAL,
	IR_NODE_OPERATION,
	IR_NODE_RET,
	IR_NODE_CALL,
	IR_NODE_GOTO
}
IRNodeType;

typedef enum
{
	IR_OPERATION_ADD,
	IR_OPERATION_SUB,
	IR_OPERATION_MUL,
	IR_OPERATION_DIV,
	IR_OPERATION_EQUALS,
	IR_OPERATION_NOT_EQUALS,
	IR_OPERATION_GREATER,
	IR_OPERATION_LESS,
	IR_OPERATION_GREATER_EQUALS,
	IR_OPERATION_LESS_EQUALS
}
IROperationType;

struct IRNodeList
{
	IRNode** elements;

	int length;
	int capacity;
};

struct IRNodeBlock
{
	IRNodeList* nodes;
};

struct IRNodeOperation
{
	IROperationType type;

	IRNode* left;
	IRNode* right;
};

struct IRNodeRet
{
	IRNode* value;
};

struct IRNodeFunc
{
	Type* type;
	
	char* name;
	IRNode* block;
};

struct IRNodeCall
{
	char* func;
};

struct IRNodeStore
{
	char* dest;
	IRNode* expr;
};

struct IRNodeLiteral
{
	VarType type;

	union
	{
		int int_val;
		float float_val;
		double double_val;
		char* string_val;
		char char_val;
		int bool_val;
	};
	
};

struct IRNodeGoto
{
	char* label;
	IRNode* cond;
};

struct IRNodeField
{
	Type* type;

	char* name;
	IRNode* value;
};

struct IRNode
{
	IRNodeType type;

	IRNodeBlock block;
	IRNodeOperation operation;
	IRNodeRet ret;
	IRNodeCall call;
	IRNodeStore store;
	IRNodeLiteral literal;
	IRNodeGoto jump;
	IRNodeFunc func;
	IRNodeField field;
};

IRNode* create_ir_node(IRNodeType type);

#endif 