#ifndef IR_NODES_H
#define IR_NODES_H
/*
	CRIADO EM 13/08/2025 - USAR FUTURAMENTE PRA OTIMIZAÇÕES E MELHOR ORGANIZAÇÃO

typedef struct Register Register;
typedef struct Type Type;

typedef struct IRNode IRNode;
typedef struct IRNodeList IRNodeList;
typedef struct IRNodeCast IRNodeCast;
typedef struct IRNodeReturn IRNodeReturn;
typedef struct IRNodeLiteral IRNodeLiteral;
typedef struct IRNodeOperation IRNodeOperation;
typedef struct IRNodeReference IRNodeReference;
typedef struct IRNodeIdentifier IRNodeIdentifier;
typedef struct IRNodeDereference IRNodeDereference;
typedef struct IRNodeAssign IRNodeAssign;
typedef struct IRNodeWhile IRNodeWhile;
typedef struct IRNodeFor IRNodeFor;
typedef struct IRNodeIf IRNodeIf;
typedef struct IRNodeParameter IRNodeParameter;
typedef struct IRNodeSwitchStatement IRNodeSwitchStatement;
typedef struct IRNodeCase IRNodeCase;
typedef struct IRNodeArrayAccess IRNodeArrayAccess;
typedef struct IRNodeBlock IRNodeBlock;
typedef struct IRNodeClass IRNodeClass;
typedef struct IRNodeMethod IRNodeMethod;
typedef struct IRNodeField IRNodeField;
typedef struct IRNodeCreateInstance IRNodeCreateInstance;
typedef struct IRNodeMethodCall IRNodeMethodCall;
typedef struct IRNodeMemberAccess IRNodeMemberAccess;

typedef enum
{
	IR_NODE_OPERATION, //
	IR_NODE_METHOD, //
	IR_NODE_FIELD, //
	IR_NODE_IDENTIFIER, //
	IR_NODE_LITERAL, //
	IR_NODE_METHOD_CALL, //
	IR_NODE_DEREFERENCE, //
	IR_NODE_REFERENCE, //
	IR_NODE_RETURN, //
	IR_NODE_WHILE, //
	IR_NODE_FOR, //
	IR_NODE_BLOCK, //
	IR_NODE_CLASS,
	IR_NODE_THIS, //
	IR_NODE_SUPER, //
	IR_NODE_BREAK, //
	IR_NODE_ARRAY_ACCESS, //
	IR_NODE_CREATE_INSTANCE, //
	IR_NODE_CAST, //
	IR_NODE_ASSIGN, //
	IR_NODE_PARAMETER, //
	IR_NODE_IF, //
	IR_NODE_SWITCH_STATEMENT, //
	IR_NODE_CASE, //
	IR_NODE_CONTINUE, //
	IR_NODE_MEMBER_ACCESS, //
}
IRNodeType;

struct IRNodeBlock //
{
	IRNodeList* statements;
};

struct IRNodeList //
{
	IRNode* head;
};

struct IRNodeOperation //
{
	IRNode* left;
	IRNode* right;

	Register* blacklisted_registers;
};

struct IRNodeLiteral //
{
	Type* type;

	union
	{
		int integer;
		int boolean;
		char character;
		char* string;
	};
	
};

struct IRNodeIdentifier //
{
	char* identifier;
};

struct IRNodeDereference //
{
	IRNode* expression;
};

struct IRNodeReference //
{
	IRNode* expression;
};

struct IRNodeMethodCall //
{
	IRNode* callee;
	IRNodeList* arguments;
};

struct IRNodeReturn //
{
	IRNode* expression; // NULL quando void.
};

struct IRNodeCast //
{
	Type* type;
	IRNode* expression;
};

struct IRNodeArrayAccess //
{
	IRNode* expression;
	IRNode* index;
};

struct IRNodeCreateInstance //
{
	char* name;
	IRNodeList* arguments;
};

struct IRNodeAssign //
{
	IRNode* expression;
	IRNode* value;
};

struct IRNodeParameter //
{
	Type* type;
	char* name;
};

struct IRNodeMethod //
{
	Type* type;

	char* name;
	
	IRNode* parameters;
	IRNode* block;
};

struct IRNodeIf //
{
	IRNode* condition;
	IRNode* then;
	IRNode* else_;
};

struct IRNodeWhile //
{
	IRNode* condition;
	IRNode* then;
};

struct IRNodeFor //
{
	IRNode* init; // statement executado antes de iniciar o loop pela primeira vez.
	IRNode* condition;
	IRNode* athen; // statement executado antes de recomeçar o loop.

	IRNode* then; // then block.
};

struct IRNodeField //
{
	Type* type;

	char* name;
	IRNode* value; // NULL caso não inicializada.
};

struct IRNodeSwitchStatement //
{
	IRNode* expression;
	IRNodeList* cases;
};

struct IRNodeCase //
{
	IRNode* value;
	IRNode* then;
};

struct IRNodeClass //
{
	char* super;
	char* name;

	IRNodeList* fields;
	IRNodeList* methods;
};

struct IRNodeMemberAccess
{
	IRNode* object;
	char* field;
};

struct IRNode
{
	IRNodeType type;
	IRNode* next; // usado pra listas.

	union 
	{
		IRNodeOperation* operation;
		IRNodeLiteral* literal;
		IRNodeIdentifier* identifier;
		IRNodeReturn* return_;
		IRNodeCast* cast;
		IRNodeCase* case_;
		IRNodeSwitchStatement* switch_;
		IRNodeFor* for_;
		IRNodeWhile* while_;
		IRNodeClass* class_;
		IRNodeField* variable;
		IRNodeMethod* method;
		IRNodeIf* if_;
		IRNodeCreateInstance* instance;
		IRNodeArrayAccess* array_access;
		IRNodeMethodCall* method_call;
		IRNodeReference* reference;
		IRNodeDereference* dereference;
		IRNodeBlock* block;
		IRNodeMemberAccess* member_access;
	};
};
*/

#endif