#ifndef CODEGEN_MBR_ACCESS_H
#define CODEGEN_MBR_ACCESS_H

#include "../../../codegen.h"

AsmReturn* generate_member_access(CodeGen* codegen, Node* node, AsmArea* area, int force_reg, int prefer_second, int argument_flag);
AsmReturn* generate_class_direct_access(CodeGen* codegen, Node* node);
AsmReturn* generate_super(CodeGen* codegen, AsmArea* area, int force_reg);

#endif