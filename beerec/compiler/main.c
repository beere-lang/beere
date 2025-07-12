#include "compiler.h"

/**
 * Função main
 *  - Compila o entry point (arquivo principal, o de inicio)
 */
int main(int _, char* argv[])
{
	ModuleHandler* handler = interpret_module_file(argv[2]);
	Module* main_module = compile(handler, argv[1], argv[3]);

	return 0;
}