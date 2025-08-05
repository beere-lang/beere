#ifndef CODEGEN_COMPARATIVE_H
#define CODEGEN_COMPARATIVE_H

#include "../../../codegen.h"

AsmReturn* generate_is_greater_equals_operation(CodeGen* codegen, AsmReturn* lreg, AsmReturn* rreg, AsmArea* area, int prefer_second, int argument_flag);
AsmReturn* generate_is_less_equals_operation(CodeGen* codegen, AsmReturn* lreg, AsmReturn* rreg, AsmArea* area, int prefer_second, int argument_flag);
AsmReturn* generate_is_not_equals_operation(CodeGen* codegen, AsmReturn* lreg, AsmReturn* rreg, AsmArea* area, int prefer_second, int argument_flag);
AsmReturn* generate_is_greater_operation(CodeGen* codegen, AsmReturn* lreg, AsmReturn* rreg, AsmArea* area, int prefer_second, int argument_flag);
AsmReturn* generate_is_equals_operation(CodeGen* codegen, AsmReturn* lreg, AsmReturn* rreg, AsmArea* area, int prefer_second, int argument_flag);
AsmReturn* generate_is_less_operation(CodeGen* codegen, AsmReturn* lreg, AsmReturn* rreg, AsmArea* area, int prefer_second, int argument_flag);
AsmReturn* generate_and_operation(CodeGen* codegen, AsmReturn* lreg, AsmReturn* rreg, AsmArea* area, int prefer_second, int argument_flag);
AsmReturn* generate_or_operation(CodeGen* codegen, AsmReturn* lreg, AsmReturn* rreg, AsmArea* area, int prefer_second, int argument_flag);

#endif