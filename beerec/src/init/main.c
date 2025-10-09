#include "compiler.h"

// Entry-Point do compiler, argv[1] é o path pro .mod e o argv[1] é o path pro Entry-Point.
int main(int argc, char** argv)
{
	compile_module(NULL, argv, argv[2]);
	
	return 0;
}