#ifndef NODES_H
#define NODES_H

#include "../../../../data/data.h"
#include "../types/types.h"
#include "../visibility/visibility.h"

typedef struct ASTNodeList	   ASTNodeList;

typedef struct ASTNode		   ASTNode;
typedef struct ASTNodeVariable   ASTNodeVariable;
typedef struct ASTNodeIdentifier ASTNodeIdentifier;
typedef struct ASTNodeFunc	   ASTNodeFunc;
typedef struct ASTNodeCall	   ASTNodeCall;

// Lista de nodes com length pra percorrer nelas.
// Usado mais em parametros, argumentos, etc.
struct ASTNodeList
{
	u32	   length;
	ASTNode* elements;
};

// Tipos de node.
typedef enum
{
	AST_NODE_VARIABLE,
	AST_NODE_IDENTIFIER,
	AST_NODE_FUNC,
	AST_NODE_CALL
} ASTNodeType;

// Structure de uma node de variavel,
// tendo suas informaçoes pra uso posterior.
struct ASTNodeVariable
{
	i32	     is_static;
	i32	     is_constant;

	Visibility visibility;

	Type*	     type;

	str	     identifier;

	ASTNode*   value;
};

// Structure de um identifier, e seu identifier.
struct ASTNodeIdentifier
{
	str identifier;
};

// Structure de uma node de função,
// tendo suas informaçoes pra uso posterior.
struct ASTNodeFunc
{
	i32		 is_static;

	i32		 is_virtual;
	i32		 is_override;

	Visibility	 visibility;

	Type*		 type;

	str		 identifier;

	ASTNodeList* parameters;

	ASTNode*	 body;
};

// Structure de uma chama de função, tendo
// seus argumentos e o corpo da chamada.
struct ASTNodeCall
{
	ASTNode*	 callee;

	ASTNodeList* arguments;
};

// Structure de uma node, contendo o tipo da node,
// e mais structures adicionais numa union.
struct ASTNode
{
	ASTNodeType type;

	union
	{
		ASTNodeVariable	variable;
		ASTNodeIdentifier identifier;
		ASTNodeFunc		func;
		ASTNodeCall		call;
	};
};

// Retorna uma node alocada na heap ja atribuida com o tipo 'type'.
ASTNode* create_node(ASTNodeType type);

#endif