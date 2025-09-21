#ifndef DOMINANCE_FRONTIER_H
#define DOMINANCE_FRONTIER_H

#include "../control-flow.h"

CFBlock** generate_dominance_frontier(CFBlock** lblocks, const int length, int* idom);

#endif
