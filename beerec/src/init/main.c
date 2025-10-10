#include "compiler.h"
#include "../frontend/modules/modules.h"

// Entry-Point do compiler, argv[1] é o path pro dotmod e o argv[2] é o path pro Entry-Point, e o resto são flags
int main(int argc, char** argv)
{
	ModuleConfig* mod_cfg = handle_module_config(argv[1], 0);
	//compile_module(NULL, argv, argv[2]);
	
	return 0;
}