#include "compiler.h"

/**
 * Função main
 *  - Compila o entry point (arquivo principal, o de inicio)
 */
int main(int _, char* argv[])
{
	Module* main_module = compile(argv[1], argv[2]);

	return 0;
}