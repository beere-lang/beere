#ifndef NODES_H
#define NODES_H

#include "../tokens/tokens.h"
#include "../types/types.h"

typedef struct ASTNode ASTNode;
typedef struct ASTNodeList ASTNodeList;
typedef struct SymbolTable SymbolTable;

typedef enum
{
	VISIBILITY_PUBLIC,
	VISIBILITY_PRIVATE
}
VisibilityType;

typedef struct
{
	SymbolTable* then_scope;
	SymbolTable* else_scope;
	
	ASTNode* condition_top;
	ASTNode* then_branch;
	ASTNode* else_branch;
} 
ASTNodeIf;

typedef struct
{
	ASTNode* left;
	ASTNode* right;
	
	TokenType op;
}
ASTNodeOperation;

typedef struct
{
	char* identifier;
	Type* var_type;
	
	ASTNode* default_value;

	int is_const;
	int is_static;

	int is_export;

	VisibilityType visibility;
}
ASTNodeField;

typedef struct
{
	char* identifier;

	ASTNodeList* params;

	Type* return_type;

	ASTNode* block_node;
	
	int only_declaration;
	
	int is_static;
	int is_constructor;
	int is_virtual;
	int is_override;
	int is_prototype;

	int is_export;

	VisibilityType visibility;
}
ASTNodeFunc;

typedef struct
{
	ASTNode* value;
}
ASTNodeArgument;

typedef struct
{
	Type* argument_type;
	char* identifier;
}
ASTNodeParam;

typedef struct
{
	ASTNode* return_value;
}
ASTNodeRet;

typedef struct
{
	Type* literal_type;

	union
	{
		char char_value;
		int int_value;
		int bool_value;
		char* string_value;
		float float_value;
		double double_value;
	};
}
ASTNodeLiteral;

typedef struct
{
	char* super_identifer;
	char* identifer;

	ASTNode** fields;
	ASTNode** funcs;

	int fields_count;
	int funcs_count;

	int is_export;

	ASTNode* constructor;
}
ASTNodeClass;

typedef struct
{
	char* identifier;
}
ASTNodeIdent;

typedef struct
{
	ASTNode* callee;
	ASTNodeList* arguments;

	int is_prototype;
	int prototype_id;

	int is_built_in;
	int built_in_id;
}
ASTNodeCall;

typedef struct
{
	ASTNode* left;
	ASTNode* expr;
}
ASTNodeAssign;

typedef struct
{
	ASTNode* init;
	ASTNode* condition;
	ASTNode* then_statement;
	ASTNode* then_block;

	SymbolTable* then_scope;
}
ASTNodeForLoop;

typedef struct
{
	SymbolTable* then_scope;

	ASTNode* condition;
	ASTNode* then_block;
}
ASTNodeWhileLoop;

typedef struct
{
	ASTNode* value;
	ASTNodeList* case_list;
}
ASTNodeSwitch;

typedef struct
{
	ASTNode* condition;
	ASTNode* block;

	int new_scope;
	SymbolTable* scope;
}
ASTNodeCase;

typedef struct
{
	char* import_path;
	char* identifier;

	int is_local;
}
ASTNodeImport;

typedef struct
{
	Type* cast_type;
	ASTNode* expr;
}
ASTNodeCast;

typedef struct
{
	ASTNode* expr;
}
ASTNodeReference;

typedef struct
{
	ASTNode* expr;
}
ASTNodeDereference;

typedef struct
{
	ASTNode* object;
	char* member_name;

	int ptr_acess;

	int is_function;
}
ASTNodeMemberAccess;

typedef struct
{
	char* class_name;

	ASTNodeList* constructor_args;
}
ASTNodeCreateInst;

typedef struct
{
	ASTNode* array;
	ASTNode* index_expr;
}
ASTNodeAccessArr;

typedef struct
{
	ASTNodeList* values;

	Type* array_type;
}
ASTNodeLiteralArr;

typedef struct
{
	ASTNodeList* statements;
}
ASTNodeBlock;

//typedef struct
//{
//	ASTNode* array;
//	ASTNode* expr;
//}
//PushArrayNode;
//
//typedef struct
//{
//	Node* array;
//}
//PopArrayNode;
//
//typedef struct
//{
//	Node* array;
//}
//ArrayLengthNode;

typedef enum
{
	NODE_CASE,
	NODE_SWITCH,
	NODE_ASSIGN,
	NODE_CALL,
	NODE_DEREFERENCE,
	NODE_FIELD,
	NODE_IDENT,
	NODE_WHILE_LOOP,
	NODE_REFERENCE,
	NODE_OPERATION,
	NODE_FOR_LOOP,
	NODE_FUNC,
	NODE_ARGUMENT,
	NODE_PARAM,
	NODE_CONTINUE,
	NODE_LITERAL,
	NODE_RET,
	NODE_IMPORT,
	NODE_CLASS,
	NODE_BLOCK,
	NODE_BREAK,
	NODE_CAST,
	NODE_IF,

	NODE_MEMBER_ACCESS,
	NODE_CREATE_INST,
	NODE_THIS,
	NODE_SUPER,
	NODE_STATIC_ACCESS,
	NODE_ARR_ACCESS,
	NODE_ARR_LITERAL,

	//NODE_ARRAY_POP,
	//NODE_ARRAY_LENGTH,
	//NODE_ARRAY_PUSH,
}
ASTNodeType;

typedef struct
{
	Symbol* class_symbol;
}
ASTNodeStaticAccess;

struct ASTNode 
{
	ASTNodeType type;
	ASTNode* next;

	union
	{
		ASTNodeOperation operation;
		ASTNodeIf if_statement;
		ASTNodeLiteral literal;
		ASTNodeField declare;
		ASTNodeFunc function;
		ASTNodeArgument argument;
		ASTNodeIdent variable;
		ASTNodeRet return_statement;
		ASTNodeBlock block;
		ASTNodeCall function_call;
		ASTNodeAssign variable_assign;
		ASTNodeParam param;
		ASTNodeWhileLoop while_loop;
		ASTNodeForLoop for_loop;
		ASTNodeSwitch switch_statement;
		ASTNodeCase switch_case_block;
		ASTNodeImport import_node;
		ASTNodeCast cast_node;
		ASTNodeClass class_node;
		ASTNodeReference adress_of;
		ASTNodeDereference dereference;
		ASTNodeMemberAccess member_access;
		ASTNodeCreateInst create_instance;
		ASTNodeAccessArr acess_array;
		ASTNodeLiteralArr array_literal;
		ASTNodeStaticAccess static_member_access;
		//PushArrayNode array_push;
		//PopArrayNode array_pop;
		//ArrayLengthNode array_length;
	};
};

struct ASTNodeList
{
	ASTNode* head;
};

#endif