#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "../modules/modules.h"

typedef struct Analyzer Analyzer;

void analyzer_global_analyze(Module* module, Node* node);
void analyzer_init(Module* module, Node** node_list);

#endif