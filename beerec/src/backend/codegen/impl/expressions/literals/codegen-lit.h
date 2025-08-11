#ifndef CODEGEN_LIT_H
#define CODEGEN_LIT_H

#include "../../../codegen.h"

AsmReturn* generate_literal(CodeGen* codegen, Node* node, AsmArea* area, int force_reg, int prefer_second, int prefer_third, int argument_flag);

#endif