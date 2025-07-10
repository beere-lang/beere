#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/analyzer/analyzer.h"
#include "../src/modules/modules.h"
#include "../src/parser/parser.h"
#include "../src/lexer/lexer.h"
#include "../src/utils/utils.h"

void free_nodes(Node** node_list)
{
	Node* node;

	int k = 0;

	while (1)
	{
		node = node_list[k];

		if (node == NULL)
		{
			break;
		}

		free_node(node);
		node = NULL;

		k++;
	}

	free(node_list);
	node_list = NULL;
}

void analyze_nodes(Module* module, Node** node_list)
{
	analyzer_init(module, node_list);

	Node* node;

	int l = 0;

	while (1)
	{
		node = node_list[l];

		if (node == NULL)
		{
			break;
		}

		analyzer_global_analyze(module, node);
		
		l++;
	}

	printf("\n[Analyzer] [Debug] Module has a valid semantic: \"%s\"...\n", module->module_path);
}

Node** parse_tokens(Token* tokens)
{
	Parser parser;

	parser.current = tokens;
	parser.tokens = tokens;

	parser.inside_class = 0;

	Node** node_list = malloc(sizeof(Node) * 200);

	int j = 0;

	while (parser.current->token_type != TOKEN_END_SRC)
	{
		Node* node = parse_stmt(&parser);

		if (node == NULL)
		{
			exit(1);
		}

		node_list[j] = node;
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

		if (type == TOKEN_END_SRC) {
			break;
		}

		i++;
	}

	return tokens;
}

void free_all(Node** node_list, Token* tokens, char* content)
{
	free_nodes(node_list);

	free(tokens);
	tokens = NULL;

	free(content);
	content = NULL;
}

Module* compile(char* file_path, char* lexer_flag)
{
	if (file_path == NULL)
	{
		printf("[Compiler] [Debug] File Path not found...\n");
		exit(1);
	}

	char* content = read_file(file_path);

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

	Token* tokens = tokenize_code(content, lexer_debug);

	Node** node_list = parse_tokens(tokens);

	analyze_nodes(module, node_list);

	return module;
}