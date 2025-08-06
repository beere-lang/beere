#include "compiler.h"

/**
 * Compila o entry point e interpreta o dotmod (passados nas args)
 */
int main(int _, char* argv[])
{
	ModuleHandler* handler = interpret_module_file(argv[2]);
	Module* main_module = compile(handler, argv[1], argv[3]);

	return 0;
}