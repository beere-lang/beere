#ifndef CODEGEN_EXPR_H
#define CODEGEN_EXPR_H

#include "../../codegen.h"

AsmReturn* generate_expr(CodeGen* codegen, Node* node, AsmArea* area, Flag flag);

#endif