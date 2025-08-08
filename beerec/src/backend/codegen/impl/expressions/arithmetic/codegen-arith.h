#ifndef CODEGEN_ARITH_H
#define CODEGEN_ARITH_H

#include "../../../codegen.h"

AsmReturn* generate_minus_operation(CodeGen* codegen, AsmReturn* left_value, AsmReturn* right_value, AsmArea* area, int argument_flag);
AsmReturn* generate_plus_operation(CodeGen* codegen, AsmReturn* left_value, AsmReturn* right_value, AsmArea* area, int argument_flag);
AsmReturn* generate_multiply_operation(CodeGen* codegen, AsmReturn* lreg, AsmReturn* rreg, AsmArea* area);
AsmReturn* generate_div_operation(CodeGen* codegen, AsmReturn* lreg, AsmReturn* rreg, AsmArea* area);
AsmReturn* generate_decrement_operation(CodeGen* codegen, AsmReturn* left_value, AsmArea* area);
AsmReturn* generate_increment_operation(CodeGen* codegen, AsmReturn* left_value, AsmArea* area);

#endif