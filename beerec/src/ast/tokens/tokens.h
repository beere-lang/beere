#ifndef TOKEN_H
#define TOKEN_H

#include <stdio.h>

typedef enum
{
	TYPE_CHAR, // 'a'
	TYPE_STRING, // "Hello, World!"
	TYPE_INT, // 10
	TYPE_FLOAT, // 10.5F
	TYPE_DOUBLE, // 10.5
	TYPE_BOOL, // true, false
	TYPE_PTR, // int*, double*, etc...
	TYPE_ANY_PTR, // any, needs to be a pointer... (fuck js)
	TYPE_ARRAY,

	TYPE_CLASS,
	
	TYPE_VOID, // void, for functions

	TYPE_NULL, // null,
	
	TYPE_UNKNOWN
} VarType;

typedef enum
{
	TOKEN_IDENTIFIER, // a-z, A-Z, _, 0-9 (should be after first char)

	TOKEN_OPERATOR_ASSIGN, // =

	TOKEN_CHAR_COLON, // :
	TOKEN_CHAR_COMMA, // ,
	TOKEN_CHAR_SEMI_COLON, // ;
	TOKEN_CHAR_OPEN_BRACE, // {
	TOKEN_CHAR_CLOSE_BRACE, // }
	TOKEN_CHAR_OPEN_PAREN, // (
	TOKEN_CHAR_CLOSE_PAREN, // )
	TOKEN_CHAR_DOUBLE_QUOTE, // "
	TOKEN_CHAR_STAR, // *
	TOKEN_CHAR_OPEN_BRACKET, // [
	TOKEN_CHAR_CLOSE_BRACKET, // ]

	TOKEN_OPERATOR_DIVIDED, // /
	TOKEN_OPERATOR_PLUS, // +
	TOKEN_OPERATOR_MINUS, // -
	TOKEN_OPERATOR_PLUS_EQUALS, // +=
	TOKEN_OPERATOR_MINUS_EQUALS, // -=
	TOKEN_OPERATOR_DIVIDED_EQUALS, // /=
	TOKEN_OPERATOR_TIMES_EQUALS, // *=
	TOKEN_OPERATOR_DECREMENT, // --
	TOKEN_OPERATOR_INCREMENT, // ++
	TOKEN_OPERATOR_AND, // &&
	TOKEN_OPERATOR_OR, // ||
	TOKEN_OPERATOR_GREATER, // >
	TOKEN_OPERATOR_LESS, // <
	TOKEN_OPERATOR_GREATER_EQUALS, // >=
	TOKEN_OPERATOR_LESS_EQUALS, // <=
	TOKEN_OPERATOR_EQUALS, // ==
	TOKEN_OPERATOR_NOT_EQUALS, // !=
	TOKEN_OPERATOR_ADRESS, // &
	TOKEN_OPERATOR_ACCESS_PTR, // ->
	TOKEN_OPERATOR_DOT, // .
	
	TOKEN_LITERAL_CHAR, // 'a'
	TOKEN_LITERAL_STRING, // "Hello, World!"
	TOKEN_LITERAL_INT, // 10
	TOKEN_LITERAL_FLOAT, // 10.5f
	TOKEN_LITERAL_DOUBLE, // 10.5
	TOKEN_LITERAL_BOOL, // true, false
	TOKEN_LITERAL_NULL, // null

	TOKEN_END_LINE, // \n
	TOKEN_END_SRC, // \0

	TOKEN_KEYWORD_RETURN, // return
	TOKEN_KEYWORD_IF, // if
	TOKEN_KEYWORD_ELSE, // else
	TOKEN_KEYWORD_FOR, // for
	TOKEN_KEYWORD_WHILE, // while
	TOKEN_KEYWORD_BREAK, // break
	TOKEN_KEYWORD_CONTINUE, // continue
	TOKEN_KEYWORD_EXTENDS, // extends
	TOKEN_KEYWORD_FUNCTION, // fn
	TOKEN_KEYWORD_LET, // let
	TOKEN_KEYWORD_CONST, // const
	TOKEN_KEYWORD_CASE, // case
	TOKEN_KEYWORD_SWITCH, // switch
	TOKEN_KEYWORD_ONE_LINE_COMMENT, // "// blablabla"
	TOKEN_KEYWORD_MULTI_LINE_COMMENT, // "/* blablabla */"
	TOKEN_KEYWORD_IMPORT, // import
	TOKEN_KEYWORD_STATIC, // static
	TOKEN_KEYWORD_CLASS, // class
	TOKEN_KEYWORD_THIS, // this
	TOKEN_KEYWORD_PUB, // pub
	TOKEN_KEYWORD_PRIV, // priv
	TOKEN_KEYWORD_NEW, // new
	TOKEN_KEYWORD_OVERRIDE, // override
	TOKEN_KEYWORD_VIRTUAL, // virtual
	TOKEN_KEYWORD_EXPORT, // export
	TOKEN_KEYWORD_AS, // as
	
	TOKEN_KEYWORD_TYPE_VOID, // void

	TOKEN_KEYWORD_TYPE, // int, string, char, float, bool, double
	TOKEN_KEYWORD_SUPER, // super

	TOKEN_UNKNOWN
} 
TokenType;

typedef struct
{
	TokenType token_type;

	const char* start;

	size_t length;
	size_t line;

	int negative;

	union // for types
	{
		VarType var_type;
		int bool_value;
		char* str_value;
	};
} 
Token;

#endif
