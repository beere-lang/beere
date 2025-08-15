#ifndef PARSER_H
#define PARSER_H

#include "../ast/tokens/tokens.h"
#include "../ast/types/types.h"
#include "../ast/nodes/nodes.h"

#define ALLOC_SIZE 8

typedef struct 
{
	Token* tokens;
	Token* current;

	int inside_class;
} 
Parser;

typedef struct
{
	char** classes;
	
	int capacity;
	int length;
}
ClassTable;

Type* create_type(VarType type_enum, char* class_name);
ASTNode* parse_stmt(Parser* parser);
void free_node(ASTNode* node);

#endif
