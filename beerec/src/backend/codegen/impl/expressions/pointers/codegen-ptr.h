#ifndef CODEGEN_PTR_H
#define CODEGEN_PTR_H

#include "../../../codegen.h"

AsmReturn* generate_reference(CodeGen* codegen, Node* node, AsmArea* area, int force_reg, int prefer_second, int prefer_third);
AsmReturn* generate_dereference(CodeGen* codegen, Node* node, AsmArea* area, int force_reg, int prefer_second, int prefer_third);

#endif