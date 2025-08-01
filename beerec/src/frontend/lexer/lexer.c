#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "lexer.h"
#include "../../utils/string_builder/string_builder.h"
#include "../../utils/utils.h"

static char peek(const Lexer* lexer)
{
	return *lexer->current;
}

static char jmp(Lexer* lexer, int jmp_size)
{
	lexer->current += jmp_size;
	return *lexer->current;
}

static char advance(Lexer* lexer)
{
	if (*lexer->current == '\0')
	{
		return '\0';
	}

	return *lexer->current++;
}

static int is_end(const Lexer* lexer)
{
	if (*lexer->current == '\0')
	{
		return 1;
	}

	return 0;
}

static int is_alpha(const char c) 
{
	if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
	{
		return 1;
	}

	return 0;
}

static int is_digit(const char c)
{
	if (c >= '0' && c <= '9')
	{
		return 1;
	}

	return 0;
}

static int is_size(const Lexer* lexer, const int size)
{
	if (lexer->current - lexer->start == size)
	{
		return 1;
	}

	return 0;
}

TokenType get_by_keyword_type(Lexer* lexer, const char* start, const size_t length)
{
	switch (start[0])
	{
		case 'i':
		{
			if (length == 3 && strncmp(start, "int", 3) == 0)
			{
				return TOKEN_KEYWORD_TYPE;
			}

			if (length == 2 && strncmp(start, "if", 2) == 0)
			{
				return TOKEN_KEYWORD_IF;
			}

			if (length == 6 && strncmp(start, "import", 6) == 0)
			{
				return TOKEN_KEYWORD_IMPORT;
			}

			break;
		}

		case 't':
		{
			if (length == 4 && strncmp(start, "this", 4) == 0)
			{
				return TOKEN_KEYWORD_THIS;
			}

			break;
		}

		case 'a':
		{
			if (length == 4 && strncmp(start, "any*", 4) == 0)
			{
				return TOKEN_KEYWORD_TYPE;
			}

			if (length == 2 && strncmp(start, "as", 2) == 0)
			{
				return TOKEN_KEYWORD_AS;
			}

			break;
		}

		case 's':
		{
			if (length == 6 && strncmp(start, "string", 6) == 0)
			{
				return TOKEN_KEYWORD_TYPE;
			}

			if (length == 6 && strncmp(start, "switch", 6) == 0)
			{
				return TOKEN_KEYWORD_SWITCH;
			}

			if (length == 6 && strncmp(start, "static", 6) == 0)
			{
				return TOKEN_KEYWORD_STATIC;
			}

			if (length == 5 && strncmp(start, "super", 5) == 0)
			{
				return TOKEN_KEYWORD_SUPER;
			}

			break;
		}

		case 'b':
		{
			if (length == 5 && strncmp(start, "break", 5) == 0)
			{
				return TOKEN_KEYWORD_BREAK;
			}

			if (length == 4 && strncmp(start, "bool", 4) == 0)
			{
				return TOKEN_KEYWORD_TYPE;
			}

			break;
		}

		case 'f':
		{
			if (length == 3 && strncmp(start, "for", 3) == 0)
			{
				return TOKEN_KEYWORD_FOR;
			}

			if (length == 5 && strncmp(start, "float", 5) == 0)
			{
				return TOKEN_KEYWORD_TYPE;
			}

			if (length == 2 && strncmp(start, "fn", 2) == 0)
			{
				return TOKEN_KEYWORD_FUNCTION;
			}

			break;
		}

		case 'r':
		{
			if (length == 6 && strncmp(start, "return", 6) == 0)
			{
				return TOKEN_KEYWORD_RETURN;
			}

			break;
		}

		case 'v':
		{
			if (length == 4 && strncmp(start, "void", 4) == 0)
			{
				return TOKEN_KEYWORD_TYPE_VOID;
			}

			if (length == 7 && strncmp(start, "virtual", 7) == 0)
			{
				return TOKEN_KEYWORD_VIRTUAL;
			}

			break;
		}

		case 'o':
		{
			if (length == 8 && strncmp(start, "override", 8) == 0)
			{
				return TOKEN_KEYWORD_OVERRIDE;
			}

			break;
		}

		case 'w':
		{
			if (length == 5 && strncmp(start, "while", 5) == 0)
			{
				return TOKEN_KEYWORD_WHILE;
			}

			break;
		}

		case 'e':
		{
			if (length == 4 && strncmp(start, "else", 4) == 0)
			{
				return TOKEN_KEYWORD_ELSE;
			}

			if (length == 7 && strncmp(start, "extends", 7) == 0)
			{
				return TOKEN_KEYWORD_EXTENDS;
			}

			if (length == 6 && strncmp(start, "export", 6) == 0)
			{
				return TOKEN_KEYWORD_EXPORT;
			}

			break;
		}

		case 'c':
		{
			if (length == 8 && strncmp(start, "continue", 8) == 0)
			{
				return TOKEN_KEYWORD_CONTINUE;
			}

			if (length == 4 && strncmp(start, "char", 4) == 0)
			{
				return TOKEN_KEYWORD_TYPE;
			}

			if (length == 5 && strncmp(start, "const", 5) == 0)
			{
				return TOKEN_KEYWORD_CONST;
			}

			if (length == 4 && strncmp(start, "case", 4) == 0)
			{
				return TOKEN_KEYWORD_CASE;
			}

			if (length == 5 && strncmp(start, "class", 5) == 0)
			{
				return TOKEN_KEYWORD_CLASS;
			}

			break;
		}

		case 'n':
		{
			if (length == 4 && strncmp(start, "null", 4) == 0)
			{
				return TOKEN_LITERAL_NULL;
			}

			if (length == 3 && strncmp(start, "new", 3) == 0)
			{
				return TOKEN_KEYWORD_NEW;
			}

			break;
		}

		case 'l':
		{
			if (length == 3 && strncmp(start, "let", 3) == 0)
			{
				return TOKEN_KEYWORD_LET;
			}

			break;
		}

		case 'p':
		{
			if (length == 6 && strncmp(start, "public", 6) == 0)
			{
				return TOKEN_KEYWORD_PUB;
			}

			if (length == 7 && strncmp(start, "private", 7) == 0)
			{
				return TOKEN_KEYWORD_PRIV;
			}

			break;
		}

		default:
		{
			break;
		}
	}


	return TOKEN_IDENTIFIER;
}

static Token identifier(Lexer* lexer)
{
	while (is_alpha(peek(lexer)) || is_digit(peek(lexer)))
	{
		advance(lexer);
	}
		 
	const size_t length = (size_t)(lexer->current - lexer->start);
	
	char* type_str = strndup(lexer->start, length);

	VarType var_type = { 0 };

	const TokenType type = get_by_keyword_type(lexer, lexer->start, length);

	if (strcmp(type_str, "int") == 0)
	{
		var_type = TYPE_INT;
	}
	else if (strcmp(type_str, "string") == 0)
	{
		var_type = TYPE_STRING;
	}
	else if (strcmp(type_str, "float") == 0)
	{
		var_type = TYPE_FLOAT;
	}
	else if (strcmp(type_str, "double") == 0)
	{
		var_type = TYPE_DOUBLE;
	}
	else if (strcmp(type_str, "char") == 0)
	{
		var_type = TYPE_CHAR;
	} 
	else if (strcmp(type_str, "bool") == 0)
	{
		var_type = TYPE_BOOL;
	}
	else if (strcmp(type_str, "any*") == 0)
	{
		var_type = TYPE_ANY_PTR;
	}

	free(type_str);

	Token token = (Token){ type, lexer->start, length, lexer->line };

	if (type == TOKEN_KEYWORD_TYPE) 
	{
		token.var_type = var_type;

		return token;
	}

	return token;
}

Token handle_literals(Lexer* lexer, int negative)
{
	const char c = peek(lexer);

	if (c == '\"')
	{
		advance(lexer);

		StringBuilder sb;
		
		init(&sb);

		while (peek(lexer) != '\"')
		{
			if (peek(lexer) == '\n') 
			{
				printf("[Lexer] [Error] String not finished...");
				exit(1);
			}
			
			const char c_ = peek(lexer);

			const char tmp[2] = { c_, '\0' };

			append(&sb, tmp);

			advance(lexer);
		}

		advance(lexer);

		char* str = malloc(strlen(sb.buffer) + 1);

		strcpy(str, sb.buffer);

		free_string_builder(&sb);

		Token tkn = (Token) { TOKEN_LITERAL_STRING, lexer->start, (size_t) (lexer->current - lexer->start), lexer->line };

		tkn.var_type = TYPE_STRING;
		tkn.str_value = str;
		
		return tkn;
	}

	if (is_digit(c))
	{
		int decimal = 0;

		int i = 0;

		char actual = peek(lexer);

		while (is_digit(actual) || actual == '.')
		{
			if (actual == '.') 
			{
				decimal++;
			}

			advance(lexer);
			actual = peek(lexer);
			i++;
		}

		if (decimal >= 2)
		{
			printf("[Lexer] [Error] Detected a malformed number: %.*s", (int)(lexer->current - lexer->start), lexer->start);
			exit(1);
		}

		if (actual == 'F' || actual == 'f')
		{
			advance(lexer);

			Token tkn = (Token) { TOKEN_LITERAL_FLOAT, lexer->start, (size_t)(lexer->current - lexer->start), lexer->line };
			tkn.negative = negative;

			return tkn;
		}

		if (decimal == 1)
		{

			Token tkn = (Token) { TOKEN_LITERAL_DOUBLE, lexer->start, (size_t)(lexer->current - lexer->start), lexer->line };
			tkn.negative = negative;

			return tkn;
		}

		Token tkn = (Token) { TOKEN_LITERAL_INT, lexer->start, (size_t)(lexer->current - lexer->start), lexer->line };
		tkn.negative = negative;

		return tkn;
	}

	if (c == '\'') 
	{
		advance(lexer);

		char value;

		if (peek(lexer) == '\\') 
		{
			advance(lexer);

			char esc = peek(lexer);

			switch (esc)
			{
				case 'n': 
				{
					value = '\n'; 

					break;
				}

				case 't':
				{
					value = '\t'; 

					break;
				}

				case '\'': 
				{
					value = '\''; 

					break;
				}

				case '\\': 
				{
					value = '\\'; 

					break;
				}

				case '0':
				{
					value = '\0';

					break;
				}

				default:
					printf("[Lexer] [Error] Invalid escape sequence \\%c\n", esc);
					exit(1);
				}

				advance(lexer);
		} 
		else 
		{
			value = peek(lexer);
			advance(lexer);
		}

		if (peek(lexer) != '\'')
		{
			printf("[Lexer] [Error] Unterminated char literal\n");
			exit(1);
		}

		advance(lexer);

		Token token;

		token.token_type = TOKEN_LITERAL_CHAR;
		token.start = lexer->start; 
		token.length = (size_t)(lexer->current - lexer->start);
		token.line = lexer->line;
		token.ch_value = value;
		
		return token;
	}

	if (peek(lexer) == 't' || peek(lexer) == 'f') 
	{
		if (strncmp(lexer->start, "true", 4) == 0) 
		{
			jmp(lexer, 4);
			
			Token tkn = (Token) { TOKEN_LITERAL_BOOL, lexer->start, (size_t) (lexer->current - lexer->start), lexer->line };
			tkn.bool_value = 1;

			return tkn;
		}

		if (strncmp(lexer->start, "false", 5) == 0) 
		{
			jmp(lexer, 5);
			
			Token tkn = (Token) { TOKEN_LITERAL_BOOL, lexer->start, (size_t) (lexer->current - lexer->start), lexer->line };
			tkn.bool_value = 0;
			
			return tkn;
		}
	}

	return (Token) { TOKEN_UNKNOWN, lexer->start, 1, lexer->line };
}

Token read_next_tkn(Lexer* lexer)
{
	while (1)
	{
		lexer->start = lexer->current;
		
		if (is_end(lexer))
		{
			return (Token){ TOKEN_END_SRC, lexer->start, 0, lexer->line };
		}

		const char c = peek(lexer);

		if (c == ' ' || c == '\r' || c == '\t')
		{
			advance(lexer);

			continue;
		}

		if (c == '\n')
		{
			Token tkn = (Token) { TOKEN_END_LINE, lexer->start, 1, lexer->line };

			advance(lexer);

			lexer->line++;

			return tkn;
		}

		int negative = 0;

		switch (c)
		{
			case '.':
			{
				advance(lexer);

				return (Token) { TOKEN_OPERATOR_DOT, lexer->start, 1, lexer->line };
				break;
			}

			case '+':
			{
				advance(lexer);
			
				if (peek(lexer) == '+') 
				{
					advance(lexer);

					return (Token){ TOKEN_OPERATOR_INCREMENT, lexer->start, 2, lexer->line };
				} 
				else if (peek(lexer) == '=') 
				{
					advance(lexer);

					return (Token){ TOKEN_OPERATOR_PLUS_EQUALS, lexer->start, 2, lexer->line };
				}

				return (Token){ TOKEN_OPERATOR_PLUS, lexer->start, 1, lexer->line };
			}

			case '-':
			{
				advance(lexer);
			
				if (peek(lexer) == '-') 
				{
					advance(lexer);

					return (Token){ TOKEN_OPERATOR_DECREMENT, lexer->start, 2, lexer->line };
				}
				else if (peek(lexer) == '=') 
				{
					advance(lexer);

					return (Token){ TOKEN_OPERATOR_MINUS_EQUALS, lexer->start, 2, lexer->line };
				}
				else if (peek(lexer) == '>') 
				{
					advance(lexer);

					return (Token){ TOKEN_OPERATOR_ACCESS_PTR, lexer->start, 2, lexer->line };
				}
				else if (is_digit(peek(lexer))) 
				{
					negative = 1;
					break;
				}

				return (Token){ TOKEN_OPERATOR_MINUS, lexer->start, 1, lexer->line };
			}

			case '=':
			{
				advance(lexer);
			
				if (peek(lexer) == '=') 
				{
					advance(lexer);

					return (Token){ TOKEN_OPERATOR_EQUALS, lexer->start, 2, lexer->line };
				}

				return (Token){ TOKEN_OPERATOR_ASSIGN, lexer->start, 1, lexer->line };
			}

			case ':':
			{
				advance(lexer);

				return (Token){ TOKEN_CHAR_COLON, lexer->start, 1, lexer->line };
			}

			case '*':
			{
				advance(lexer);
			
				if (peek(lexer) == '=') 
				{
					advance(lexer);
					
					return (Token){ TOKEN_OPERATOR_TIMES_EQUALS, lexer->start, 2, lexer->line };
				}
				
				return (Token){ TOKEN_CHAR_STAR, lexer->start, 1, lexer->line };
			}

			case '/':
			{
				advance(lexer);

				if (peek(lexer) == '=') 
				{
					advance(lexer);
					
					return (Token){ TOKEN_OPERATOR_DIVIDED_EQUALS, lexer->start, 2, lexer->line };
				}
				else if (peek(lexer) == '/')
				{
					while (peek(lexer) != '\n' && !is_end(lexer))
					{
						advance(lexer);
					}
			
					return (Token){ TOKEN_KEYWORD_ONE_LINE_COMMENT, lexer->start, (lexer->current - lexer->start), lexer->line };
				}
				else if (peek(lexer) == '*')
				{
					while (!is_end(lexer))
					{
						if (peek(lexer) == '*')
						{
							advance(lexer);

							if (peek(lexer) == '/')
							{
								advance(lexer);

								break;
							}
						}

						advance(lexer);
					}
			
					return (Token) { TOKEN_KEYWORD_MULTI_LINE_COMMENT, lexer->start, (lexer->current - lexer->start), lexer->line };
				}

				return (Token) { TOKEN_OPERATOR_DIVIDED, lexer->start, 1, lexer->line };
			}

			case '{':
			{
				advance(lexer);

				return (Token) { TOKEN_CHAR_OPEN_BRACE, lexer->start, 1, lexer->line };
			}

			case '}':
			{
				advance(lexer);

				return (Token) { TOKEN_CHAR_CLOSE_BRACE, lexer->start, 1, lexer->line };
			}

			case '[':
			{
				advance(lexer);

				return (Token) { TOKEN_CHAR_OPEN_BRACKET, lexer->start, 1, lexer->line };
			}

			case ']':
			{
				advance(lexer);

				return (Token) { TOKEN_CHAR_CLOSE_BRACKET, lexer->start, 1, lexer->line };
			}

			case '(':
			{
				advance(lexer);

				return (Token) { TOKEN_CHAR_OPEN_PAREN, lexer->start, 1, lexer->line };
			}

			case ')':
			{
				advance(lexer);
			
				return (Token) { TOKEN_CHAR_CLOSE_PAREN, lexer->start, 1, lexer->line };
			}

			case ',':
			{
				advance(lexer);

				return (Token) { TOKEN_CHAR_COMMA, lexer->start, 1, lexer->line };
			}

			case '>':
			{
				advance(lexer);

				if (peek(lexer) == '=') 
				{
					advance(lexer);

					return (Token) { TOKEN_OPERATOR_GREATER_EQUALS, lexer->start, 1, lexer->line };	
				}

				return (Token) { TOKEN_OPERATOR_GREATER, lexer->start, 1, lexer->line };
			}

			case '<':
			{
				advance(lexer);

				if (peek(lexer) == '=') 
				{
					advance(lexer);

					return (Token) { TOKEN_OPERATOR_LESS_EQUALS, lexer->start, 1, lexer->line };
				}

				return (Token) { TOKEN_OPERATOR_LESS, lexer->start, 1, lexer->line };
			}

			case '!':
			{
				advance(lexer);

				if (peek(lexer) == '=') 
				{
					advance(lexer);

					return (Token) { TOKEN_OPERATOR_NOT_EQUALS, lexer->start, 1, lexer->line };
				}

				break;
			}

			case ';':
			{
				advance(lexer);

				return (Token) { TOKEN_CHAR_SEMI_COLON, lexer->start, 1, lexer->line };
				break;
			}

			default:
			{
				break;
			}
		}

		const Token literal = handle_literals(lexer, negative);

		if (literal.token_type != TOKEN_UNKNOWN) 
		{
			return literal;
		}

		if (c == '&') 
		{
			advance(lexer);

			if (peek(lexer) == '&') 
			{
				advance(lexer);

				return (Token){ TOKEN_OPERATOR_AND, lexer->start, 2, lexer->line };
			}

			return (Token){ TOKEN_OPERATOR_ADRESS, lexer->start, 1, lexer->line };
		}

		if (c == '|') 
		{
			advance(lexer);

			if (peek(lexer) == '|')
			{
				advance(lexer);

				return (Token){ TOKEN_OPERATOR_OR, lexer->start, 1, lexer->line };
			}
		}

		if (is_alpha(c))
		{
			return identifier(lexer);
		}

		advance(lexer);

		return (Token){ TOKEN_IDENTIFIER, lexer->start, 1, lexer->line };
	}
}

const char* token_type_to_string(const TokenType type)
{
	switch (type) 
	{
		case TOKEN_IDENTIFIER: return "TOKEN_IDENTIFIER";

		case TOKEN_OPERATOR_ASSIGN: return "TOKEN_OPERATOR_ASSIGN";

		case TOKEN_CHAR_COLON: return "TOKEN_CHAR_COLON";
		case TOKEN_CHAR_COMMA: return "TOKEN_CHAR_COMMA";
		case TOKEN_CHAR_SEMI_COLON: return "TOKEN_CHAR_SEMI_COLON";
		case TOKEN_CHAR_OPEN_BRACE: return "TOKEN_CHAR_OPEN_BRACE";
		case TOKEN_CHAR_CLOSE_BRACE: return "TOKEN_CHAR_CLOSE_BRACE";
		case TOKEN_CHAR_OPEN_PAREN: return "TOKEN_CHAR_OPEN_PAREN";
		case TOKEN_CHAR_CLOSE_PAREN: return "TOKEN_CHAR_CLOSE_PAREN";
		case TOKEN_CHAR_DOUBLE_QUOTE: return "TOKEN_CHAR_DOUBLE_QUOTE";
		case TOKEN_CHAR_STAR: return "TOKEN_CHAR_STAR";
		case TOKEN_CHAR_OPEN_BRACKET: return "TOKEN_CHAR_OPEN_BRACKET";
		case TOKEN_CHAR_CLOSE_BRACKET: return "TOKEN_CHAR_CLOSE_BRACKET";

		case TOKEN_OPERATOR_DIVIDED: return "TOKEN_OPERATOR_DIVIDED";
		case TOKEN_OPERATOR_PLUS: return "TOKEN_OPERATOR_PLUS";
		case TOKEN_OPERATOR_MINUS: return "TOKEN_OPERATOR_MINUS";
		case TOKEN_OPERATOR_PLUS_EQUALS: return "TOKEN_OPERATOR_PLUS_EQUALS";
		case TOKEN_OPERATOR_MINUS_EQUALS: return "TOKEN_OPERATOR_MINUS_EQUALS";
		case TOKEN_OPERATOR_DIVIDED_EQUALS: return "TOKEN_OPERATOR_DIVIDED_EQUALS";
		case TOKEN_OPERATOR_TIMES_EQUALS: return "TOKEN_OPERATOR_TIMES_EQUALS";
		case TOKEN_OPERATOR_DECREMENT: return "TOKEN_OPERATOR_DECREMENT";
		case TOKEN_OPERATOR_INCREMENT: return "TOKEN_OPERATOR_INCREMENT";
		case TOKEN_OPERATOR_AND: return "TOKEN_OPERATOR_AND";
		case TOKEN_OPERATOR_OR: return "TOKEN_OPERATOR_OR";
		case TOKEN_OPERATOR_GREATER: return "TOKEN_OPERATOR_GREATER";
		case TOKEN_OPERATOR_LESS: return "TOKEN_OPERATOR_LESS";
		case TOKEN_OPERATOR_GREATER_EQUALS: return "TOKEN_OPERATOR_GREATER_EQUALS";
		case TOKEN_OPERATOR_LESS_EQUALS: return "TOKEN_OPERATOR_LESS_EQUALS";
		case TOKEN_OPERATOR_EQUALS: return "TOKEN_OPERATOR_EQUALS";
		case TOKEN_OPERATOR_NOT_EQUALS: return "TOKEN_OPERATOR_NOT_EQUALS";
		case TOKEN_OPERATOR_ADRESS: return "TOKEN_OPERATOR_ADRESS";
		case TOKEN_OPERATOR_ACCESS_PTR: return "TOKEN_OPERATOR_ACCESS_PTR";
		case TOKEN_OPERATOR_DOT: return "TOKEN_OPERATOR_DOT";

		case TOKEN_LITERAL_CHAR: return "TOKEN_LITERAL_CHAR";
		case TOKEN_LITERAL_STRING: return "TOKEN_LITERAL_STRING";
		case TOKEN_LITERAL_INT: return "TOKEN_LITERAL_INT";
		case TOKEN_LITERAL_FLOAT: return "TOKEN_LITERAL_FLOAT";
		case TOKEN_LITERAL_DOUBLE: return "TOKEN_LITERAL_DOUBLE";
		case TOKEN_LITERAL_BOOL: return "TOKEN_LITERAL_BOOL";
		case TOKEN_LITERAL_NULL: return "TOKEN_LITERAL_NULL";

		case TOKEN_END_LINE: return "TOKEN_END_LINE";
		case TOKEN_END_SRC: return "TOKEN_END_SRC";

		case TOKEN_KEYWORD_RETURN: return "TOKEN_KEYWORD_RETURN";
		case TOKEN_KEYWORD_IF: return "TOKEN_KEYWORD_IF";
		case TOKEN_KEYWORD_ELSE: return "TOKEN_KEYWORD_ELSE";
		case TOKEN_KEYWORD_FOR: return "TOKEN_KEYWORD_FOR";
		case TOKEN_KEYWORD_WHILE: return "TOKEN_KEYWORD_WHILE";
		case TOKEN_KEYWORD_BREAK: return "TOKEN_KEYWORD_BREAK";
		case TOKEN_KEYWORD_CONTINUE: return "TOKEN_KEYWORD_CONTINUE";
		case TOKEN_KEYWORD_EXTENDS: return "TOKEN_KEYWORD_EXTENDS";
		case TOKEN_KEYWORD_FUNCTION: return "TOKEN_KEYWORD_FUNCTION";
		case TOKEN_KEYWORD_LET: return "TOKEN_KEYWORD_LET";
		case TOKEN_KEYWORD_CONST: return "TOKEN_KEYWORD_CONST";
		case TOKEN_KEYWORD_CASE: return "TOKEN_KEYWORD_CASE";
		case TOKEN_KEYWORD_SWITCH: return "TOKEN_KEYWORD_SWITCH";
		case TOKEN_KEYWORD_ONE_LINE_COMMENT: return "TOKEN_KEYWORD_ONE_LINE_COMMENT";
		case TOKEN_KEYWORD_MULTI_LINE_COMMENT: return "TOKEN_KEYWORD_MULTI_LINE_COMMENT";
		case TOKEN_KEYWORD_IMPORT: return "TOKEN_KEYWORD_IMPORT";
		case TOKEN_KEYWORD_STATIC: return "TOKEN_KEYWORD_STATIC";
		case TOKEN_KEYWORD_CLASS: return "TOKEN_KEYWORD_CLASS";
		case TOKEN_KEYWORD_THIS: return "TOKEN_KEYWORD_THIS";
		case TOKEN_KEYWORD_PUB: return "TOKEN_KEYWORD_PUB";
		case TOKEN_KEYWORD_PRIV: return "TOKEN_KEYWORD_PRIV";
		case TOKEN_KEYWORD_NEW: return "TOKEN_KEYWORD_NEW";
		case TOKEN_KEYWORD_OVERRIDE: return "TOKEN_KEYWORD_OVERRIDE";
		case TOKEN_KEYWORD_VIRTUAL: return "TOKEN_KEYWORD_VIRTUAL";
		case TOKEN_KEYWORD_EXPORT: return "TOKEN_KEYWORD_EXPORT";
		case TOKEN_KEYWORD_AS: return "TOKEN_KEYWORD_AS";

		case TOKEN_KEYWORD_TYPE_VOID: return "TOKEN_KEYWORD_TYPE_VOID";
		case TOKEN_KEYWORD_TYPE: return "TOKEN_KEYWORD_TYPE";
		case TOKEN_KEYWORD_SUPER: return "TOKEN_KEYWORD_SUPER";

		case TOKEN_UNKNOWN: return "TOKEN_UNKNOWN";

		default: return "UNKNOWN_TOKEN_TYPE";
	}
}