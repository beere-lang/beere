#ifndef LLVM_GEN_H
#define LLVM_GEN_H

#include <llvm-c/Core.h>
#include "../../structure/ir/ir-nodes.h"

void generate_llvm_from_node(const LLVMModuleRef module, const LLVMBuilderRef llvm, IRNode *node);

#endif