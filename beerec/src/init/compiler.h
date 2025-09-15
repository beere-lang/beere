#ifndef COMPILER_H
#define COMPILER_H

#include "../frontend/modules/modules.h"

Token* tokenize_code(char* content, int lexer_debug);
void analyze_nodes(Module* module, ASTNode** node_list);
Module* compile(ModuleHandler* handler, char* file_path, char* lexer_flag);
ASTNode** parse_tokens(Token* tokens);

#endif