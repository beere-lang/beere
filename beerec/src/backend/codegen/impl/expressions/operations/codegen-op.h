#ifndef CODEGEN_OP_H
#define CODEGEN_OP_H

#include "../../../codegen.h"

AsmReturn* generate_operation(CodeGen* codegen, Node* node, AsmArea* area, int force_reg);

#endif