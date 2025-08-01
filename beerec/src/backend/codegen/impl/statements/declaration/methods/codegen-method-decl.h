#ifndef CODEGEN_METHOD_DECL_H
#define CODEGEN_METHOD_DECL_H

#include "../../../../codegen.h"

void generate_method_declaration(CodeGen* codegen, Node* node, AsmArea* area, int is_class, char* class_method_name);

#endif