#ifndef COMPILER_H
#define COMPILER_H

#include "../frontend/modules/modules.h"

Token* tokenize_code(char* content, int lexer_debug);
void analyze_nodes(Module* module, Node** node_list);
Module* compile(ModuleHandler* handler, char* file_path, char* lexer_flag);
Node** parse_tokens(Token* tokens);

#endif