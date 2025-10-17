#ifndef TYPES_H
#define TYPES_H

typedef struct Type Type;

typedef enum
{
	TYPE_INT,
	TYPE_FLOAT,
	TYPE_DOUBLE,
	TYPE_BOOL,
	TYPE_STRING,
	TYPE_CHAR
} BaseType;

struct Type
{
	BaseType type;

	Type*	   base;
};

#endif