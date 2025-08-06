#ifndef CODEGEN_IF_H
#define CODEGEN_IF_H

#include "../../../codegen.h"

int if_counts;
int if_thens_count;
int if_elses_count;

void generate_if_statement(CodeGen* codegen, Node* node, AsmArea* area);

#endif