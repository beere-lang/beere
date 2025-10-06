#ifndef DOMINANCE_FRONTIER_H
#define DOMINANCE_FRONTIER_H

#include "../control-flow.h"
#include "../dominator-tree/dominator-tree.h"

DList** generate_dominance_frontier(CFBlock** lblocks, const int tlength, int* idominators, DTBlock** blocks, int size);

#endif
