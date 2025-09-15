#include "logger.h"

#include <stdarg.h>
#include <stdio.h>

void println(const char* str, ...)
{
	va_list args;
	va_start(args, str);

	char buff[1024];
	sprintf(buff, "%s%c", str, '\n');

	printf(buff, args);

	va_end(args);
}