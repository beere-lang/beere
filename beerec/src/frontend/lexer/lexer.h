#ifndef LEXER_H
#define LEXER_H

#include "../../../data/data.h"
#include "../modules/modules.h"
#include "../structure/tokens/tokens.h"

typedef struct Lexer Lexer;

// Tamanho do buffer em que os tokens são guardados após o module ser tokenizado.
#define LEXER_TOKENS_BUFFER_SIZE 2048

// Structure do lexer, usado mais pra organizar
// o output e o gerenciamento do content.
struct Lexer
{
	str	 content;

	char*	 current; // ponteiro pro char atual
	u32	 index;   // index do char atual

	// index maximo que o 'index' pode chegar
	// (geralmente é o tamanho do buffer 'content')
	u32	 max_index;

	Token* tokens; // output
	u32	 tokens_length;
};

// Tokeniza todo o conteúdo 'content' (texto plano) em tokens, que dão
// um signicado aos pedaços do conteúdo do module 'content'.
Lexer* tokenize_module(Module* module, const u32 max_index, str content);

// Da free na structure do lexer 'lexer' e seu conteúdo.
void	 free_lexer(Lexer* lexer);

#endif