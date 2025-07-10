#include "logger.h"
#include <stdio.h>

void parser_error(const char* message)
{
	printf("[Parser] [Error] %s\n", message);
}

void parser_alert(const char* message)
{
	printf("[Parser] [Alert] %s\n", message);
}

void parser_info(const char* message)
{
	printf("[Parser] [Debug] %s\n", message);
}