#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#include "parser.h"
#include "../lexer/lexer.h"
#include "../utils/logger/logger.h"
#include "../utils/utils.h"

static Node* parse_array_access(Parser* parser, Node* array_access);
static Node* parse_func_call(Parser* parser, Node* callee);
static Node* parse_expression(Parser* parser);
static Node* parse_comparator(Parser* parser);
static NodeList* parse_params(Parser* parser);
static NodeList* parse_args(Parser* parser);
static Node* parse_primary(Parser* parser);
static Node* parse_unary(Parser* parser);
static Node* parse_block(Parser* parser);
static Node* parse_and(Parser* parser);
static Node* parse_or(Parser* parser);

static Token* peek_tkn(const Parser* parser)
{
	return parser->current;
}

static void advance_tkn(Parser* parser)
{
	parser->current++;
}

static Token* peek_ahead(Parser* parser)
{
	return parser->current + 1;
}

static void expect_tkn(const Parser* parser, const TokenType* expected_type, const size_t size)
{
	const TokenType current_type = peek_tkn(parser)->token_type;

	for (size_t i = 0; i < size; i++)
	{
		if (current_type == expected_type[i])
		{
			return;
		}
	}

	const char* error_parts[] = { "Unexpected token found. Token: \"", token_type_to_string(current_type), "\" Expected Token: " };

	printf("[Parser] [Error] %s%s%s", error_parts[0], error_parts[1], error_parts[2]);

	for (size_t i = 0; i < size; i++)
	{
		printf("\"%s\", ", token_type_to_string(expected_type[i]));
	}

	printf("%.*s", (int) peek_tkn(parser)->length, peek_tkn(parser)->start);

	exit(1);
}

static Token* consume_token(Parser* parser)
{
	Token* token = peek_tkn(parser);
	advance_tkn(parser);
	return token;
}

static int convert_number_int(const char* start)
{
	char* end_ptr;
	errno = 0;
	const long val = strtol(start, &end_ptr, 10);

	if (end_ptr == start || errno == ERANGE || val > INT_MAX || val < INT_MIN)
	{
		return INT_MIN;
	}

	return (int) val;
}

static float convert_number_float(const char* start)
{
	char* end_ptr;
	errno = 0;
	const float val = strtof(start, &end_ptr);

	if (end_ptr == start || errno == ERANGE || val > INT_MAX || val < INT_MIN)
	{
		return NAN;
	}

	return val;
}

static double convert_number_double(const char* nbr)
{
	errno = 0;
	char* end_ptr;
	double val = strtod(nbr, &end_ptr);

	if (nbr == end_ptr || errno == ERANGE) 
	{
		parser_error("Malformed double number literal found...");
		exit(1);
	}

	return val;
}

static int convert_bool(const char* str)
{
	int val = INT_MIN;

	if (strcmp(str, "true") == 0) 
	{
		val = 1;
	}
	if (strcmp(str, "false") == 0) 
	{
		val = 0;
	}

	return val;
}

static char convert_char(const char* str)
{
	return str[0]; // :skull: top 10 codes
}

static void skip_end_lines(Parser* parser) 
{
	while (peek_tkn(parser)->token_type == TOKEN_END_LINE)
	{
		advance_tkn(parser);
	}
}

Type* create_type(VarType type_enum, char* class_name)
{
	Type* type = malloc(sizeof(Type));

	if (type == NULL)
	{
		printf("[Parser] [Debug] Failed to allocate memory for type struct...");
		exit(1);
	}

	type->base = NULL;
	type->type = type_enum;
	type->class_name = class_name;

	return type;
}

static Type* chain_type(Type* type, VarType type_enum, char* class_name)
{
	type->base = create_type(type_enum, class_name);

	return type->base->base;
}

void print_tree(Type* head)
{
	Type* next = head;
	int count = 0;

	while (next != NULL)
	{
		if (count > 0)
		{
			printf(" --> ");
		}
		printf("%d", next->type);

		next = next->base;
		count++;
	}

	printf("\n");
}

static void handle_ptr_array(Parser* parser, Type** result_ref)
{
	if (peek_tkn(parser)->token_type == TOKEN_CHAR_STAR)
	{
		Type* ptr = create_type(TYPE_PTR, NULL);
		
		ptr->base = *result_ref;
		*result_ref = ptr;
	}
	else if (peek_tkn(parser)->token_type == TOKEN_CHAR_OPEN_BRACKET)
	{
		advance_tkn(parser);
		expect_tkn(parser, (TokenType[]) { TOKEN_CHAR_CLOSE_BRACKET }, 1);
		
		Type* arr = create_type(TYPE_ARRAY, NULL);

		arr->base = *result_ref;
		*result_ref = arr;
	}

	advance_tkn(parser);
}

static Type* handle_type_declaration(Parser* parser) {
	advance_tkn(parser);
	expect_tkn(parser, (TokenType[]) { TOKEN_KEYWORD_TYPE, TOKEN_KEYWORD_TYPE_VOID, TOKEN_IDENTIFIER }, 3);

	Type* result = NULL;
	
	if (peek_tkn(parser)->token_type == TOKEN_IDENTIFIER)
	{
		Token* name = consume_token(parser);
		result = create_type(TYPE_CLASS, strndup(name->start, name->length));
	}
	else
	{
		Token* type = consume_token(parser);
		result = create_type(type->var_type, NULL);
	}
   
	while (peek_tkn(parser)->token_type == TOKEN_CHAR_STAR || peek_tkn(parser)->token_type == TOKEN_CHAR_OPEN_BRACKET) 
	{
		handle_ptr_array(parser, &result);
	}

	return result;
}

static Node* parse_literal(Parser* parser)
{
	Node* expr = NULL;

	const TokenType tkn_type = peek_tkn(parser)->token_type;
	const Token* tkn = peek_tkn(parser);
	
	if (tkn_type == TOKEN_LITERAL_INT || 
		tkn_type == TOKEN_LITERAL_FLOAT || 
		tkn_type == TOKEN_LITERAL_BOOL || 
		tkn_type == TOKEN_LITERAL_CHAR ||
		tkn_type == TOKEN_LITERAL_STRING ||
		tkn_type == TOKEN_LITERAL_NULL ||
		tkn_type == TOKEN_LITERAL_DOUBLE
	)
	{
		Node* literal_node = malloc(sizeof(Node));

		if (literal_node == NULL) 
		{ 
			parser_error("Failed to allocate memory..."); exit(1); 
		}

		literal_node->type = NODE_LITERAL;

		switch (tkn_type) 
		{
			case TOKEN_LITERAL_INT: 
			{
				Type* type = create_type(TYPE_INT, NULL);
				literal_node->literal_node.literal.literal_type = type;
				literal_node->literal_node.literal.int_value = convert_number_int(peek_tkn(parser)->start);
				
				break;
			}

			case TOKEN_LITERAL_STRING: 
			{
				Type* type = create_type(TYPE_STRING, NULL);
				literal_node->literal_node.literal.literal_type = type;
				literal_node->literal_node.literal.string_value = strdup(peek_tkn(parser)->str_value);
				
				break;
			}

			case TOKEN_LITERAL_FLOAT: 
			{
				Type* type = create_type(TYPE_FLOAT, NULL);
				literal_node->literal_node.literal.literal_type = type;
				literal_node->literal_node.literal.float_value = convert_number_float(peek_tkn(parser)->start);
				
				break;
			}

			case TOKEN_LITERAL_BOOL: 
			{
				Type* type = create_type(TYPE_BOOL, NULL);

				literal_node->literal_node.literal.literal_type = type;
				literal_node->literal_node.literal.bool_value = peek_tkn(parser)->bool_value;
				
				break;
			}

			case TOKEN_LITERAL_CHAR: 
			{
				Type* type = create_type(TYPE_CHAR, NULL);

				literal_node->literal_node.literal.literal_type = type;
				literal_node->literal_node.literal.char_value = tkn->ch_value;
				
				break;
			}

			case TOKEN_LITERAL_DOUBLE: 
			{
				Type* type = create_type(TYPE_DOUBLE, NULL);

				literal_node->literal_node.literal.literal_type = type;
				literal_node->literal_node.literal.double_value = convert_number_double(peek_tkn(parser)->start);
				
				break;
			}

			case TOKEN_LITERAL_NULL: 
			{
				Type* type = create_type(TYPE_NULL, NULL);
				literal_node->literal_node.literal.literal_type = type;
				
				break;
			}

			default:
			{
				parser_error("Invalid type found...");
				exit(1);
			}
		}

		advance_tkn(parser);
		expr = literal_node;
	}
	else if (tkn_type == TOKEN_IDENTIFIER) 
	{
		Token* token_identifier = peek_tkn(parser);

		char* identifier = strndup(token_identifier->start, token_identifier->length);
		advance_tkn(parser);

		if (peek_tkn(parser)->token_type == TOKEN_CHAR_OPEN_PAREN) 
		{
			Node* callee = malloc(sizeof(Node));

			if (callee == NULL) 
			{ 
				parser_error("Failed to allocate memory for callee..."); 
				exit(1); 
			}

			callee->type = NODE_IDENTIFIER;
			callee->variable_node.variable.identifier = identifier;

			parser_info("Found a function in primary expression part...");

			advance_tkn(parser);

			expr = parse_func_call(parser, callee);
		}
		else if (peek_tkn(parser)->token_type == TOKEN_CHAR_OPEN_BRACKET) 
		{
			Node* callee = malloc(sizeof(Node));

			if (callee == NULL) 
			{ 
				parser_error("Failed to allocate memory for callee..."); 
				exit(1); 
			}

			callee->type = NODE_IDENTIFIER;
			callee->variable_node.variable.identifier = identifier;

			parser_info("Found a array access in primary expression part...");

			expr = parse_array_access(parser, callee);
		}
		else 
		{
			Node* var_node = malloc(sizeof(Node));

			if (var_node == NULL) 
			{ 
				parser_error("Failed to allocate memory for variable node..."); 
				exit(1); 
			}

			var_node->type = NODE_IDENTIFIER;
			var_node->variable_node.variable.identifier = identifier;

			expr = var_node;
		}
	}
	else if (tkn_type == TOKEN_KEYWORD_THIS)
	{
		advance_tkn(parser);

		Node* this_node = malloc(sizeof(Node));

		if (this_node == NULL) 
		{ 
			parser_error("Failed to allocate memory for this node..."); 
			exit(1); 
		}

		this_node->type = NODE_THIS;

		expr = this_node;
	}
	else if (tkn_type == TOKEN_CHAR_OPEN_PAREN) 
	{
		advance_tkn(parser);

		expr = parse_expression(parser);
		
		if (peek_tkn(parser)->token_type != TOKEN_CHAR_CLOSE_PAREN) 
		{
			parser_error("Invalid syntax expected: ')'");
			exit(1);
		}

		advance_tkn(parser);
	}
	else 
	{
		printf("[Parser] [Debug] Invalid syntax in expression, exiting: %s, %s...", token_type_to_string(peek_tkn(parser)->token_type), token_type_to_string(peek_ahead(parser)->token_type));
		exit(1);
	}

	return expr;
}

static Node* parse_primary(Parser* parser)
{
	Node* expr = parse_literal(parser);
   
	while (1)
	{
		if (peek_tkn(parser)->token_type == TOKEN_OPERATOR_DOT || peek_tkn(parser)->token_type == TOKEN_OPERATOR_ACCESS_PTR)
		{
			int ptr_acess = peek_tkn(parser)->token_type == TOKEN_OPERATOR_ACCESS_PTR;

			advance_tkn(parser);

			expect_tkn(parser, (TokenType[]) { TOKEN_IDENTIFIER }, 1);
			Token* identifier = consume_token(parser);

			char* _identifier = strndup(identifier->start, identifier->length);

			Node* object = expr;

			Node* node = malloc(sizeof(Node));

			if (node == NULL) 
			{ 
				parser_error("Failed to allocate memory for member access node..."); 
				exit(1); 
			}

			node->type = NODE_MEMBER_ACCESS;

			node->member_access_node.member_access.object = object;
			node->member_access_node.member_access.member_name = _identifier;
			node->member_access_node.member_access.ptr_acess = ptr_acess;
			node->member_access_node.member_access.is_function = 0;

			expr = node;
		}
		/**
		 * Detecta o '(' e detecta chamada de função.
		 */
		else if (peek_tkn(parser)->token_type == TOKEN_CHAR_OPEN_PAREN)
		{
			printf("[Parser] [Debug] Calling a function from a object...\n");
			
			advance_tkn(parser);

			if (expr->type == NODE_MEMBER_ACCESS)
			{
				expr->member_access_node.member_access.is_function = 1;
			}

			Node* func_call = parse_func_call(parser, expr);
			expr = func_call;
		}
		/**
		 * Detecta o '[' e detecta acesso de array.
		 */
		else if (peek_tkn(parser)->token_type == TOKEN_CHAR_OPEN_BRACKET)
		{
			printf("[Parser] [Debug] Accessing a array...\n");

			Node* array_access = parse_array_access(parser, expr);
			expr = array_access;
		}
		else
		{
			break;
		}
	}

	return expr;
}

static Node* parse_arithmetic(Parser* parser)
{
	Node* left = parse_unary(parser);

	TokenType tkn_type = peek_tkn(parser)->token_type;

	/**
	 * +, -, *, /, +=, -=, *=, /=
	 */
	while (
		tkn_type == TOKEN_OPERATOR_PLUS ||
		tkn_type == TOKEN_OPERATOR_MINUS ||
		tkn_type == TOKEN_CHAR_STAR ||
		tkn_type == TOKEN_OPERATOR_DIVIDED ||
		tkn_type == TOKEN_OPERATOR_PLUS_EQUALS ||
		tkn_type == TOKEN_OPERATOR_MINUS_EQUALS ||
		tkn_type == TOKEN_OPERATOR_TIMES_EQUALS ||
		tkn_type == TOKEN_OPERATOR_DIVIDED_EQUALS
	)
	{
		const TokenType operator = peek_tkn(parser)->token_type;

		advance_tkn(parser);
		
		Node* right = parse_unary(parser);
		
		Node* node = malloc(sizeof(Node));

		if (node == NULL)
		{
			parser_error("Failed to allocate memory...");
			exit(1);
		}

		node->type = NODE_OPERATION;
		node->operation_node.operation.left = left;
		node->operation_node.operation.right = right;
		node->operation_node.operation.op = operator;

		left = node;
		tkn_type = peek_tkn(parser)->token_type;
	}
	
	return left;
}

static Node* parse_comparator(Parser* parser)
{
	Node* left = parse_arithmetic(parser);

	TokenType tkn_type = peek_tkn(parser)->token_type;

	/**
	 * ==, >, >=, <, <=, !=
	 */
	while (
		tkn_type == TOKEN_OPERATOR_EQUALS ||
		tkn_type == TOKEN_OPERATOR_GREATER ||
		tkn_type == TOKEN_OPERATOR_GREATER_EQUALS ||
		tkn_type == TOKEN_OPERATOR_LESS ||
		tkn_type == TOKEN_OPERATOR_LESS_EQUALS ||
		tkn_type == TOKEN_OPERATOR_NOT_EQUALS
	)
	{
		const TokenType operator = peek_tkn(parser)->token_type;

		advance_tkn(parser);
		
		Node* right = parse_arithmetic(parser);
		
		Node* node = malloc(sizeof(Node));

		if (node == NULL)
		{
			parser_error("Failed to allocate memory...");
			exit(1);
		}

		node->type = NODE_OPERATION;
		node->operation_node.operation.left = left;
		node->operation_node.operation.right = right;
		node->operation_node.operation.op = operator;

		left = node;
		tkn_type = peek_tkn(parser)->token_type;
	}
	
	return left;
}

static Node* parse_and(Parser* parser)
{
	Node* left = parse_comparator(parser);

	while (peek_tkn(parser)->token_type == TOKEN_OPERATOR_AND) 
	{
		advance_tkn(parser);

		Node* right = parse_comparator(parser);

		Node* node = malloc(sizeof(Node));

		if (node == NULL) 
		{
			parser_error("Failed to allocate memory...");
			exit(1);
		}

		node->type = NODE_OPERATION;
		node->operation_node.operation.left = left;
		node->operation_node.operation.right = right;
		node->operation_node.operation.op = TOKEN_OPERATOR_AND;

		left = node;
	}

	return left;
}

static Node* parse_or(Parser* parser)
{
	Node* left = parse_and(parser);

	while (peek_tkn(parser)->token_type == TOKEN_OPERATOR_OR)
	{
		advance_tkn(parser);

		Node* right = parse_and(parser);

		Node* node = malloc(sizeof(Node));

		if (node == NULL) 
		{
			parser_error("Failed to allocate memory...");
			exit(1);
		}

		node->type = NODE_OPERATION;
		node->operation_node.operation.left = left;
		node->operation_node.operation.right = right;
		node->operation_node.operation.op = TOKEN_OPERATOR_OR;

		left = node;
	}

	return left;
}

static Node* parse_expression(Parser* parser)
{
	return parse_or(parser);
}

static Node* parse_block(Parser* parser)
{
	expect_tkn(parser, (TokenType[]){ TOKEN_CHAR_OPEN_BRACE }, 1);
	advance_tkn(parser);

	skip_end_lines(parser);

	NodeList* node_list = malloc(sizeof(NodeList));

	if (node_list == NULL) 
	{ 
		parser_error("Failed to allocate memory for node list..."); 
		exit(1); 
	}

	Node** current = &node_list->head;

	while (peek_tkn(parser)->token_type != TOKEN_CHAR_CLOSE_BRACE) 
	{
		Node* statement = parse_stmt(parser);
	
		if (statement == NULL) 
		{
			parser_error("Failed to parse statement...");
			exit(1);
		}

		parser_info("Parsed successfully...");
		
		*current = statement;
		current = &statement->next;
		
		skip_end_lines(parser);
	}

	*current = NULL;

	Node* node = malloc(sizeof(Node));

	if (node == NULL) 
	{
		parser_error("Failed to allocate memory for block node");
		exit(1);
	}
  
	node->type = NODE_BLOCK;
	node->block_node.block.statements = node_list;

	return node;
}

static int check_case_scope(Parser* parser, int true)
{
	if (true)
	{
		return peek_tkn(parser)->token_type != TOKEN_CHAR_CLOSE_BRACE;
	}

	return peek_tkn(parser)->token_type != TOKEN_KEYWORD_CASE && peek_tkn(parser)->token_type != TOKEN_CHAR_CLOSE_BRACE;
}

static Node* parse_switch_label_content(Parser* parser, Node* condition, int new_scope)
{
	NodeList* statement_list = malloc(sizeof(NodeList));

	if (statement_list == NULL) 
	{ 
		parser_error("Failed to allocate memory for node list..."); 
		exit(1); 
	}

	Node** current = &statement_list->head;
	
	while (check_case_scope(parser, new_scope)) 
	{
		Node* statement = parse_stmt(parser);

		if (statement == NULL)
		{
			exit(1);
		}

		*current = statement;
		current = &statement->next;

		skip_end_lines(parser);
	}

	*current = NULL;

	if (new_scope)
	{
		advance_tkn(parser);

		skip_end_lines(parser);
	}

	Node* block_node = malloc(sizeof(Node));

	if (block_node == NULL) 
	{ 
		parser_error("Failed to allocate memory for block node..."); 
		exit(1); 
	}

	block_node->type = NODE_BLOCK;

	block_node->block_node.block.statements = statement_list;

	Node* node = malloc(sizeof(Node));

	if (node == NULL) 
	{ 
		parser_error("Failed to allocate memory for switch case block node..."); 
		exit(1); 
	}
	
	node->type = NODE_SWITCH_CASE_BLOCK;
	node->switch_case_block_node.switch_case_block.condition = condition;
	node->switch_case_block_node.switch_case_block.block = block_node;

	return node;
}

static Node* parse_var_assign(Parser* parser, Node* left)
{
	Node* expression = parse_expression(parser);

	Node* node = malloc(sizeof(Node));

	if (node == NULL) 
	{ 
		parser_error("Failed to allocate memory for variable assign node..."); 
		exit(1); 
	}

	node->type = NODE_VARIABLE_ASSIGN;

	node->variable_assign_node.variable_assign.left = left;
	node->variable_assign_node.variable_assign.assign_value = expression;

	skip_end_lines(parser);

	return node;
}

static Node* parse_var_increment(Parser* parser, Node* left)
{
	printf("[Parser] [Debug] Parsing a increment operation...\n");
	
	Node* node = malloc(sizeof(Node));

	if (node == NULL) 
	{ 
		parser_error("Failed to allocate memory for operation node..."); 
		exit(1); 
	}

	Node* node_literal = malloc(sizeof(Node));

	if (node_literal == NULL) 
	{ 
		parser_error("Failed to allocate memory for literal node..."); 
		exit(1); 
	}

	node_literal->type = NODE_LITERAL;

	Type* type = create_type(TYPE_INT, NULL);

	node_literal->literal_node.literal.literal_type = type;
	node_literal->literal_node.literal.int_value = 1;

	node->type = NODE_OPERATION;
	node->operation_node.operation.op = TOKEN_OPERATOR_INCREMENT;
	node->operation_node.operation.left = left;
	node->operation_node.operation.right = node_literal;

	advance_tkn(parser);

	skip_end_lines(parser);

	return node;
}

static Node* parse_var_decrement(Parser* parser, Node* left)
{
	printf("[Parser] [Debug] Parsing a increment operation...\n");
	
	Node* node = malloc(sizeof(Node));

	if (node == NULL) 
	{ 
		parser_error("Failed to allocate memory for operation node..."); 
		exit(1); 
	}

	Node* node_literal = malloc(sizeof(Node));

	if (node_literal == NULL) 
	{ 
		parser_error("Failed to allocate memory for literal node..."); 
		exit(1); 
	}

	node_literal->type = NODE_LITERAL;

	Type* type = create_type(TYPE_INT, NULL);

	node_literal->literal_node.literal.literal_type = type;
	node_literal->literal_node.literal.int_value = 1;

	node->type = NODE_OPERATION;
	node->operation_node.operation.op = TOKEN_OPERATOR_DECREMENT;
	node->operation_node.operation.left = left;
	node->operation_node.operation.right = node_literal;

	advance_tkn(parser);

	skip_end_lines(parser);

	return node;
}

static Node* parse_var_plus_equals(Parser* parser, Node* left) 
{
	advance_tkn(parser);
	
	Node* node = malloc(sizeof(Node));

	if (node == NULL) 
	{ 
		parser_error("Failed to allocate memory for operation node..."); 
		exit(1); 
	}

	node->type = NODE_OPERATION;

	Node* expression = parse_expression(parser);
	
	node->operation_node.operation.op = TOKEN_OPERATOR_PLUS_EQUALS;
	node->operation_node.operation.left = left;
	node->operation_node.operation.right = expression;

	return node;
}

static Node* parse_var_minus_equals(Parser* parser, Node* left) 
{
	advance_tkn(parser);
	
	Node* node = malloc(sizeof(Node));

	if (node == NULL) 
	{ 
		parser_error("Failed to allocate memory for operation node..."); 
		exit(1); 
	}

	node->type = NODE_OPERATION;

	Node* expression = parse_expression(parser);

	node->operation_node.operation.op = TOKEN_OPERATOR_MINUS_EQUALS;
	node->operation_node.operation.left = left;
	node->operation_node.operation.right = expression;

	return node;
}

static Node* parse_var_times_equals(Parser* parser, Node* left) 
{
	advance_tkn(parser);
	
	Node* node = malloc(sizeof(Node));

	if (node == NULL) 
	{ 
		parser_error("Failed to allocate memory for operation node..."); 
		exit(1); 
	}

	node->type = NODE_OPERATION;

	Node* expression = parse_expression(parser);
	
	node->operation_node.operation.op = TOKEN_OPERATOR_TIMES_EQUALS;
	node->operation_node.operation.left = left;
	node->operation_node.operation.right = expression;

	return node;
}

static Node* parse_var_divided_equals(Parser* parser, Node* left) 
{
	advance_tkn(parser);
	
	Node* node = malloc(sizeof(Node));

	if (node == NULL) 
	{ 
		parser_error("Failed to allocate memory for operation node..."); 
		exit(1); 
	}

	node->type = NODE_OPERATION;

	Node* expression = parse_expression(parser);
	
	node->operation_node.operation.op = TOKEN_OPERATOR_DIVIDED_EQUALS;
	node->operation_node.operation.left = left;
	node->operation_node.operation.right = expression;

	return node;
}

static Node* parse_return(Parser* parser)
{
	advance_tkn(parser);

	Node* expression = NULL;

	if (peek_tkn(parser)->token_type != TOKEN_END_LINE && peek_tkn(parser)->token_type != TOKEN_END_SRC)
	{
		expression = parse_expression(parser);
	}
	else 
	{
		printf("[Parser] [Debug] Returning void...\n");
	}

	skip_end_lines(parser);

	Node* node = calloc(1, sizeof(Node));
	node->type = NODE_RETURN;
	node->return_statement_node.return_statement.return_value = expression;

	return node;
}

static Node* parse_if(Parser* parser)
{
	advance_tkn(parser);

	expect_tkn(parser, (TokenType[]){ TOKEN_CHAR_OPEN_PAREN }, 1);
	advance_tkn(parser);

	Node* expression = parse_expression(parser);

	expect_tkn(parser, (TokenType[]){ TOKEN_CHAR_CLOSE_PAREN }, 1);
	advance_tkn(parser);

	skip_end_lines(parser);

	Node* statements = parse_block(parser);

	skip_end_lines(parser);

	expect_tkn(parser, (TokenType[]){ TOKEN_CHAR_CLOSE_BRACE }, 1);
	advance_tkn(parser);

	skip_end_lines(parser);

	Node* node = malloc(sizeof(Node));

	if (node == NULL) 
	{ 
		parser_error("Failed to allocate memory for if node..."); 
		exit(1); 
	}

	node->if_statement_node.if_statement.else_branch = NULL;

	if (peek_tkn(parser)->token_type == TOKEN_KEYWORD_ELSE)
	{
		parser_info("Parsing else...");

		Node* else_statements = malloc(sizeof(Node));

		if (else_statements == NULL) 
		{
			parser_error("Failed to allocate memory for block node..."); 
			exit(1); 
		}

		else_statements->type = NODE_BLOCK;

		advance_tkn(parser);

		skip_end_lines(parser);

		if (peek_tkn(parser)->token_type == TOKEN_KEYWORD_IF) 
		{
			else_statements = parse_if(parser);
		}
		else
		{
			else_statements = parse_block(parser);
			advance_tkn(parser);
		}

		skip_end_lines(parser);

		node->if_statement_node.if_statement.else_branch = else_statements;
	}

	if (node == NULL) 
	{
		parser_error("Failed to allocate memory...");
		exit(1);
	}

	node->type = NODE_IF;

	node->if_statement_node.if_statement.condition_top = expression;
	node->if_statement_node.if_statement.then_branch = statements;

	return node;
}

/**
 * Deve ser chamada quando o parser encontrar um '['
 */
static Node* parse_array_access(Parser* parser, Node* array_access)
{
	advance_tkn(parser);
	
	Node* expression = parse_expression(parser);

	expect_tkn(parser, (TokenType[]) { TOKEN_CHAR_CLOSE_BRACKET }, 1);
	advance_tkn(parser);

	Node* access_node = malloc(sizeof(Node));

	if (access_node == NULL) 
	{ 
		parser_error("Failed to allocate memory for array access node..."); 
		exit(1); 
	}

	access_node->type = NODE_ARRAY_ACCESS;

	access_node->acess_array_node.acess_array.array = array_access;
	access_node->acess_array_node.acess_array.index_expr = expression;

	skip_end_lines(parser);

	return access_node;
}

static Node* parse_func_call(Parser* parser, Node* callee_expr)
{
	NodeList* args = NULL;
   
	if (peek_tkn(parser)->token_type != TOKEN_CHAR_CLOSE_PAREN)
	{
		args = parse_args(parser);
	}

	advance_tkn(parser);

	Node* node = malloc(sizeof(Node));

	if (node == NULL) 
	{
		parser_error("Failed to allocate memory for function call");
		exit(1);
	}

	node->type = NODE_FUNCTION_CALL;

	node->function_call_node.function_call.arguments = args;
	node->function_call_node.function_call.callee = callee_expr;
	node->function_call_node.function_call.is_prototype = 0;

	return node;
}

static Node* parse_func(
	Parser* parser,
	VisibilityType visibility,
	const Token* identifier_token,
	NodeList* parameters,
	Type* type,
	int is_static,
	int is_virtual,
	int is_override,
	int is_constructor
) {
	parser_info("Parsing function declaration...");

	advance_tkn(parser);
	skip_end_lines(parser);

	FunctionNode function_node = { 0 };
	function_node.return_type = type;
	function_node.identifier = strndup(identifier_token->start, identifier_token->length);
	function_node.is_static = is_static;
	function_node.visibility = visibility;
	function_node.is_prototype = 0;
	function_node.params = parameters;
	function_node.is_virtual = is_virtual;
	function_node.is_override = is_override;
	
	Node* statements = NULL;

	if (peek_tkn(parser)->token_type == TOKEN_CHAR_OPEN_BRACE) {
		statements = parse_block(parser);

		if (statements == NULL) {
			return NULL;
		}

		skip_end_lines(parser);
		expect_tkn(parser, (TokenType[]){ TOKEN_CHAR_CLOSE_BRACE }, 1);
		advance_tkn(parser);
	}

	skip_end_lines(parser);

	Node* node = malloc(sizeof(Node));

	if (node == NULL) 
	{
		parser_error("Failed to allocate memory for function node");
		exit(1);
	}

	node->type = NODE_FUNCTION;
	node->function_node.function = function_node;
	node->function_node.function.block_node = statements;
	node->function_node.function.is_constructor = is_constructor;
	node->function_node.function.only_declaration = (statements == NULL);
	
	parser_info(statements ? "Normal function found..." : "Only declaration function found...");

	return node;
}

static Node* parse_var(
	Parser* parser, 
	VisibilityType visibility,
	const Token* identifier_token, 
	Type* type, 
	int is_const,
	int is_static
)
{
	parser_info("Parsing variable declaration");

	DeclareNode declare_node = { 0 };
	
	declare_node.identifier = strndup(identifier_token->start, identifier_token->length);
	declare_node.default_value = NULL;
	declare_node.is_const = is_const;
	declare_node.is_static = is_static;
	declare_node.var_type = type;
	declare_node.visibility = visibility;
	
	if (parser->current->token_type == TOKEN_OPERATOR_ASSIGN)
	{
		advance_tkn(parser);

		Node* expression = parse_expression(parser);

		declare_node.default_value = expression;
	}

	Node* node = malloc(sizeof(Node));

	if (node == NULL) 
	{
		parser_error("Failed to allocate memory for declaration node");
		exit(1);
	}

	node->type = NODE_DECLARATION;
	node->declare_node.declare = declare_node;

	skip_end_lines(parser);

	return node;
}

static Node* parse_while_loop(Parser* parser)
{
	parser_info("Parsing a while loop statement...");
	
	Node* node = malloc(sizeof(Node));
	node->type = NODE_WHILE_LOOP;

	advance_tkn(parser);
	expect_tkn(parser, (const TokenType[]) { TOKEN_CHAR_OPEN_PAREN }, 1);
	advance_tkn(parser);

	Node* condition = parse_expression(parser);
	
	expect_tkn(parser, (const TokenType[]) { TOKEN_CHAR_CLOSE_PAREN }, 1);
	advance_tkn(parser);

	skip_end_lines(parser);

	expect_tkn(parser, (const TokenType[]) { TOKEN_CHAR_OPEN_BRACE }, 1);
	Node* then_block = parse_block(parser);
	then_block->type = NODE_BLOCK;

	advance_tkn(parser);
	skip_end_lines(parser);

	node->while_loop_node.while_loop.condition = condition;
	node->while_loop_node.while_loop.then_block = then_block;

	return node;
}

static Node* parse_for_loop(Parser* parser)
{
	advance_tkn(parser);
	expect_tkn(parser, (TokenType[]) { TOKEN_CHAR_OPEN_PAREN }, 1);
	advance_tkn(parser);

	Node* initializer = parse_stmt(parser);

	expect_tkn(parser, (TokenType[]) { TOKEN_CHAR_SEMI_COLON }, 1);
	advance_tkn(parser);

	Node* condition = parse_expression(parser);

	expect_tkn(parser, (TokenType[]) { TOKEN_CHAR_SEMI_COLON }, 1);
	advance_tkn(parser);

	Node* then_statement = parse_stmt(parser);

	expect_tkn(parser, (TokenType[]) { TOKEN_CHAR_CLOSE_PAREN }, 1);
	advance_tkn(parser);

	skip_end_lines(parser);

	Node* then_block = parse_block(parser);

	advance_tkn(parser);
	skip_end_lines(parser);
	
	Node* node = malloc(sizeof(Node));

	if (node == NULL) 
	{ 
		parser_error("Failed to allocate memory for for loop node..."); 
		exit(1); 
	}

	node->for_loop_node.for_loop.init = initializer;
	node->for_loop_node.for_loop.condition = condition;
	node->for_loop_node.for_loop.then_statement = then_statement;
	
	node->for_loop_node.for_loop.then_block = then_block;
	
	node->type = NODE_FOR_LOOP;

	return node;
}

static Node* parse_switch(Parser* parser)
{
	advance_tkn(parser);

	expect_tkn(parser, (TokenType[]) { TOKEN_CHAR_OPEN_PAREN }, 1);
	advance_tkn(parser);

	Node* expression = parse_expression(parser);

	expect_tkn(parser, (TokenType[]){ TOKEN_CHAR_CLOSE_PAREN }, 1);
	advance_tkn(parser);

	skip_end_lines(parser);

	expect_tkn(parser, (TokenType[]){ TOKEN_CHAR_OPEN_BRACE }, 1);
	advance_tkn(parser);

	skip_end_lines(parser);
	
	NodeList* cases = malloc(sizeof(NodeList));

	if (cases == NULL) 
	{ 
		parser_error("Failed to allocate memory for node list..."); 
		exit(1); 
	}

	Node** current = &cases->head;

	while (peek_tkn(parser)->token_type != TOKEN_CHAR_CLOSE_BRACE)
	{
		expect_tkn(parser, (TokenType[]){ TOKEN_KEYWORD_CASE }, 1);
		advance_tkn(parser);

		Node* condition = parse_expression(parser);

		expect_tkn(parser, (TokenType[]){ TOKEN_CHAR_COLON }, 1);
		advance_tkn(parser);

		skip_end_lines(parser);

		int is_new_scope = 0;

		if (peek_tkn(parser)->token_type == TOKEN_CHAR_OPEN_BRACE)
		{
			is_new_scope = 1;

			advance_tkn(parser);

			skip_end_lines(parser);
		}

		Node* case_node = parse_switch_label_content(parser, condition, is_new_scope);

		case_node->switch_case_block_node.switch_case_block.new_scope = is_new_scope;

		*current = case_node;
		current = &case_node->next;

		skip_end_lines(parser);
	}

	*current = NULL;

	advance_tkn(parser);

	skip_end_lines(parser);

	Node* node = malloc(sizeof(Node));

	if (node == NULL) 
	{ 
		parser_error("Failed to allocate memory for switch statement node..."); 
		exit(1); 
	}

	node->type = NODE_SWITCH_STATEMENT;
	node->switch_statement_node.switch_statement.value = expression;
	node->switch_statement_node.switch_statement.case_list = cases;

	return node;
}

static Node* parse_continue(Parser* parser)
{
	Node* node = malloc(sizeof(Node));

	if (node == NULL) 
	{ 
		parser_error("Failed to allocate memory for continue node..."); 
		exit(1); 
	}

	node->type = NODE_CONTINUE;

	return node;
}

static Node* parse_break(Parser* parser)
{
	Node* node = malloc(sizeof(Node));

	if (node == NULL) 
	{ 
		parser_error("Failed to allocate memory for operation node..."); 
		exit(1); 
	}

	node->type = NODE_BREAK;
	advance_tkn(parser);

	return node;
}

static NodeList* parse_params(Parser* parser)
{
	NodeList* params_list = malloc(sizeof(NodeList));

	if (params_list == NULL) 
	{
		parser_error("Failed to allocate memory for argument list");
		exit(1);
	}

	params_list->head = NULL;
	Node** current = &params_list->head;

	int offset = 0;

	while (peek_tkn(parser)->token_type != TOKEN_CHAR_CLOSE_PAREN)
	{
		if (offset > 0)
		{
			expect_tkn(parser, (TokenType[]){ TOKEN_CHAR_COMMA }, 1);
			advance_tkn(parser);
		}
		
		expect_tkn(parser, (TokenType[]){ TOKEN_IDENTIFIER }, 1);
		const Token* ident_token = consume_token(parser);

		expect_tkn(parser, (TokenType[]){ TOKEN_CHAR_COLON }, 1);
		
		Type* type = handle_type_declaration(parser);

		if (type == NULL)
		{
			exit(1);
		}
		
		Node* arg_node = malloc(sizeof(Node));

		if (arg_node == NULL) 
		{
			parser_error("Failed to allocate memory for param node...");
			exit(1);
		}

		arg_node->type = NODE_PARAMETER;
		arg_node->next = NULL;
		arg_node->param_node.param = (ParamNode) { type, strndup(ident_token->start, ident_token->length) };
		
		offset++;
		*current = arg_node;
		current = &arg_node->next;

		char offset_msg[50];
		snprintf(offset_msg, sizeof(offset_msg), "New argument added, offset: %d", offset);
		parser_info(offset_msg);
	}

	return params_list;
}

static NodeList* parse_args(Parser* parser)
{
	NodeList* node_list = malloc(sizeof(NodeList));

	if (node_list == NULL) 
	{
		exit(1);
	}

	node_list->head = NULL;

	Node** current = &node_list->head;

	int offset = 0;

	while (peek_tkn(parser)->token_type != TOKEN_CHAR_CLOSE_PAREN)
	{
		if (offset > 0) 
		{
			expect_tkn(parser, (TokenType[]) { TOKEN_CHAR_COMMA }, 1);
			advance_tkn(parser);
		}

		Node* expression = parse_expression(parser);

		if (expression == NULL)
		{
			exit(1);
		}

		Node* node_next = malloc(sizeof(Node));

		if (node_next == NULL)
		{
			exit(1);
		}

		node_next->type = NODE_ARGUMENT;
		node_next->argument_node.argument.value = expression;

		*current = node_next;
		current = &node_next->next;

		offset++;
		printf("[Parse] [Debug] Added a argument: %d...\n", offset);
	}

	*current = NULL;

	return node_list;
}

static Node* parse_import(Parser* parser)
{
	advance_tkn(parser);

	expect_tkn(parser, (TokenType[]) { TOKEN_LITERAL_STRING }, 1);
	Token* token_lib_name = consume_token(parser);

	expect_tkn(parser, (TokenType[]) { TOKEN_KEYWORD_AS }, 1);
	advance_tkn(parser);
	
	expect_tkn(parser, (TokenType[]) { TOKEN_IDENTIFIER }, 1);
	Token* identifier = consume_token(parser);

	skip_end_lines(parser);

	char* identifier_str = strndup(identifier->start, identifier->length);

	char* import_path = token_lib_name->str_value;

	Node* node = malloc(sizeof(Node));

	if (node == NULL) 
	{ 
		parser_error("Failed to allocate memory for import node..."); 
		exit(1); 
	}

	node->type = NODE_IMPORT;

	node->import_statement_node.import_node.is_local = 0;

	node->import_statement_node.import_node.is_local = 1;

	node->import_statement_node.import_node.import_path = token_lib_name->str_value;
	node->import_statement_node.import_node.identifier = identifier_str;

	return node;
}

static Node* parse_new_class_instance(Parser* parser, char* class_name)
{
	advance_tkn(parser);

	NodeList* args = NULL;

	if (peek_tkn(parser)->token_type != TOKEN_CHAR_CLOSE_PAREN)
	{
		args = parse_args(parser);
	}
	
	advance_tkn(parser);

	Node* instance = malloc(sizeof(Node));

	if (instance == NULL) 
	{ 
		parser_error("Failed to allocate memory for create instance node..."); 
		exit(1); 
	}

	instance->type = NODE_CREATE_INSTANCE;

	instance->create_instance_node.create_instance.class_name = class_name;
	instance->create_instance_node.create_instance.constructor_args = args;

	return instance;
}

static NodeList* parse_array_values(Parser* parser)
{
	NodeList* list = malloc(sizeof(NodeList));

	if (list == NULL) 
	{ 
		parser_error("Failed to allocate memory for node list..."); 
		exit(1); 
	}

	list->head = NULL;

	Node** current = &list->head;

	int size = 0;
	
	while (peek_tkn(parser)->token_type != TOKEN_CHAR_CLOSE_BRACE)
	{
		if (size > 0)
		{
			expect_tkn(parser, (TokenType[]) { TOKEN_CHAR_COMMA }, 1);
			advance_tkn(parser);
		}
		
		Node* expr = parse_expression(parser);

		*current = expr;
		current = &expr->next;

		size++;
	}

	*current = NULL;

	return list;
}

static Node* parse_new_array(Parser* parser, Type* array_type)
{
	expect_tkn(parser, (TokenType[]) { TOKEN_CHAR_OPEN_BRACE }, 1);
	advance_tkn(parser);

	NodeList* values = NULL;
	
	if (peek_tkn(parser)->token_type != TOKEN_CHAR_CLOSE_BRACE)
	{
		values = parse_array_values(parser);
	}

	expect_tkn(parser, (TokenType[]) { TOKEN_CHAR_CLOSE_BRACE }, 1);
	advance_tkn(parser);

	Node* node = malloc(sizeof(Node));

	if (node == NULL) 
	{ 
		parser_error("Failed to allocate memory for array literal node..."); 
		exit(1); 
	}

	node->type = NODE_ARRAY_LITERAL;

	node->array_literal_node.array_literal.array_type = array_type;
	node->array_literal_node.array_literal.values = values;

	return node;
}

static Node* parse_new(Parser* parser)
{
	Type* type = handle_type_declaration(parser); // Essa função automaticamente da jump no 'new'.

	if (type->type == TYPE_CLASS)
	{
		expect_tkn(parser, (TokenType[]) { TOKEN_CHAR_OPEN_PAREN }, 1);

		return parse_new_class_instance(parser, type->class_name);
	}
	else if (type->type == TYPE_ARRAY)
	{
		return parse_new_array(parser, type);
	}

	printf("[Parser] [Error] Expected a class or type array declaration after 'new' keyword...\n");
	exit(1);
}

static Node** parse_class_declarations(Node* block, int* out_count, Node** constructor_ptr)
{
	int capacity = 4;
	int count = 0;

	int constructor_count = 0;

	Node** node_list = malloc(sizeof(Node*) * capacity);

	if (node_list == NULL) 
	{ 
		parser_error("Failed to allocate memory for node array..."); 
		exit(1); 
	}

	Node* statement = block->block_node.block.statements->head;

	while (statement != NULL)
	{
		if (statement->type != NODE_FUNCTION && statement->type != NODE_DECLARATION)
		{
			printf("[Parser] [Error] Class global scope statements need to be a function or variable declaration...\n");
			exit(1);
		}

		if (statement->type == NODE_FUNCTION && statement->function_node.function.is_constructor)
		{
			if (constructor_count > 0)
			{
				printf("[Analyzer] [Debug] Constructor already declared in class...\n");
				exit(1);
			}

			*constructor_ptr = statement;
			statement = statement->next;

			constructor_count++;
			continue;
		}

		if (capacity <= count)
		{
			capacity *= 2;
			node_list = realloc(node_list, sizeof(Node*) * capacity);

			if (node_list == NULL)
			{
				printf("[Parser] [Error] Failed to realloc memory for list...\n");
				exit(1);
			}
		}

		node_list[count++] = statement;
		statement = statement->next;
	}

	node_list = realloc(node_list, sizeof(Node*) * (count + 1));
	node_list[count] = NULL;

	*out_count = count;

	return node_list;
}

static Node** filter_declarations(Node** list, int type_filter, int* out_count)
{
	int capacity = 4;
	int count = 0;

	Node** result = malloc(sizeof(Node*) * capacity);

	if (result == NULL) 
	{ 
		parser_error("Failed to allocate memory for node array..."); 
		exit(1); 
	}

	for (int i = 0; list[i] != NULL; i++)
	{
		if (list[i]->type == type_filter)
		{
			if (capacity <= count)
			{
				capacity *= 2;
				Node** new_result = realloc(result, sizeof(Node*) * capacity);
				if (new_result == NULL)
				{
					printf("[Parser] [Error] Failed to realloc memory for filtered list...\n");
					free(result);
					exit(1);
				}
				result = new_result;
			}
			result[count++] = list[i];
		}
	}

	result = realloc(result, sizeof(Node*) * (count + 1));
	result[count] = NULL;

	*out_count = count;
	return result;
}

static Node* parse_class(Parser* parser)
{
	advance_tkn(parser);
	expect_tkn(parser, (TokenType[]) { TOKEN_IDENTIFIER }, 1);

	const Token* identifier_token = consume_token(parser);
	skip_end_lines(parser);

	char* super_identifier = NULL;

	if (peek_tkn(parser)->token_type == TOKEN_KEYWORD_EXTENDS)
	{
		advance_tkn(parser);

		expect_tkn(parser, (TokenType[]) { TOKEN_IDENTIFIER }, 1);
		const Token* super = consume_token(parser);
		
		skip_end_lines(parser);

		super_identifier = strndup(super->start, super->length);
	}

	parser->inside_class = 1;

	Node* block = parse_block(parser);

	parser->inside_class = 0;

	advance_tkn(parser);
	skip_end_lines(parser);

	char* identifier = strndup(identifier_token->start, identifier_token->length);

	int decl_count = 0;

	Node* constructor = NULL;

	Node** declarations = parse_class_declarations(block, &decl_count, &constructor);

	int var_count = 0;
	int func_count = 0;

	Node** var_list = filter_declarations(declarations, NODE_DECLARATION, &var_count);
	Node** func_list = filter_declarations(declarations, NODE_FUNCTION, &func_count);

	free(declarations);

	Node* node = malloc(sizeof(Node));

	if (node == NULL) 
	{ 
		parser_error("Failed to allocate memory for class node..."); 
		exit(1); 
	}

	node->type = NODE_CLASS;

	node->class_node.class_node.identifer = identifier;
	node->class_node.class_node.super_identifer = super_identifier;

	node->class_node.class_node.constructor = constructor;

	node->class_node.class_node.var_declare_list = var_list;
	node->class_node.class_node.func_declare_list = func_list;

	node->class_node.class_node.var_count = var_count;
	node->class_node.class_node.func_count = func_count;

	return node;
}

Node* setup_parse_function(Parser* parser, int is_public, int is_static, int is_virtual, int is_override, int is_constructor)
{
	VisibilityType visibility = (is_public) ? VISIBILITY_PUBLIC : VISIBILITY_PRIVATE;
	
	if (!is_constructor)
	{
		advance_tkn(parser);
	}

	expect_tkn(parser, (TokenType[]) { TOKEN_IDENTIFIER }, 1);
	Token* identifier_token = consume_token(parser);

	expect_tkn(parser, (TokenType[]) { TOKEN_CHAR_OPEN_PAREN }, 1);
	advance_tkn(parser);

	NodeList* parameters = NULL;
	
	if (peek_tkn(parser)->token_type != TOKEN_CHAR_CLOSE_PAREN)
	{
		parameters = parse_params(parser);
	}

	expect_tkn(parser, (TokenType[]) { TOKEN_CHAR_CLOSE_PAREN }, 1);
	advance_tkn(parser);

	Type* type = create_type(TYPE_VOID, NULL);
	
	if (peek_tkn(parser)->token_type == TOKEN_CHAR_COLON)
	{
		type = handle_type_declaration(parser);
	}

	return parse_func(parser, visibility, identifier_token, parameters, type, is_static, is_virtual, is_override, is_constructor);
}

Node* setup_parse_variable(Parser* parser, int is_public, int is_static, int is_const)
{
	if ((parser->inside_class && is_const) || !parser->inside_class)
	{
		advance_tkn(parser);
	}
	
	VisibilityType visibility = (is_public) ? VISIBILITY_PUBLIC : VISIBILITY_PRIVATE;
	
	expect_tkn(parser, (TokenType[]) { TOKEN_IDENTIFIER }, 1);
	Token* identifier_token = consume_token(parser);

	expect_tkn(parser, (TokenType[]) { TOKEN_CHAR_COLON }, 1);
	
	Type* type = handle_type_declaration(parser);

	return parse_var(parser, visibility, identifier_token, type, is_const, is_static);
}

Node* handle_function_and_variable_parse(Parser* parser)
{
	int is_public = 0;
	int is_static = 0;
	
	if (peek_tkn(parser)->token_type == TOKEN_KEYWORD_PUB || peek_tkn(parser)->token_type == TOKEN_KEYWORD_PRIV)
	{
		if (!parser->inside_class)
		{
			exit(1);
		}

		is_public = (peek_tkn(parser)->token_type == TOKEN_KEYWORD_PUB);

		advance_tkn(parser);
	}

	if (peek_tkn(parser)->token_type == TOKEN_KEYWORD_STATIC)
	{
		is_static = 1;

		advance_tkn(parser);
	}

	if ((peek_tkn(parser)->token_type == TOKEN_KEYWORD_LET || peek_tkn(parser)->token_type == TOKEN_KEYWORD_CONST) && !parser->inside_class)
	{
		return setup_parse_variable(parser, is_public, is_static, (peek_tkn(parser)->token_type == TOKEN_KEYWORD_CONST));
	}
	else if ((peek_tkn(parser)->token_type == TOKEN_IDENTIFIER || peek_tkn(parser)->token_type == TOKEN_KEYWORD_CONST) && parser->inside_class)
	{
		return setup_parse_variable(parser, is_public, is_static, (peek_tkn(parser)->token_type == TOKEN_KEYWORD_CONST));
	}

	int is_virtual = 0;
	int is_override = 0;
	
	if (peek_tkn(parser)->token_type == TOKEN_KEYWORD_VIRTUAL || peek_tkn(parser)->token_type == TOKEN_KEYWORD_OVERRIDE)
	{
		if (!parser->inside_class)
		{
			printf("[Analyzer] [Debug] Override / Virtual keyword outside a class...\n");
			exit(1);
		}
		
		is_virtual = (peek_tkn(parser)->token_type == TOKEN_KEYWORD_VIRTUAL);
		is_override = (peek_tkn(parser)->token_type == TOKEN_KEYWORD_OVERRIDE);

		advance_tkn(parser);
	}

	if (peek_tkn(parser)->token_type == TOKEN_KEYWORD_FUNCTION)
	{
		return setup_parse_function(parser, is_public, is_static, is_virtual, is_override, 0);
	}

	return setup_parse_function(parser, is_public, is_static, is_virtual, is_override, 1);
}

int is_var_or_function(Parser* parser)
{
	TokenType type = peek_tkn(parser)->token_type;

	return (
		type == TOKEN_KEYWORD_PUB ||
		type == TOKEN_KEYWORD_PRIV ||
		type == TOKEN_KEYWORD_LET ||
		type == TOKEN_KEYWORD_CONST ||
		type == TOKEN_KEYWORD_STATIC ||
		type == TOKEN_KEYWORD_VIRTUAL ||
		type == TOKEN_KEYWORD_OVERRIDE ||
		type == TOKEN_KEYWORD_FUNCTION
	);
}

Node* parse_stmt(Parser* parser)
{
	if (peek_tkn(parser)->token_type == TOKEN_KEYWORD_IMPORT)
	{
		return parse_import(parser);
	}

	if (peek_tkn(parser)->token_type == TOKEN_KEYWORD_CLASS)
	{
		return parse_class(parser);
	}

	if (is_var_or_function(parser))
	{
		return handle_function_and_variable_parse(parser);
	}

	if (peek_tkn(parser)->token_type == TOKEN_KEYWORD_IF)
	{
		return parse_if(parser);
	}

	if (peek_tkn(parser)->token_type == TOKEN_KEYWORD_SWITCH)
	{
		return parse_switch(parser);
	}

	if (parser->current->token_type == TOKEN_KEYWORD_RETURN)
	{
		return parse_return(parser);
	}

	if (parser->current->token_type == TOKEN_KEYWORD_CONTINUE)
	{
		return parse_continue(parser);
	}

	if (parser->current->token_type == TOKEN_KEYWORD_BREAK)
	{
		return parse_break(parser);
	}

	if (peek_tkn(parser)->token_type == TOKEN_KEYWORD_WHILE)
	{
		return parse_while_loop(parser);
	}

	if (peek_tkn(parser)->token_type == TOKEN_KEYWORD_FOR)
	{
		return parse_for_loop(parser);
	}

	Node* left = parse_unary(parser);

	if (peek_tkn(parser)->token_type == TOKEN_OPERATOR_ASSIGN)
	{
		advance_tkn(parser);

		return parse_var_assign(parser, left);
	}

	if (peek_tkn(parser)->token_type == TOKEN_OPERATOR_INCREMENT)
	{
		return parse_var_increment(parser, left);
	}

	if (peek_tkn(parser)->token_type == TOKEN_OPERATOR_DECREMENT)
	{
		return parse_var_decrement(parser, left);
	}

	if (peek_tkn(parser)->token_type == TOKEN_OPERATOR_PLUS_EQUALS)
	{
		return parse_var_plus_equals(parser, left);
	}

	if (peek_tkn(parser)->token_type == TOKEN_OPERATOR_MINUS_EQUALS)
	{
		return parse_var_minus_equals(parser, left);
	}

	if (peek_tkn(parser)->token_type == TOKEN_OPERATOR_DIVIDED_EQUALS)
	{
		return parse_var_divided_equals(parser, left);
	}

	if (peek_tkn(parser)->token_type == TOKEN_OPERATOR_TIMES_EQUALS)
	{
		return parse_var_times_equals(parser, left);
	}

	return left;
}

static Node* parse_unary(Parser* parser)
{
	if (peek_tkn(parser)->token_type == TOKEN_KEYWORD_TYPE)
	{
		Type* type = create_type(peek_tkn(parser)->var_type, NULL);
		
		advance_tkn(parser);

		expect_tkn(parser, (TokenType[]) { TOKEN_CHAR_OPEN_PAREN }, 1);
		advance_tkn(parser);

		Node* expression = parse_expression(parser);

		expect_tkn(parser, (TokenType[]) { TOKEN_CHAR_CLOSE_PAREN }, 1);
		advance_tkn(parser);
		
		Node* node = malloc(sizeof(Node));

		if (node == NULL) 
		{ 
			parser_error("Failed to allocate memory for cast node..."); 
			exit(1); 
		}

		node->type = NODE_CAST;

		node->cast_statement_node.cast_node.cast_type = type;
		node->cast_statement_node.cast_node.expression = expression;

		return node;
	}
	
	if (peek_tkn(parser)->token_type == TOKEN_OPERATOR_ADRESS)
	{
		advance_tkn(parser);

		Node* expr = parse_unary(parser);

		Node* node = malloc(sizeof(Node));

		if (node == NULL) 
		{ 
			parser_error("Failed to allocate memory for adress of node..."); 
			exit(1); 
		}

		node->type = NODE_ADRESS_OF;
		node->adress_of_node.adress_of.expression = expr;

		return node;
	}

	if (peek_tkn(parser)->token_type == TOKEN_CHAR_STAR)
	{
		advance_tkn(parser);

		Node* expr = parse_unary(parser);

		Node* node = malloc(sizeof(Node));

		if (node == NULL) 
		{ 
			parser_error("Failed to allocate memory for dereference node..."); 
			exit(1); 
		}

		node->type = NODE_DEREFERENCE;
		node->dereference_node.dereference.ptr = expr;

		return node;
	}
	
	if (peek_tkn(parser)->token_type == TOKEN_KEYWORD_NEW)
	{
		return parse_new(parser);
	}

	return parse_primary(parser);
}

void free_type(Type* type, Node* node)
{
	if (type == NULL)
	{
		return;
	}

	if (type->type > 28 || type->type < 0)
	{
		type = NULL;

		return;
	}

	free_type(type->base, node);
	
	if (type->class_name != NULL)
	{
		free(type->class_name);
	}

	free(type);
	type = NULL;
}

void free_node_list(NodeList* list)
{
	if (list == NULL) 
	{
		return;
	}

	Node* node = list->head;

	while (node) 
	{
		Node* next = node->next;
		free_node(node);
		node = next;
	}

	free(list);
}

void free_node_array(Node** array, int count)
{
	if (!array) 
	{
		return;
	}
	
	for (int i = 0; i < count; ++i) 
	{
		if (array[i] != NULL) 
		{
			free_node(array[i]);
		}
	}
}

void free_node(Node* node)
{
	if (node == NULL)
	{
		return;
	}

	switch (node->type)
	{
		case NODE_DECLARATION:
		{ 
			free(node->declare_node.declare.identifier);
			
			if (node->declare_node.declare.default_value != NULL) 
			{
				free_node(node->declare_node.declare.default_value);
			}

			free_type(node->declare_node.declare.var_type, node);

			break;
		}

		case NODE_FUNCTION:
		{
			free(node->function_node.function.identifier);
			
			if (node->function_node.function.params != NULL) 
			{
				free_node_list(node->function_node.function.params);
			}
			
			if (node->function_node.function.block_node != NULL)
			{
				free_node(node->function_node.function.block_node);
			}

			break;
		}

		case NODE_IDENTIFIER:
		{
			free(node->variable_node.variable.identifier);
			
			break;
		}

		case NODE_OPERATION:
		{
			free_node(node->operation_node.operation.left);
			free_node(node->operation_node.operation.right);

			break;
		}

		case NODE_FUNCTION_CALL:
		{
			free_node(node->function_call_node.function_call.callee);
			
			if (node->function_call_node.function_call.arguments != NULL)
			{
				free_node_list(node->function_call_node.function_call.arguments);
			}

			break;
		}

		case NODE_VARIABLE_ASSIGN:
		{
			free(node->variable_assign_node.variable_assign.left);

			free_node(node->variable_assign_node.variable_assign.assign_value);

			break;
		}

		case NODE_ARGUMENT:
		{
			Node* value = node->argument_node.argument.value;

			free_node(value);

			break;
		}

		case NODE_RETURN:
		{
			if (node->return_statement_node.return_statement.return_value != NULL)
			{
				free_node(node->return_statement_node.return_statement.return_value);
			}
			
			break;
		}

		case NODE_SWITCH_STATEMENT:
		{
			free_node(node->switch_statement_node.switch_statement.value);
			
			free_node_list(node->switch_statement_node.switch_statement.case_list);

			break;
		}

		case NODE_SWITCH_CASE_BLOCK:
		{
			free_node(node->switch_case_block_node.switch_case_block.condition);

			free_node(node->switch_case_block_node.switch_case_block.block);

			break;
		}

		case NODE_BLOCK:
		{
			free_node_list(node->block_node.block.statements);

			break;
		}

		case NODE_THIS:
		{
			break;
		}

		case NODE_IF:
		{
			free_node(node->if_statement_node.if_statement.condition_top);
			
			free_node(node->if_statement_node.if_statement.then_branch);
			
			if (node->if_statement_node.if_statement.else_branch != NULL) 
			{
				free_node(node->if_statement_node.if_statement.else_branch);
			}

			break;
		}

		case NODE_FOR_LOOP:
		{
			free_node(node->for_loop_node.for_loop.init);
			free_node(node->for_loop_node.for_loop.condition);
			free_node(node->for_loop_node.for_loop.then_statement);

			free_node(node->for_loop_node.for_loop.then_block);
			
			break;
		}

		case NODE_LITERAL:
		{
			if (node->literal_node.literal.literal_type->type == TYPE_STRING)
			{
				free(node->literal_node.literal.string_value);
			}
			
			free_type(node->literal_node.literal.literal_type, node);

			break;
		}

		case NODE_CLASS:
		{
			if (node->class_node.class_node.super_identifer != NULL)
			{
				free(node->class_node.class_node.super_identifer);
			}

			free(node->class_node.class_node.identifer);

			if (node->class_node.class_node.constructor != NULL)
			{
				free_node(node->class_node.class_node.constructor);
			}

			free_node_array(node->class_node.class_node.func_declare_list, node->class_node.class_node.func_count);
			
			free_node_array(node->class_node.class_node.var_declare_list, node->class_node.class_node.var_count);

			free(node->class_node.class_node.func_declare_list);
			free(node->class_node.class_node.var_declare_list);

			break;
		}

		case NODE_PARAMETER:
		{
			free_type(node->param_node.param.argument_type, node);

			free(node->param_node.param.identifier);

			break;
		}

		case NODE_CREATE_INSTANCE:
		{
			free(node->create_instance_node.create_instance.class_name);

			free_node_list(node->create_instance_node.create_instance.constructor_args);

			break;
		}

		case NODE_MEMBER_ACCESS:
		{
			free_node(node->member_access_node.member_access.object);

			free(node->member_access_node.member_access.member_name);

			break;
		}

		case NODE_ARRAY_ACCESS:
		{
			free_node(node->acess_array_node.acess_array.index_expr);

			free_node(node->acess_array_node.acess_array.array);

			break;
		}

		case NODE_CAST:
		{
			free_node(node->cast_statement_node.cast_node.expression);

			free_type(node->cast_statement_node.cast_node.cast_type, node);

			break;
		}

		case NODE_ARRAY_LITERAL:
		{
			free_node_list(node->array_literal_node.array_literal.values);

			free_type(node->array_literal_node.array_literal.array_type, node);

			break;
		}

		case NODE_IMPORT:
		{
			free(node->import_statement_node.import_node.identifier);
			free(node->import_statement_node.import_node.import_path);

			break;
		}

		case NODE_BREAK:
		{
			break;
		}

		case NODE_DEREFERENCE:
		{
			free_node(node->dereference_node.dereference.ptr);

			break;
		}

		case NODE_ADRESS_OF:
		{
			free_node(node->adress_of_node.adress_of.expression);
			
			break;
		}

		default:
		{
			printf("[Parser] [Debug] Invalid node type: %d...\n", node->type);
			break;
		}
	}

	
	free(node);
	node = NULL;
}