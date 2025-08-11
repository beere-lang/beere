#ifndef CODEGEN_EXPR_H
#define CODEGEN_EXPR_H

#include "../../codegen.h"

AsmReturn* generate_expression(CodeGen* codegen, Node* node, AsmArea* area, int force_reg);
AsmReturn* generate_this(CodeGen* codegen, AsmArea* area, int force_reg);

#endif