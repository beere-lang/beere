#include "compiler.h"
#include "../utils/logger/logger.h"
#include "../frontend/modules/modules.h"

// Entry-Point do compiler, argv[1] é o path pro dotmod e o argv[2] é o path pro module Entry-Point, e o resto são flags
int main(int argc, char** argv)
{
	ModuleConfig* cfg = handle_dotmod(argv[1], 0);
	
	if (cfg == NULL)
	{
		log_error("Failed to handle dotmod from path: \"%s\"...", argv[1]);
		return 1;
	}
	
	compile_module(cfg, argv, argv[2]);
	
	return 0;
}