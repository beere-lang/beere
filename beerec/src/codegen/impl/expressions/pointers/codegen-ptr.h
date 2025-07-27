#ifndef CODEGEN_PTR_H
#define CODEGEN_PTR_H

#include "../../../codegen.h"

AsmReturn* generate_reference(CodeGen* codege, Node* node, AsmArea* area, int force_reg, int prefer_second);
AsmReturn* generate_dereference(CodeGen* codege, Node* node, AsmArea* area, int force_reg, int prefer_second);

#endif