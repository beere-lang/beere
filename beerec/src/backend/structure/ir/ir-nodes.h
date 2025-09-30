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
typedef struct IRNodeMemberAccess IRNodeMemberAccess;
typedef struct IRNodeParam IRNodeParam;
typedef struct IRNodeArgument IRNodeArgument;
typedef struct IRNodeCast IRNodeCast;
typedef struct IRNodeGoto IRNodeGoto;
typedef struct IRNodePhi IRNodePhi;
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
	IR_NODE_MEMBER_ACCESS,
	IR_NODE_REFERENCE,
	IR_NODE_FIELD_LITERAL,
	IR_NODE_PARAM,
        IR_NODE_ARGUMENT,
	IR_NODE_CAST,
	IR_NODE_PHI
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
	IR_OPERATION_LESS_EQUALS,
	IR_OPERATION_AND,
	IR_OPERATION_OR
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
	DList* phis;

	DList* nodes;
};
 
struct IRNodeOperation
{
	IROperationType operation;
	Type*           type;

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

	DList**      frontiers;
	unsigned int frontiers_length;

	IRNode**     fields;
	unsigned int fields_length;
};

struct IRNodeCall
{
	IRNode* func;
	DList*  args;
};

struct IRNodeStore
{
	char*   ssa_label;

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

struct IRNodeMemberAccess
{
	IRNode* object;
	char*   member;

	int     is_func;
};

struct IRNodeArgument
{
        IRNode* value;
};

struct IRNodeCast
{
	IRNode* expr;

	Type*   from;
	Type*   to;
};

struct IRNodePhi
{
	DList* labels;
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
	IRNodeMemberAccess member_access;
        IRNodeArgument     argument;
	IRNodeCast         cast;
	IRNodePhi          phi;
};

IRNode* create_ir_node(IRNodeType type);

#endif
