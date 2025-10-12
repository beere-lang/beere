#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "../../utils/logger/logger.h"
#include "../../utils/string/string.h"

static i32 is_digit(char ch);
static i32 is_alpha(char ch);
static i32 is_aldig(char ch);
static char get_char(Lexer* lexer);
static i32 is_unknown(Token* token);
static char* advance_char(Lexer* lexer);
static char* consume_char(Lexer* lexer);
static Lexer* setup_lexer(u32 max_index, str content);
static void add_token(Token* token, u32* tokens_length, Token* tokens);

// ==---------------------------------- Core --------------------------------------== \\

// Lida com um pedaço que pode ser considerado uma palavra com chars ("a-Z", "_", "0-9") e
// seta os valores das referencias 'start_ref', 'end_ref' e 'length_ref' pra uso posterior.
// Retorna NULL caso não seja um identifier.
static str handle_identifier(Lexer* lexer, char** start_ref, char** end_ref, u32* length_ref)
{
	if (!is_alpha(get_char(lexer)))
	{
		return NULL;
	}

	u32 length = 0;
	char* start = lexer->current;
	char* end = NULL;
	
	while (get_char(lexer) == '_' || is_aldig(get_char(lexer)))
	{
		end = consume_char(lexer);
		++length;
	}
	
	*start_ref = start;
	*end_ref = end;
	*length_ref = length;

	str temp =  strndup(start, length);
	
	return temp;
}

// Verifica se a palavra 'identifier' se é igual ao nome de um tipo.
// Retorna desconhecido 'TOKEN_UNKNOWN' caso não seja.
static Token handle_types(Lexer* lexer, str identifier, char* start)
{
	Token token = CREATE_TOKEN(TOKEN_UNKNOWN, start, 0);

	if (strncmp(identifier, "int", 3) == 0)
	{
		token.type = TOKEN_TYPE_INT;
		token.length = 3;
	}
	else if (strncmp(identifier, "float", 5) == 0)
	{
		token.type = TOKEN_TYPE_FLOAT;
		token.length = 5;
	}
	else if (strncmp(identifier, "double", 6) == 0)
	{
		token.type = TOKEN_TYPE_DOUBLE;
		token.length = 6;
	}
	else if (strncmp(identifier, "char", 4) == 0)
	{
		token.type = TOKEN_TYPE_CHAR;
		token.length = 4;
	}
	else if (strncmp(identifier, "string", 6) == 0)
	{
		token.type = TOKEN_TYPE_STRING;
		token.length = 6;
	}
	else if (strncmp(identifier, "bool", 4) == 0)
	{
		token.type = TOKEN_TYPE_BOOL;
		token.length = 4;
	}

	return token;
}

// Verifica se a palavra 'identifier' se é igual ao nome de uma keyword.
// Retorna desconhecido 'TOKEN_UNKNOWN' caso não seja.
static Token handle_keywords(Lexer* lexer, str identifier, char* start)
{
	Token token = CREATE_TOKEN(TOKEN_UNKNOWN, start, 0);
	
	if (strncmp(identifier, "if", 2) == 0)
	{
		token.type = TOKEN_KEYWORD_IF;
		token.length = 2;
	}
	else if (strncmp(identifier, "else", 4) == 0)
	{
		token.type = TOKEN_KEYWORD_ELSE;
		token.length = 4;
	}
	else if (strncmp(identifier, "continue", 8) == 0)
	{
		token.type = TOKEN_KEYWORD_CONTINUE;
		token.length = 8;
	}
	else if (strncmp(identifier, "break", 5) == 0)
	{
		token.type = TOKEN_KEYWORD_BREAK;
		token.length = 5;
	}
	else if (strncmp(identifier, "class", 5) == 0)
	{
		token.type = TOKEN_KEYWORD_CLASS;
		token.length = 5;
	}
	else if (strncmp(identifier, "as", 2) == 0)
	{
		token.type = TOKEN_KEYWORD_AS;
		token.length = 2;
	}
	else if (strncmp(identifier, "import", 6) == 0)
	{
		token.type = TOKEN_KEYWORD_IMPORT;
		token.length = 6;
	}
	else if (strncmp(identifier, "static", 6) == 0)
	{
		token.type = TOKEN_KEYWORD_STATIC;
		token.length = 6;
	}
	else if (strncmp(identifier, "super", 5) == 0)
	{
		token.type = TOKEN_KEYWORD_SUPER;
		token.length = 5;
	}
	else if (strncmp(identifier, "return", 6) == 0)
	{
		token.type = TOKEN_KEYWORD_RETURN;
		token.length = 6;
	}
	else if (strncmp(identifier, "let", 3) == 0)
	{
		token.type = TOKEN_KEYWORD_LET;
		token.length = 3;
	}
	else if (strncmp(identifier, "fn", 2) == 0)
	{
		token.type = TOKEN_KEYWORD_FN;
		token.length = 2;
	}
	else if (strncmp(identifier, "const", 5) == 0)
	{
		token.type = TOKEN_KEYWORD_CONST;
		token.length = 5;
	}
	else if (strncmp(identifier, "virtual", 7) == 0)
	{
		token.type = TOKEN_KEYWORD_VIRTUAL;
		token.length = 7;
	}
	else if (strncmp(identifier, "override", 8) == 0)
	{
		token.type = TOKEN_KEYWORD_OVERRIDE;
		token.length = 8;
	}
	else if (strncmp(identifier, "public", 6) == 0)
	{
		token.type = TOKEN_KEYWORD_PUBLIC;
		token.length = 6;
	}
	else if (strncmp(identifier, "private", 7) == 0)
	{
		token.type = TOKEN_KEYWORD_PRIVATE;
		token.length = 7;
	}
	else if (strncmp(identifier, "for", 3) == 0)
	{
		token.type = TOKEN_KEYWORD_FOR;
		token.length = 3;
	}
	else if (strncmp(identifier, "while", 5) == 0)
	{
		token.type = TOKEN_KEYWORD_WHILE;
		token.length = 5;
	}
	else if (strncmp(identifier, "void", 4) == 0)
	{
		token.type = TOKEN_KEYWORD_VOID;
		token.length = 4;
	}

	return token;
}

// Verifica se o char atual é algum char especifico ("()", "*", "-", "+", etc).
// Retorna desconhecido 'TOKEN_UNKNOWN' caso não seja.
static Token handle_chars(Lexer* lexer)
{
	Token token = CREATE_TOKEN(TOKEN_UNKNOWN, lexer->current, 1);

	char ch = get_char(lexer);
	char next = *(lexer->current + 1);

	if (ch == '+')
	{
		if (next == '+')
		{
			token.type = TOKEN_OPERATOR_INCREMENT;
			token.length = 2;
		}
		else if (next == '=')
		{
			token.type = TOKEN_OPERATOR_PLUS_EQUALS;
			token.length = 2;
		}
		else
		{
			token.type = TOKEN_CHAR_PLUS;
		}
	}
	else if (ch == '-')
	{
		if (next == '-')
		{
			token.type = TOKEN_OPERATOR_DECREMENT;
			token.length = 2;
		}
		else if (next == '=')
		{
			token.type = TOKEN_OPERATOR_MINUS_EQUALS;
			token.length = 2;
		}
		else
		{
			token.type = TOKEN_CHAR_MINUS;
		}
	}
	else if (ch == '*')
	{
		if (next == '=')
		{
			token.type = TOKEN_OPERATOR_TIMES_EQUALS;
			token.length = 2;
		}
		else
		{
			token.type = TOKEN_CHAR_ASTERISK;
		}
	}
	else if (ch == '/')
	{
		if (next == '=')
		{
			token.type = TOKEN_OPERATOR_DIVIDED_EQUALS;
			token.length = 2;
		}
		else
		{
			token.type = TOKEN_CHAR_DASH;
		}
	}
	else if (ch == '|')
	{
		if (next == '|')
		{
			token.type = TOKEN_OPERATOR_LOG_OR;
			token.length = 2;
		}
	}
	else if (ch == '&')
	{
		if (next == '&')
		{
			token.type = TOKEN_OPERATOR_LOG_AND;
			token.length = 2;
		}
	}
	else if (ch == '=')
	{
		if (next == '=')
		{
			token.type = TOKEN_OPERATOR_EQUALS;
			token.length = 2;
		}
		else
		{
			token.type = TOKEN_CHAR_EQUALS;
		}
	}
	else if (ch == '(')
	{
		token.type = TOKEN_CHAR_LEFT_PAREN;
	}
	else if (ch == ')')
	{
		token.type = TOKEN_CHAR_RIGHT_PAREN;
	}
	else if (ch == '{')
	{
		token.type = TOKEN_CHAR_LEFT_BRACE;
	}
	else if (ch == '}')
	{
		token.type = TOKEN_CHAR_RIGHT_BRACE;
	}
	else if (ch == '[')
	{
		token.type = TOKEN_CHAR_LEFT_BRACKET;
	}
	else if (ch == ']')
	{
		token.type = TOKEN_CHAR_RIGHT_BRACKET;
	}
	else if (ch == '.')
	{
		token.type = TOKEN_CHAR_DOT;
	}
	else if (ch == ':')
	{
		token.type = TOKEN_CHAR_COLON;
	}

	return token;
}

// Verifica se vai vir uma constant boolean em especifico.
// Retorna desconhecido 'TOKEN_UNKNOWN' caso não seja.
static Token handle_bool_constants(Lexer* lexer, str identifier, char* start)
{
	Token token = CREATE_TOKEN(TOKEN_UNKNOWN, start, 0);

	if (strncmp(identifier, "true", 4) == 0)
	{
		token.type = TOKEN_CONSTANT_BOOL;
		token.constant_bool.value = 1;
		token.length = 4;
	}
	else if (strncmp(identifier, "false", 5) == 0)
	{
		token.type = TOKEN_CONSTANT_BOOL;
		token.constant_bool.value = 0;
		token.length = 5;
	}

	return token;
}

// Verifica se vai vir uma constant de numero.
// Retorna desconhecido 'TOKEN_UNKNOWN' caso não seja.
static Token handle_number_constants(Lexer* lexer)
{
	Token token = CREATE_TOKEN(TOKEN_UNKNOWN, lexer->current, 0);

	char ch = get_char(lexer);

	if (is_digit(ch))
	{
		i32 decimal = 0;

		token.type = TOKEN_CONSTANT_NUMBER;
		token.constant_number.type = NUMBER_TYPE_INT;

		u32 length = 0;
		
		while (is_digit(get_char(lexer)) || get_char(lexer) == '.')
		{
			if (get_char(lexer) == '.')
			{
				if (decimal)
				{
					log_error("Found 2 '.' in number constant...");
					exit(1);
				}

				decimal = 1;
			}

			advance_char(lexer);
			++length;
		}

		token.length = length;
		
		if (decimal)
		{
			token.constant_number.type = NUMBER_TYPE_DOUBLE;
		}
		
		if (get_char(lexer) == 'F' || get_char(lexer) == 'f')
		{
			token.constant_number.type = NUMBER_TYPE_FLOAT;
			advance_char(lexer);
		}
		else if (get_char(lexer) == 'D' || get_char(lexer) == 'd')
		{
			token.constant_number.type = NUMBER_TYPE_DOUBLE;
			advance_char(lexer);
		}

		str str_value = strndup(token.start, length);

		switch (token.constant_number.type)
		{
			case NUMBER_TYPE_INT:
			{
				token.constant_number.int_value = atoi(str_value);
				break;
			}

			case NUMBER_TYPE_FLOAT:
			{
				token.constant_number.float_value = atof(str_value);
				break;
			}

			case NUMBER_TYPE_DOUBLE:
			{
				char* end = NULL;
				token.constant_number.double_value = strtod(str_value, &end);

				if (end == NULL || end == token.start)
				{
					log_error("Failed to convert number constant (string) to double...");
					exit(1);
				}

				break;
			}
		}
	}

	return token;
}

// Verifica se vai vir uma constant de string.
// Retorna desconhecido 'TOKEN_UNKNOWN' caso não seja.
static Token handle_string_constants(Lexer* lexer) /** TODO: adicionar chars que usam '\'  ('\n', '0', etc).  */
{
	Token token = CREATE_TOKEN(TOKEN_UNKNOWN, NULL, 0);

	char ch = get_char(lexer);

	if (ch == '\"')
	{
		u32 length = 0;

		token.type = TOKEN_CONSTANT_STRING;

		advance_char(lexer);
		token.start = lexer->current;

		while (get_char(lexer) != '\"')
		{
			if (get_char(lexer) == '\n' || get_char(lexer) == '\0')
			{
				log_error("Not terminated string constant found...");
				exit(1);
			}

			++length;
		}

		token.constant_string.value = strndup(token.start, length);

		token.length = length;
		
		advance_char(lexer);
	}

	return token;
}

// Verifica se vai vir uma constant de char.
// Retorna desconhecido 'TOKEN_UNKNOWN' caso não seja.
static Token handle_char_constants(Lexer* lexer) /** TODO: adicionar chars que usam '\'  ('\n', '0', etc).  */
{
	Token token = CREATE_TOKEN(TOKEN_UNKNOWN, NULL, 0);

	char ch = get_char(lexer);

	if (ch == '\'')
	{
		u32 length = 0;

		token.type = TOKEN_CONSTANT_CHAR;

		advance_char(lexer);
		token.start = lexer->current;

		while (get_char(lexer) != '\'')
		{
			if (get_char(lexer) == '\n' || get_char(lexer) == '\0')
			{
				log_error("Not terminated char constant found...");
				exit(1);
			}

			token.constant_char.value = *(consume_char(lexer));
			++length;
		}

		if (length > 1)
		{
			log_error("Found a char constant with more than 1 char...");
			exit(1);
		}

		token.length = length;
		
		advance_char(lexer);
	}

	return token;
}

Lexer* tokenize_module(Module* module, const u32 max_index, str content)
{
	Lexer* lexer = setup_lexer(max_index, content);
	u32 tokens_length = 0;
	
	while (1)
	{
		const char ch = get_char(lexer);

		if (ch == '\r' || ch == ' ' || ch == '\t')
		{
			advance_char(lexer);
			continue;
		}

		if (ch == '\n')
		{
			Token token = CREATE_TOKEN(TOKEN_END_LINE, lexer->current, 1);
			add_token(&token, &tokens_length, lexer->tokens);
			
			advance_char(lexer);
			continue;
		}

		if (ch == '\0' || ch == EOF)
		{
			Token token = CREATE_TOKEN(TOKEN_END_SOURCE, lexer->current, 1);
			add_token(&token, &tokens_length, lexer->tokens);

			log_warning("Found source end at index: %d...", lexer->index);

			break;
		}

		char* start = NULL;
		char* end = NULL;
		u32 length = 0;

		str identifier = handle_identifier(lexer, &start, &end, &length);

		if (identifier != NULL)
		{
			Token token_type = handle_types(lexer, identifier, start);
			
			if (!is_unknown(&token_type))
			{
				add_token(&token_type, &tokens_length, lexer->tokens);
				continue;
			}
	
			Token token_keyword = handle_keywords(lexer, identifier, start);
	
			if (!is_unknown(&token_keyword))
			{
				add_token(&token_keyword, &tokens_length, lexer->tokens);
				continue;
			}

			Token token_bool = handle_bool_constants(lexer, identifier, start);

			if (!is_unknown(&token_bool))
			{
				add_token(&token_bool, &tokens_length, lexer->tokens);
				continue;
			}
		}

		Token token_char = handle_chars(lexer);
	
		if (!is_unknown(&token_char))
		{
			for (i32 i = 0; i < token_char.length; i++)
			{
				advance_char(lexer);
			}

			add_token(&token_char, &tokens_length, lexer->tokens);
			continue;
		}

		Token number_constant = handle_number_constants(lexer);

		if (!is_unknown(&number_constant))
		{
			add_token(&number_constant, &tokens_length, lexer->tokens);
			continue;
		}

		Token str_constant = handle_string_constants(lexer);

		if (!is_unknown(&str_constant))
		{
			add_token(&str_constant, &tokens_length, lexer->tokens);
			continue;
		}

		Token char_constant = handle_char_constants(lexer);

		if (!is_unknown(&char_constant))
		{
			add_token(&char_constant, &tokens_length, lexer->tokens);
			continue;
		}

		if (identifier != NULL)
		{
			log_warning("Adding a identifier: \"%s\"", identifier);
			
			Token token = CREATE_TOKEN(TOKEN_IDENTIFIER, start, length);
			
			add_token(&token, &tokens_length, lexer->tokens);
			continue;
		}
		else
		{
			log_error("Invalid token found, exiting...");
			exit(1);
		}
	}

	return lexer;
}

// ==--------------------------------- Utils --------------------------------------== \\

// Checa se um char 'ch' é um char de numero.
static i32 is_digit(char ch)
{
	return ch >= '0' && ch <= '9';
}

// Checa se um char 'ch' é um char alpha numerico ou um underline (usado em identifiers).
static i32 is_alpha(char ch)
{
	return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_';
}

// Checa se um char 'ch' é um digito ou um char alpha numerico.
// Veja 'is_digit' e 'is_alpha' pra entender melhor.
static i32 is_aldig(char ch)
{
	return is_digit(ch) || is_alpha(ch);
}

// Retorna se um token é do tipo desconhecido 'TOKEN_UNKNOWN'.
static i32 is_unknown(Token* token)
{
	return token->type == TOKEN_UNKNOWN;
}

// Adiciona o token 'token' a array de tokens 'tokens' e incrementa o valor
// que o pointer 'tokens_length' aponta.
static void add_token(Token* token, u32* tokens_length, Token* tokens)
{
	tokens[(*tokens_length)++] = *token;
}

// Avança pro proximo char do content em 'lexer', ja retornando o proximo char e
// dando erro caso acesse o conteúdo fora do 'max_index'
static char* advance_char(Lexer* lexer)
{
	if (lexer == NULL)
	{
		log_error("Invalid lexer found is NULL...");
		exit(1);
	}
	
	++lexer->current;
	++lexer->index;

	if (lexer->index > lexer->max_index)
	{
		log_error("Failed to tokenize module, read outside content memory space...");
		exit(1);
	}

	return lexer->current;
}

// Retorna o char atual, avança pro proximo char do content em 'lexer' e
// dando erro caso acesse o conteúdo fora do 'max_index'
static char* consume_char(Lexer* lexer)
{
	if (lexer == NULL)
	{
		log_error("Invalid lexer found is NULL...");
		exit(1);
	}
	
	char* ch = lexer->current;

	++lexer->current;
	++lexer->index;
	
	if (lexer->index > lexer->max_index)
	{
		log_error("Failed to tokenize module, read outside content memory space...");
		exit(1);
	}

	return ch;
}

// Retorna o char em que o ponteiro 'current' aponta.
static char get_char(Lexer* lexer)
{
	return *lexer->current;
}

// Da setup na structure do lexer (alocado na heap).
static Lexer* setup_lexer(const u32 max_index, str content)
{
	Lexer* lexer = calloc(1, sizeof(Lexer));

	lexer->content = content;
	lexer->current = content;

	lexer->index = 3;

	lexer->max_index = max_index;
	lexer->tokens = calloc(LEXER_TOKENS_BUFFER_SIZE, sizeof(Token));
	
	return lexer;
}

// ==--------------------------- Memory Management ---------------------------------== \\

void free_lexer(Lexer* lexer)
{
	if (lexer->content != NULL)
	{
		free(lexer->content);
	}

	free(lexer->tokens);
	free(lexer);
}