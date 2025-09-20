/**
 * TODO: iterar por todas as CFPathBlock e ver se o numero de repetiçoes de cada dominator é igual ao numero de paths possiveis,
 * caso seja igual, mantém o dominator (ignorar o proprio Block)
 */

#include <stdlib.h>

#include "dominator-tree.h"

static const int DT_BLOCK_LIST_DEFAULT_START_CAPACITY = 4;
static const int PATH_BLOCK_LIST_DEFAULT_START_CAPACITY = 4;

static CFPathBlockList* list = NULL;

static void generate_dominator_childs(DTBlock* parent);

static CFPathBlock* setup_path_block(CFBlock* block)
{
	CFPathBlock* path_block = malloc(sizeof(CFPathBlock));

	path_block->paths = 0;
	path_block->block = block;

	return path_block;
}

static CFPathBlockList* setup_path_block_list()
{
	CFPathBlockList* list = malloc(sizeof(CFPathBlockList));

	list->capacity = PATH_BLOCK_LIST_DEFAULT_START_CAPACITY;
	list->length = 0;
	list->elements = malloc(sizeof(CFPathBlock*) * PATH_BLOCK_LIST_DEFAULT_START_CAPACITY);

	return list;
}

static void add_element_to_path_block_list(CFPathBlockList* list, CFPathBlock* block)
{
	if (list->length >= list->capacity)
	{
		list->capacity *= 2;
		list->elements = realloc(list->elements, sizeof(CFPathBlock*) * list->capacity);
	}

	list->elements[list->length++] = block;
}

static void setup_global_list()
{
	list = setup_path_block_list();
}

CFPathBlock* find_cf_path_block(CFPathBlockList* cf_list, CFBlock* block)
{
	if (cf_list == NULL)
	{
		return NULL;
	}
	
	int length = cf_list->length;

	for (int i = 0; i < length; i++)
	{
		CFPathBlock* curr = cf_list->elements[i];

		if (curr == NULL)
		{
			continue;
		}

		if (curr->block != block)
		{
			continue;
		}

		return curr;
	}

	return NULL;
}

static DTBlockList* setup_dt_block_list()
{
	DTBlockList* list = malloc(sizeof(DTBlockList));

	list->capacity = DT_BLOCK_LIST_DEFAULT_START_CAPACITY;
	list->length = 0;
	list->elements = malloc(sizeof(DTBlock*) * DT_BLOCK_LIST_DEFAULT_START_CAPACITY);

	return list;
}

static void add_element_to_dt_block_list(DTBlockList* list, DTBlock* block)
{
	if (list->length >= list->capacity)
	{
		list->capacity *= 2;
		list->elements = realloc(list->elements, sizeof(DTBlock*) * list->capacity);
	}

	list->elements[list->length++] = block;
}

static DTBlock* setup_dt_block(CFBlock* block)
{
	DTBlock* dt_block = malloc(sizeof(DTBlock));
	
	dt_block->block = block;
	dt_block->dominators = setup_dt_block_list();

	return dt_block;
}

static void generate_dominator_child(DTBlock* parent, CFBlock* block, CFPathBlock* self)
{
	DTBlock* dt_block = setup_dt_block(block);

	int length = parent->dominators->length;

	for (int i = 0; i < length; i++)
	{
		DTBlock* dblock = parent->dominators->elements[i];

		if (dblock == NULL)
		{
			continue;
		}

		add_element_to_dt_block_list(dt_block->dominators, dblock);
	}
	
	if (self == NULL)
	{
		add_element_to_dt_block_list(dt_block->dominators, dt_block);
		
		self = setup_path_block(block);
	}
	
	self->paths++;
	
	generate_dominator_childs(dt_block);
}

static void generate_dominator_childs(DTBlock* parent)
{
	CFBlockList* childs = parent->block->successors;
	int length = childs->length;

	for (int i = 0; i < length; i++)
	{
		CFBlock* block = childs->elements[i];

		if (block == NULL)
		{
			continue;
		}

		CFPathBlock* self = find_cf_path_block(list, block);

		generate_dominator_child(parent, block, self);
	}
}

DTBlock* setup_generate_dominator_tree(CFBlock* block)
{
	setup_global_list();
	
	DTBlock* dt_block = setup_dt_block(block);
	add_element_to_dt_block_list(dt_block->dominators, dt_block);

	generate_dominator_childs(dt_block);

	return dt_block;
}