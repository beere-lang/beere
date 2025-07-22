#ifndef CODEGEN_REF_H
#define CODEGEN_REF_H

#include "../../../codegen.h"

AsmReturn* generate_variable_reference(CodeGen* codegen, Node* node, AsmArea* area, int force_reg, int prefer_second);

#endif