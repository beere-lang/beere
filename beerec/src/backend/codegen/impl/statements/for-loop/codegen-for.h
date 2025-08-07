#ifndef CODEGEN_FOR_H
#define CODEGEN_FOR_H

#include "../../../codegen.h"

extern int fors_count;

void generate_for(CodeGen* codegen, Node* node, AsmArea* area);

#endif