#include <stdlib.h>
#include <string.h>

#include "dominance-frontier.h"
#include "../dominator-tree/dominator-tree.h"

#include "../control-flow.h"

#define DOMINANCE_FRONTIER_LIST_START_CAPACITY 8

DList** generate_dominance_frontier(CFBlock** lblocks, const int tlength, int* idominators, DTBlock** blocks, int size)
{
	DList** frontiers = malloc(sizeof(DList*) * tlength);
	memset(frontiers, 0, sizeof(DList*) * tlength);

	for (int i = 0; i < tlength; i++)
	{
		CFBlock* block = lblocks[i];
                frontiers[i] = create_list(DOMINANCE_FRONTIER_LIST_START_CAPACITY);

		const int slength = block->successors->length;

		for (int j = 0; j < slength; j++)
		{
			CFBlock* succ = block->successors->elements[j];
			const int index = succ->dt_index;

			if (idominators[index] == i)
			{
				continue;
			}

			add_element_to_list(frontiers[i], succ);
		}
        }

        for (int i = 0; i < tlength; i++)
        {
                CFBlock* block = lblocks[i];

		DTBlock* dtblock = blocks[block->dt_index];
                const int clength = dtblock->dominateds->length;

		for (int j = 0; j < clength; j++)
                {
                        DTBlock* child = dtblock->dominateds->elements[j];

                        DList* frontier = frontiers[child->block->dt_index];
                        const int flength = frontier->length;

                        for (int k = 0; k < flength; k++)
                        {
                                CFBlock* block = frontier->elements[k];

                                if (idominators[block->dt_index] == child->block->dt_index)
                                {
                                        continue;
                                }

                                add_element_to_list(frontiers[i], block);
                        }
                }

        }

	return frontiers;
}
