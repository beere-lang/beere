#ifndef MODULE_HANDLER_H
#define MODULE_HANDLER_H

#include "../../structure/ast/tokens/tokens.h"

typedef struct ModuleHandler ModuleHandler;
typedef struct ModuleParser ModuleParser;
typedef struct ModuleNode ModuleNode;

typedef enum
{
	MODULE_NODE_DECLARATION,
}
ModuleNodeType;

typedef struct
{
	char* identifier;
	char* value;
}
ModuleNodeDeclaration;

struct ModuleNode
{
	ModuleNodeType type;
	
	union
	{
		ModuleNodeDeclaration* module_node_declaration;
	};
};

struct ModuleParser
{
	Token* tokens;
	Token* current;
};

struct ModuleHandler
{
	char* original_path;
	char* root_path;
};

void handle_nodes(ModuleHandler* handler, ModuleNode** node_list);
ModuleNode** parse_statements(ModuleParser* parser);

#endif