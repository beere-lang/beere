#include "codegen-continue.h"

// do codegen-break
extern SymbolTable* found_loop_scope(SymbolTable* scope);

extern int fors_count;
extern int whiles_count;

void generate_continue(CodeGen* codegen, Node* node, AsmArea* area)
{
	SymbolTable* scope = found_loop_scope(codegen->scope);
	char buff[64];

	if (scope->scope_kind == SYMBOL_WHILE)
	{ 
		snprintf(buff, 64, "	jmp	.while_then_%d", whiles_count - 1);
		add_line_to_area(area, buff);
	}
	else if(scope->scope_kind == SYMBOL_FOR)
	{
		snprintf(buff, 64, "	jmp	.for_inc_%d", fors_count - 1);
		add_line_to_area(area, buff);
	}
}