#ifndef IR_NODES_H
#define IR_NODES_H

#include "../../../frontend/structure/ast/types/types.h"
#include "../../../utils/list/list.h"

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
typedef struct IRNodeFieldLiteral IRNodeFieldLiteral;
typedef struct IRNodeReference IRNodeReference;
typedef struct IRNodeBranch IRNodeBranch;
typedef struct IRNodeParam IRNodeParam;
typedef struct IRNodeGoto IRNodeGoto;
typedef struct IRType IRType;

typedef enum
{
	IR_NODE_BLOCK,
	IR_NODE_STORE,
	IR_NODE_LITERAL,
	IR_NODE_OPERATION,
	IR_NODE_RET,
	IR_NODE_CALL,
	IR_NODE_BRANCH,
	IR_NODE_GOTO,
	IR_NODE_FIELD,
	IR_NODE_FUNC,
	IR_NODE_DEREFERENCE,
	IR_NODE_REFERENCE,
	IR_NODE_FIELD_LITERAL,
	IR_NODE_PARAM
}
IRNodeType;

typedef enum
{
	IR_OPERATION_ADD,
	IR_OPERATION_ADD_EQUALS,
	IR_OPERATION_SUB,
	IR_OPERATION_SUB_EQUALS,
	IR_OPERATION_MUL,
	IR_OPERATION_MUL_EQUALS,
	IR_OPERATION_DIV,
	IR_OPERATION_DIV_EQUALS,
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

	int      length;
	int      capacity;
};

struct IRNodeBlock
{
	char*  label;
	DList* nodes;
};

struct IRNodeOperation
{
	IROperationType type;

	IRNode*         left;
	IRNode*         right;
};

struct IRNodeRet
{
	IRNode* value;
};

struct IRNodeFunc
{
	Type*        type;

	char*        name;

	DList*       blocks;

	IRNode**     params;
	unsigned int params_size;
};

struct IRNodeCall
{
	IRNode* func;
	DList*  args;
};

struct IRNodeStore
{
	IRNode* dest;
	IRNode* expr;
};

struct IRNodeGoto
{
	IRNode* block;
};

struct IRNodeBranch
{
	IRNode* condition;

	IRNode* then_block;
	IRNode* else_block;
};

struct IRNodeLiteral
{
	Type*          type;

	union
	{
		int    int_val;
		float  float_val;
		double double_val;
		char*  string_val;
		char   char_val;
		int    bool_val;
	};
};

struct IRNodeField
{
	Type*   type;

	char*   name;
	IRNode* value;
};

struct IRNodeParam
{
	Type* type;

	char* name;
};

struct IRNodeDereference
{
	IRNode* expr;
};

struct IRNodeReference
{
	IRNode* expr;
};

struct IRNodeFieldLiteral
{
	char* name;
};

struct IRNode
{
	IRNodeType type;

	IRNodeBlock        block;
	IRNodeOperation    operation;
	IRNodeRet          ret;
	IRNodeCall         call;
	IRNodeStore        store;
	IRNodeLiteral      literal;
	IRNodeDereference  dereference;
	IRNodeReference    reference;
	IRNodeFunc         func;
	IRNodeField        field;
	IRNodeFieldLiteral field_literal;
	IRNodeGoto         go_to;
	IRNodeBranch       branch;
	IRNodeParam        param;
};

IRNode* create_ir_node(IRNodeType type);

#endif
