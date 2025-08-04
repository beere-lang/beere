#ifndef CODEGEN_EXPR_H
#define CODEGEN_EXPR_H

#include "../../codegen.h"

AsmReturn* generate_expression(CodeGen* codegen, Node* node, AsmArea* area, int force_reg, int prefer_second, int argument_flag);
AsmReturn* generate_this(CodeGen* codegen);

#endif