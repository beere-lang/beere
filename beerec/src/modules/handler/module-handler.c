#include <string.h>

#include "stdlib.h"
#include "module-handler.h"
#include "../../utils/utils.h"

static void advance_token(ModuleParser* parser)
{
	parser->current++;
}

static Token* peek_token(ModuleParser* parser)
{
	return parser->current;
}

static Token* consume_token(ModuleParser* parser)
{
	Token* token = peek_token(parser);

	advance_token(parser);

	return token;
}

static void skip_end_lines(ModuleParser* parser)
{
	while (peek_token(parser)->token_type == TOKEN_END_LINE)
	{
		advance_token(parser);
	}
}

/**
 * Não é uma função segura, mas não tem como dar algum erro em uso normal.
 */
static char* get_token_names(Token* arr, int size)
{
	char* str = malloc(2048);
	str[0] = '\0';

	int offset = 0;

	for (int i = 0; i < size; i++)
	{
		Token current = arr[i];
		char* f = (i < size - 1) ? "%d," : "%d";

		if (offset > 2044)
		{
			printf("[ModuleHandler] [Debug] About to overflow string...\n");
			exit(1);
		}
		
		int written = sprintf(str + offset, f, current.token_type);

		offset += written;
	}

	return str;
}

static void expect_token(ModuleParser* parser, Token* arr, int size)
{
	int i = 0;

	while (i < size)
	{
		if (arr[i].token_type == peek_token(parser)->token_type)
		{
			return;
		}

		i++;
	}

	char* str = get_token_names(arr, size);
	
	printf("[ModuleHandler] [Debug] Unexpected token find: %d, Expected: %s...\n", peek_token(parser)->token_type, str);
	free(str);
	exit(1);
}

static ModuleNode* parse_module_declaration(ModuleParser* parser)
{
	expect_token(parser, (Token[]) { TOKEN_IDENTIFIER }, 1);
	Token* identifier = consume_token(parser);

	expect_token(parser, (Token[]) { TOKEN_OPERATOR_ASSIGN }, 1);
	advance_token(parser);

	expect_token(parser, (Token[]) { TOKEN_LITERAL_STRING }, 1);
	Token* str_value = consume_token(parser);

	ModuleNode* module_node = malloc(sizeof(ModuleNode));

	if (module_node == NULL)
	{
		printf("[ModuleHandler] [Debug] Failed to alloc memory for module node...\n");
		exit(1);
	}

	module_node->type = MODULE_NODE_DECLARATION;

	module_node->module_node_declaration = malloc(sizeof(ModuleNodeDeclaration));

	if (module_node->module_node_declaration == NULL)
	{
		printf("[ModuleHandler] [Debug] Failed to alloc memory for declaration node...\n");
		exit(1);
	}

	module_node->module_node_declaration->identifier = strndup(identifier->start, identifier->length);

	if (str_value->str_value == NULL)
	{
		exit(1);
	}

	module_node->module_node_declaration->value = str_value->str_value;

	skip_end_lines(parser);

	return module_node;
}

static ModuleNode* parse_statement(ModuleParser* parser)
{
	if (peek_token(parser)->token_type == TOKEN_IDENTIFIER)
	{
		return parse_module_declaration(parser);
	}

	printf("[ModuleHandler] [Debug] Unexpected token while parsing statements: %d...\n", peek_token(parser)->token_type);
	exit(1);
}

ModuleNode** parse_statements(ModuleParser* parser)
{
	ModuleNode** node_list = malloc(sizeof(ModuleNode*) * 2048);

	int i = 0;

	while (peek_token(parser)->token_type != TOKEN_END_SRC)
	{
		ModuleNode* node = parse_statement(parser);
		node_list[i] = node;

		i++;
	}

	node_list[i] = NULL;

	return node_list;
}

static void handle_declaration(ModuleHandler* handler, ModuleNode* node)
{
	if (strcmp(node->module_node_declaration->identifier, "path") == 0)
	{
		printf("[ModuleHandler] [Debug] Found a path declaration: \"%s\"...\n", node->module_node_declaration->value);

		char* real_path = get_dot_mod_relative_path(handler->original_path, node->module_node_declaration->value);
		
		handler->root_path = malloc(strlen(real_path) + 1);
		strcpy(handler->root_path, real_path);

		return;
	}

	printf("[ModuleHandler] [Debug] Invalid identifier: \"%s\"...\n", node->module_node_declaration->identifier);
	exit(1);
}

static void handle_node(ModuleHandler* handler, ModuleNode* node)
{
	if (node->type == MODULE_NODE_DECLARATION)
	{
		handle_declaration(handler, node);

		return;
	}

	exit(1);
}

void handle_nodes(ModuleHandler* handler, ModuleNode** node_list)
{
	ModuleNode* node = node_list[0];
	
	int i = 0;
	
	while (node != NULL)
	{
		handle_node(handler, node);

		i++;
		node = node_list[i];
	}
}