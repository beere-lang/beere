#ifndef CODEGEN_CAST_H
#define CODEGEN_CAST_H

#include "../../../codegen.h"

AsmReturn* generate_cast(CodeGen* codegen, Node* node, AsmArea* area, int prefer_second, int argument_flag);

#endif