#ifndef LIST_H
#define LIST_H

typedef struct DList DList;

struct DList
{
	void** elements;

	size_t length;
	size_t capacity;
};

DList* create_list(size_t init_capacity);

void	 add_element_to_list(DList* list, void* element);
void*	 get_element_from_index(DList* list, const size_t index);

void*	 pop_list(DList* list);
int	 is_empty_list(DList* list);

void	 free_list(DList* list);

int	 contains_element(DList* list, void* element);
void*	 get_element(DList* list, void* element);

#endif
