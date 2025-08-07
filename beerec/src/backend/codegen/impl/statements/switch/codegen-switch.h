#ifndef CODEGEN_SWITCH_H
#define CODEGEN_SWITCH_H

#include "../../../codegen.h"

extern int switches_case_count;

void generate_switch(CodeGen* codegen, Node* node, AsmArea* area);

#endif