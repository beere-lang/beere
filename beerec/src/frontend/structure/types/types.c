#include <corecrt_malloc.h>
#include <stdlib.h>

#include "../../../utils/logger/logger.h"
#include "types.h"

TypeTable* setup_type_table()
{
	TypeTable* table = calloc(1, sizeof(TypeTable));

	if (table == NULL)
	{
		log_error("Failed to alloc memory for type table...");
		exit(1);
	}

	table->types	    = calloc(TYPE_TABLE_START_CAPACITY, sizeof(Type));
	table->types_capacity = TYPE_TABLE_START_CAPACITY;
	table->types_length   = 0;

	return table;
}

void add_type_to_table(TypeTable* table, Type* type)
{
	if (table == NULL)
	{
		log_error("The type table is NULL...");
		exit(1);
	}

	if (table->types_length >= table->types_capacity - 1)
	{
		table->types_capacity *= 2;
		table->types = _recalloc(table->types, table->types_capacity, sizeof(Type));
	}

	table->types[table->types_length++] = type;
}

Type* create_type(BaseType base, str class_name)
{
	Type* type = calloc(1, sizeof(Type));

	if (type == NULL)
	{
		log_error("Failed to alloc memory for node...");
		exit(1);
	}

	type->type = base;
	type->base = NULL;

	return type;
}