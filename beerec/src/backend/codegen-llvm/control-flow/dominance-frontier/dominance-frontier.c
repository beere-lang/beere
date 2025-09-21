#include <stdlib.h>

#include "dominance-frontier.h"
#include "../control-flow.h"

int get_block_number(CFBlock** blocks, const int length, CFBlock* block)
{
	for (int i = 0; i < length; i++)
	{
		CFBlock* curr = blocks[i];

		if (block != curr)
		{
			continue;
		}

		return i;
	}

	return -1;
}

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
			const int index = get_block_number(lblocks, tlength, succ);

			if (idominators[index] == i)
			{
				continue;
			}

			frontier[i] = succ;
		}

		/**
		 * TODO: adicionar o segundo loop que vai checar
		 */
	}

	return frontier;
}
