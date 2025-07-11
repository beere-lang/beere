#ifndef MODULE_HANDLER_H
#define MODULE_HANDLER_H

#include "../../ast/tokens/tokens.h"

typedef struct ModuleHandler ModuleHandler;
typedef struct ModuleNode ModuleNode;
typedef struct ModuleParser ModuleParser;

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
	char* root_path;
};

void handle_nodes(ModuleHandler* handler, ModuleNode** node_list);
ModuleNode** parse_statements(ModuleParser* parser);

#endif