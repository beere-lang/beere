#ifndef IR_GEN_H
#define IR_GEN_H

#include "../structure/ir/ir-nodes.h"
#include "../../frontend/structure/ast/nodes/nodes.h"

void dump_module(IRNode** nodes, const unsigned int nodes_length);
IRNode** generate_ir_nodes(ASTNode** nodes, const unsigned int length);
void free_nodes(IRNode** nodes, int length);

#endif
