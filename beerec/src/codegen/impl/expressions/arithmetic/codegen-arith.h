#ifndef CODEGEN_ARITH_H
#define CODEGEN_ARITH_H

#include "../../../codegen.h"

AsmReturn* generate_decrement_operation(CodeGen* codegen, Node* node, AsmArea* area, Flag flag);
AsmReturn* generate_increment_operation(CodeGen* codegen, Node* node, AsmArea* area, Flag flag);
AsmReturn* generate_minus_operation(CodeGen* codegen, Node* node, AsmArea* area, Flag flag);
AsmReturn* generate_plus_operation(CodeGen* codegen, Node* node, AsmArea* area, Flag flag);

#endif