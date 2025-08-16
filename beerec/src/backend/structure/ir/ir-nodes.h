#ifndef IR_NODES_H
#define IR_NODES_H

typedef struct IRNode IRNode;
typedef struct IRNodeBlock IRNodeBlock;
typedef struct IRNodeOperation IRNodeOperation;
typedef struct IRNodeRet IRNodeRet;
typedef struct IRNodeList IRNodeList;
typedef struct IRNodeCall IRNodeCall;
typedef struct IRNodeStore IRNodeStore;
typedef struct IRNodeLiteral IRNodeLiteral;

typedef struct VirtualReg VirtualReg;

typedef enum
{
	IR_NODE_BLOCK,
	IR_NODE_STORE,
	IR_NODE_LITERAL,
	IR_NODE_OPERATION,
}
IRNodeType;

typedef enum
{
	IR_OPERATION_ADD,
	IR_OPERATION_SUB,
	IR_OPERATION_MUL,
	IR_OPERATION_DIV
}
IROperationType;

typedef enum
{
	IR_LITERAL_INT,
	IR_LITERAL_FLOAT,
	IR_LITERAL_DOUBLE,
	IR_LITERAL_STRING,
	IR_LITERAL_BOOL,
	IR_LITERAL_CHAR
}
IRLiteralType;

struct IRNodeList
{
	IRNode** elements;

	int length;
	int capacity;
};

struct IRNodeBlock
{
	IRNodeList* instructions;
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

struct IRNodeCall
{
	char* func;
};

struct IRNodeStore
{
	VirtualReg* dest;
	IRNode* src;
};

struct IRNodeLiteral
{
	IRLiteralType type;

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

struct IRNode
{
	int id; // -1 caso não seja relativo a uma IRNode geral de uma função
	IRNodeType type;

	IRNodeBlock block;
	IRNodeOperation operation;
	IRNodeRet ret;
	IRNodeCall call;
	IRNodeStore store;
	IRNodeLiteral literal;
};

#endif