#ifndef TYPES_H
#define TYPES_H

#include "../../../../data/data.h"

typedef struct Type	 Type;
typedef struct TypeTable TypeTable;

// Capacidade padrão de uma type table.
#define TYPE_TABLE_START_CAPACITY 4

// Tipos de tipo, lol.
typedef enum
{
	TYPE_PTR,
	TYPE_INT,
	TYPE_FLOAT,
	TYPE_DOUBLE,
	TYPE_BOOL,
	TYPE_STRING,
	TYPE_CHAR,
	TYPE_CLASS,
	TYPE_VOID
} BaseType;

// Structure de um tipo, contendo o tipo base 'type' (int, float, double, etc...),
// o nome da class 'class_name', que é utilizado quando o tipo base for do tipo class,
// e um pointer pro tipo de depth menor 'base', usado em tipos de pointers e arrays:
// (Pointer -> Int, Array -> Int, etc...).
struct Type
{
	BaseType type;
	str	   class_name;

	Type*	   base;
};

// Uma table de types, com uma array de types 'types', a capacidade da array 'types_capacity', e a length da array
// 'types_length'.
// Costuma ser usado no "sistema" de tipos pra registrar eles (tipo um typedef).
struct TypeTable
{
	Type** types;

	u32	 types_capacity;
	u32	 types_length;
};

// Retorna um type alocado na heap ja atribuido com o tipo base, o class name e seta a base pra NULL.
// O 'class_name' é usado apenas quando o 'base' for 'TYPE_CLASS', ja que vai ser preciso saber o nome da class,
// é recomendado deixar o 'class_name' NULL caso o 'base' não seja 'TYPE_CLASS'.
Type*	     create_type(BaseType base, str class_name);

// Retorna uma type table alocada na heap, com os campos ja "inicializados".
TypeTable* setup_type_table();

// Adiciona o tipo 'type' na type table 'table'.
void	     add_type_to_table(TypeTable* table, Type* type);

#endif