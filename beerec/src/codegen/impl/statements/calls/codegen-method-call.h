#ifndef CODEGEN_METHOD_CALL_H
#define CODEGEN_METHOD_CALL_H

#include "../../../codegen.h"

AsmReturn* generate_method_call(CodeGen* codegen, Node* node, AsmArea* area, int prefer_second, int argument_flag);

#endif