#include "logger.h"

#include <stdarg.h>
#include <stdio.h>

void println(const str message, ...)
{
	va_list args;
	va_start(args, message);

	char buff[1024];
	sprintf(buff, "%s%c", message, '\n');

	vprintf(buff, args);

	va_end(args);
}

void log_error(const str message, ...)
{
	va_list args;
	va_start(args, message);

	char buff[1024];
	sprintf(buff, "%s %s%c", LOGGER_ERROR_PREFIX, message, '\n');

	vprintf(buff, args);

	va_end(args);
}

void log_success(const str message, ...)
{
	va_list args;
	va_start(args, message);

	char buff[1024];
	sprintf(buff, "%s %s%c", LOGGER_SUCCESS_PREFIX, message, '\n');

	vprintf(buff, args);

	va_end(args);
}

void log_warning(const str message, ...)
{
	va_list args;
	va_start(args, message);

	char buff[1024];
	sprintf(buff, "%s %s%c", LOGGER_WARNING_PREFIX, message, '\n');

	vprintf(buff, args);

	va_end(args);
}