#ifndef CODEGEN_WHILE_H
#define CODEGEN_WHILE_H

#include "../../../codegen.h"

extern int whiles_count;

void generate_while(CodeGen* codegen, Node* node, AsmArea* area);

#endif