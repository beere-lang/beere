#include <stdlib.h>

#include "dominance-frontier.h"
#include "../control-flow.h"

CFBlock** generate_dominance_frontier(CFBlock** lblocks, const int tlength, int* idominators)
{
	CFBlock** frontier = malloc(sizeof(CFBlock*) * tlength);

	for (int i = 0; i < tlength; i++)
	{
		CFBlock* block = lblocks[i];

		const int slength = block->successors->length;

		for (int j = 0; j < slength; j++)
		{
			CFBlock* succ = block->successors->elements[j];
			const int index = succ->dt_index;

			if (idominators[index] == i)
			{
				continue;
			}

			frontier[i] = succ;
		}

		/**
		 * TODO: adicionar o segundo loop que vai checar fronteiras indiretas
		 */
	}

	return frontier;
}
