#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../frontend/semantic/analyzer/analyzer.h"
#include "../frontend/modules/modules.h"
#include "../frontend/structure/parser/parser.h"
#include "../frontend/lexer/lexer.h"
#include "../utils/utils.h"

void free_nodes(ASTNode** node_list)
{
	ASTNode* ASTNode;

	int k = 0;

	while (1)
	{
		ASTNode = node_list[k];

		if (ASTNode == NULL)
		{
			break;
		}

		free_node(ASTNode);
		ASTNode = NULL;

		k++;
	}

	free(node_list);
	node_list = NULL;
}

void analyze_nodes(Module* module, ASTNode** node_list)
{
	analyzer_init(module, node_list);

	ASTNode* ASTNode;

	int l = 0;

	while (1)
	{
		ASTNode = node_list[l];

		if (ASTNode == NULL)
		{
			break;
		}

		analyzer_global_analyze(module, ASTNode);
		
		l++;
	}

	printf("\n[Analyzer] [Debug] Module has a valid semantic: \"%s\"...\n", module->module_path);
}

ASTNode** parse_tokens(Token* tokens)
{
	Parser parser;

	parser.current = tokens;
	parser.tokens = tokens;

	parser.inside_class = 0;

	ASTNode** node_list = malloc(sizeof(ASTNode) * 200);

	int j = 0;

	while (parser.current->token_type != TOKEN_END_SRC)
	{
		ASTNode* ASTNode = parse_stmt(&parser);

		if (ASTNode == NULL)
		{
			exit(1);
		}

		node_list[j] = ASTNode;
		j++;
	}

	node_list[j] = NULL;

	return node_list;
}

Token* tokenize_code(char* content, int lexer_debug)
{
	Lexer lexer;
	lexer.start = content;
	lexer.current = content;
	lexer.line = 1;

	Token* tokens = malloc(sizeof(Token) * MAX_TOKEN_LENGTH);

	int i = 0;

	while (1)
	{
		Token token = read_next_tkn(&lexer);

		if (token.token_type == TOKEN_KEYWORD_ONE_LINE_COMMENT || token.token_type == TOKEN_KEYWORD_MULTI_LINE_COMMENT)
		{
			continue;
		}

		tokens[i] = token;
		TokenType type = tokens[i].token_type;

		if (type == TOKEN_IDENTIFIER)
		{
			type = get_by_keyword_type(&lexer, lexer.current, (size_t)(lexer.current - lexer.start));
		}

		if (lexer_debug)
		{
			printf("[Lexer] [Debug] Token Type: %s\n", token_type_to_string(type));
		}

		if (type == TOKEN_END_SRC) 
		{
			break;
		}

		i++;
	}

	return tokens;
}

static void free_tokens(Token* token_list)
{
	if (token_list == NULL)
	{
		return;
	}

	free(token_list);
	token_list = NULL;
}

static void free_content(char* content)
{
	if (content == NULL)
	{
		return;
	}

	free(content);
	content = NULL;
}

static void generate_assembly(ASTNode** nodes, Module* module)
{
	// em breve
}

Module* compile(ModuleHandler* handler, char* file_path, char* lexer_flag)
{
	if (file_path == NULL)
	{
		printf("[Compiler] [Debug] File Path not found...\n");
		exit(1);
	}

	char* content = read_file(file_path, 0);

	int lexer_debug = 0;

	if (lexer_flag != NULL)
	{
		if (strcmp(lexer_flag, "-l") == 0)
		{
			lexer_debug = 1;
		}
	}

	if (content == NULL)
	{
		printf("[Compiler] [Debug] Failed to read input file: %s...\n", file_path);
		exit(1);
	}

	Module* module = setup_module(file_path, NULL);
	module->handler = handler;

	Token* tokens = tokenize_code(content, lexer_debug);

	ASTNode** node_list = parse_tokens(tokens);

	free_tokens(tokens);
	free_content(content);

	analyze_nodes(module, node_list);

	generate_assembly(node_list, module);

	free_nodes(node_list);

	return module;
}