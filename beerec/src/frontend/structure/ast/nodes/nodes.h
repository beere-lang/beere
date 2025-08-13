#ifndef NODES_H
#define NODES_H

#include "../tokens/tokens.h"
#include "../types/types.h"

typedef struct Node Node;
typedef struct NodeList NodeList;
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
	
	Node* condition_top;
	Node* then_branch;
	Node* else_branch;
} IfNode;

typedef struct
{
	Node* left;
	Node* right;
	TokenType op;
}
OperationNode;

typedef struct
{
	char* identifier;
	Type* var_type;
	
	Node* default_value;

	int is_const;
	int is_static;

	int is_export;

	VisibilityType visibility;
}
DeclareNode;

typedef struct
{
	char* identifier;

	NodeList* params;

	Type* return_type;

	Node* block_node;
	
	int only_declaration;
	
	int is_static;
	int is_constructor;
	int is_virtual;
	int is_override;
	int is_prototype;

	int is_export;

	VisibilityType visibility;
}
FunctionNode;

typedef struct
{
	Node* value;
}
ArgumentNode;

typedef struct
{
	Type* argument_type;
	char* identifier;
}
ParamNode;

typedef struct
{
	Node* return_value;
}
ReturnNode;

typedef struct
{
	Type* literal_type;

	union
	{
		char char_value;
		int int_value;
		char* string_value;
		int bool_value;
		float float_value;
		double double_value;
		// doesnt need a null value
	};
} 
LiteralNode;

typedef struct
{
	char* super_identifer;
	char* identifer;

	Node** var_declare_list;
	Node** func_declare_list;

	int var_count;
	int func_count;

	int is_export;

	Node* constructor;
}
ClassNode;

typedef struct
{
	char* identifier;
}
VariableNode;

typedef struct
{
	Node* callee;
	NodeList* arguments;

	int is_prototype;
	int prototype_id;

	int is_built_in;
	int built_in_id;
}
FunctionCallNode;

typedef struct
{
	Node* left;
	Node* assign_value;
}
VariableAssignNode;

typedef struct
{
	Node* init;
	Node* condition;
	Node* then_statement;
	Node* then_block;

	SymbolTable* then_scope;
}
ForLoopNode;

typedef struct
{
	SymbolTable* then_scope;

	Node* condition;
	Node* then_block;

	int loop_id;
}
WhileLoopNode;

typedef struct
{
	Node* value;
	NodeList* case_list;
}
SwitchCaseStatement;

typedef struct
{
	Node* condition;
	Node* block;

	int new_scope;
	SymbolTable* scope;
}
SwitchCaseBlock;

typedef struct
{
	char* import_path;
	char* identifier;

	int is_local;
}
ImportStatement;

typedef struct
{
	Type* cast_type;
	Node* expression;
}
CastNode;

typedef struct
{
	Node* expression;
}
AdressOfNode;

typedef struct
{
	Node* ptr;
}
DereferenceNode;

typedef struct
{
	Node* object;
	char* member_name;

	int ptr_acess;

	int is_function;
}
MemberAccessNode;

typedef struct
{
	char* class_name;

	NodeList* constructor_args;
}
CreateInstanceNode;

typedef struct
{
	Node* array;
	Node* index_expr;
}
AccessArrayNode;

typedef struct
{
	NodeList* values;

	Type* array_type;
}
ArrayLiteralNode;

typedef struct
{
	NodeList* statements;
}
BlockNode;

typedef struct
{
	Node* array;
	Node* expr;
}
PushArrayNode;

typedef struct
{
	Node* array;
}
PopArrayNode;

typedef struct
{
	Node* array;
}
ArrayLengthNode;

typedef enum
{
	NODE_SWITCH_CASE_BLOCK, // 0
	NODE_SWITCH_STATEMENT, // 1
	NODE_VARIABLE_ASSIGN, // 2
	NODE_FUNCTION_CALL, // 3
	NODE_DEREFERENCE, // 4
	NODE_DECLARATION, // 5
	NODE_IDENTIFIER, // 6
	NODE_WHILE_LOOP, // 7
	NODE_ADRESS_OF, // 8
	NODE_OPERATION, // 9
	NODE_FOR_LOOP, // 10
	NODE_FUNCTION, // 11
	NODE_ARGUMENT, // 12
	NODE_PARAMETER, // 13
	NODE_CONTINUE, // 14
	NODE_LITERAL, // 15
	NODE_NUMBER, // 16
	NODE_RETURN, // 17
	NODE_IMPORT, // 18
	NODE_CLASS, // 19
	NODE_BLOCK, // 20
	NODE_BREAK, // 21
	NODE_CAST, // 22
	NODE_IF, // 23

	NODE_MEMBER_ACCESS, // 24
	NODE_CREATE_INSTANCE, // 25
	NODE_THIS, // 26
	NODE_ARRAY_ACCESS, // 27
	NODE_ARRAY_LITERAL, // 28
	NODE_ARRAY_PUSH, // 29
	NODE_ARRAY_POP, // 30
	NODE_ARRAY_LENGTH, // 31
	NODE_SUPER, // 32
	NODE_DIRECT_CLASS // 33
}
NodeType;

typedef struct
{
	Symbol* class_symbol;
}
DirectClassAccessNode;

struct Node 
{
	NodeType type;
	Node* next;

	union
	{
		OperationNode operation;
		IfNode if_statement;
		LiteralNode literal;
		DeclareNode declare;
		FunctionNode function;
		ArgumentNode argument;
		VariableNode variable;
		ReturnNode return_statement;
		BlockNode block;
		FunctionCallNode function_call;
		VariableAssignNode variable_assign;
		ParamNode param;
		WhileLoopNode while_loop;
		ForLoopNode for_loop;
		SwitchCaseStatement switch_statement;
		SwitchCaseBlock switch_case_block;
		ImportStatement import_node;
		CastNode cast_node;
		ClassNode class_node;
		AdressOfNode adress_of;
		DereferenceNode dereference;
		MemberAccessNode member_access;
		CreateInstanceNode create_instance;
		AccessArrayNode acess_array;
		ArrayLiteralNode array_literal;
		PushArrayNode array_push;
		PopArrayNode array_pop;
		ArrayLengthNode array_length;
		DirectClassAccessNode direct_class_access;
	};
};

struct NodeList
{
	Node* head;
};

#endif