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
typedef struct IRNodeIf IRNodeIf;
typedef struct IRNodeFunc IRNodeFunc;
typedef struct IRNodeField IRNodeField;
typedef struct IRNodeDereference IRNodeDereference;
typedef struct IRNodeReference IRNodeReference;
typedef struct IRType IRType;

typedef enum
{
	IR_NODE_BLOCK,
	IR_NODE_STORE,
	IR_NODE_LITERAL,
	IR_NODE_OPERATION,
	IR_NODE_RET,
	IR_NODE_CALL,
	IR_NODE_IF,
	IR_NODE_FIELD,
	IR_NODE_FUNC,
	IR_NODE_DEREFERENCE,
	IR_NODE_REFERENCE
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
	Type* type;

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

struct IRNodeIf
{
	IRNode* cond;

	IRNode* then_block;
	IRNode* else_block;
};

struct IRNodeField
{
	Type* type;

	char* name;
	IRNode* value;
};

struct IRNodeDereference
{
	IRNode* expr;
};

struct IRNodeReference
{
	IRNode* expr;
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
	IRNodeIf if_statement;
	IRNodeDereference dereference;
	IRNodeReference reference;
	IRNodeFunc func;
	IRNodeField field;
};

IRNode* create_ir_node(IRNodeType type);

#endif 