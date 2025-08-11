#ifndef CODEGEN_OP_H
#define CODEGEN_OP_H

#include "../../../codegen.h"

AsmReturn* generate_operation(CodeGen* codegen, Node* node, AsmArea* area, int force_reg, int prefer_second, int prefer_third, int argument_flag);

#endif