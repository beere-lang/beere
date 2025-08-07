#include "codegen-break.h"

extern int fors_count;
extern int whiles_count;

SymbolTable* found_loop_scope(SymbolTable* scope)
{
	if (scope == NULL)
	{
		return NULL;
	}

	if (scope->scope_kind == SYMBOL_FOR || scope->scope_kind == SYMBOL_WHILE)
	{
		return scope;
	}

	return found_loop_scope(scope->parent);
}

void generate_break(CodeGen* codegen, Node* node, AsmArea* area)
{
	SymbolTable* scope = found_loop_scope(codegen->scope);
	char buff[64];

	if (scope->scope_kind == SYMBOL_WHILE)
	{ 
		snprintf(buff, 64, "	jmp	.while_post_%d", whiles_count - 1);
		add_line_to_area(area, buff);
	}
	else if(scope->scope_kind == SYMBOL_FOR)
	{
		snprintf(buff, 64, "	jmp	.for_post_%d", fors_count - 1);
		add_line_to_area(area, buff);
	}
}