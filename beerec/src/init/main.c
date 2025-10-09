#include "compiler.h"
#include "../frontend/modules/modules.h"

// Entry-Point do compiler, argv[1] é o path pro .mod e o argv[1] é o path pro Entry-Point, e o resto são flags
int main(int argc, char** argv)
{
	ModuleConfig* mod_cfg = handle_module_config(argv[1]);
	compile_module(NULL, argv, argv[2]);
	
	return 0;
}