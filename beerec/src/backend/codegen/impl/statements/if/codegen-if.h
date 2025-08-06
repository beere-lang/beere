#ifndef CODEGEN_IF_H
#define CODEGEN_IF_H

#include "../../../codegen.h"

extern int if_thens_count;
extern int if_elses_count;
extern int if_posts_count;

void generate_if(CodeGen* codegen, Node* node, AsmArea* area);

#endif