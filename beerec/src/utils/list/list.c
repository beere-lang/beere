#include <stdlib.h>

#include "list.h"
#include "../logger/logger.h"

DList* create_list(const size_t init_capacity)
{
	DList* list = malloc(sizeof(DList));

	if (list == NULL)
	{
		println("Failed to alloc memory for list...");
		exit(1);
	}

	list->capacity = init_capacity;
	list->length = 0;

	list->elements = malloc(8 * init_capacity);

	if (list->elements == NULL)
	{
		println("Failed to alloc memory for list...");
		exit(1);
	}

	return list;
}

void add_element_to_list(DList* list, void* element)
{
	if (list == NULL)
	{
		println("List is NULL...");
		exit(1);
	}

	if (list->length >= list->capacity)
	{
		list->capacity *= 2;
		list->elements = realloc(list->elements, 8 * list->capacity);
	}

	list->elements[list->length++] = element;
}

void* get_element_from_index(DList* list, const size_t index)
{
	if (list == NULL)
	{
		println("List is NULL...");
		exit(1);
	}

	if (index > list->length - 1)
	{
		println("Accessing a invalid array element from index: %d...", index);
		exit(1);
	}

	return list->elements[index];
}

int contains_element(DList* list, void* element)
{
	if (list == NULL)
	{
		println("List is NULL...");
		exit(1);
	}

	if (element == NULL)
	{
		println("Element is NULL...");
		exit(1);
	}

	for (size_t i = 0; i < list->length; i++)
	{
		void* e = list->elements[i];

		if (e != element)
		{
			continue;
		}

		return 1;
	}

	return 0;
}

void* pop_list(DList* list)
{
	void* element = list->elements[list->length - 1];

	list->length--;
	list->elements[list->length - 1] = NULL;

	return element;
}

int is_empty_list(DList* list)
{
	return list->length <= 0;
}

void free_list(DList* list)
{
	for (int i = 0; i < list->length; i++)
	{
		void* element = list->elements[i];

		if (element == NULL)
		{
			continue;
		}

		free(element);
		list->elements[i] = NULL;
	}

	free(list);
}
