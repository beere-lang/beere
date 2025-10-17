#ifndef IR_GEN_H
#define IR_GEN_H

#include "../../frontend/structure/ast/nodes/nodes.h"
#include "../structure/ir/ir-nodes.h"

// Func entry block label.
#define ENTRY_BLOCK_LABEL			".entry"

// Func label prefix.
#define FUNC_LABEL_PREFIX			".fn_"

// While statement block labels prefixes.
#define WHILE_COND_LABEL_PREFIX		".while_cond_"
#define WHILE_THEN_LABEL_PREFIX		".while_then_"
#define WHILE_POST_LABEL_PREFIX		".while_post_"

// If statement blocks label prefixes.
#define IF_THEN_LABEL_PREFIX			".if_then_"
#define IF_ELSE_LABEL_PREFIX			".if_else_"
#define IF_POST_LABEL_PREFIX			".if_post_"

// Capacidade inicial de blocks em funcs.
#define FUNC_START_BLOCKS_CAPACITY		8

// Capacidade inicial de nodes em blocks.
#define BLOCK_START_INSTRUCTIONS_CAPACITY 16

// Capacidade incial de methods e fields em classes.
#define CLASSES_START_FIELDS_CAPACITY	8
#define CLASSES_START_METHODS_CAPACITY	8

void	   dump_module(IRNode** nodes, const unsigned int nodes_length);
IRNode** generate_ir_nodes(ASTNode** nodes, const unsigned int length);
void	   free_nodes(IRNode** nodes, int length);

#endif
