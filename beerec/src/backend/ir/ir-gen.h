#ifndef IR_GEN_H
#define IR_GEN_H

#include "../structure/ir/ir-nodes.h"
#include "../../frontend/structure/ast/nodes/nodes.h"

IRNode** generate_ir_nodes(ASTNode** nodes, const int length);
void free_nodes(IRNode** nodes, int length);
Type* get_operation_type(IRNode* node);

#endif
