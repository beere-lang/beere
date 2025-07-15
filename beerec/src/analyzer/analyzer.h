#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "../modules/modules.h"

typedef struct Analyzer Analyzer;

Symbol* analyzer_find_symbol_from_scope(const char* identifier, SymbolTable* scope, int is_variable, int is_function, int is_class, int is_module);
Type* analyzer_return_type_of_expression(Module* module, Node* expression, SymbolTable* scope, NodeList* args, int member_access, int* direct);
int analyzer_get_type_size(Type* type, SymbolTable* scope);
void analyzer_global_analyze(Module* module, Node* node);
void analyzer_init(Module* module, Node** node_list);

#endif