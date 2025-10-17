#ifndef LOGGER_H
#define LOGGER_H

#include "../../../data/data.h"

// Prefixes usados no logger.
#define LOGGER_ERROR_PREFIX	"[ERROR]"
#define LOGGER_SUCCESS_PREFIX "[SUCCESS]"
#define LOGGER_WARNING_PREFIX "[WARNING]"

// Printa a string 'message' (formatada) com uma quebra de linha no final '\n'.
void println(const str message, ...);

// Printa a string 'message' (formatada), com o prefix correspondente e uma quebra de linha no final '\n'.
void log_error(const str message, ...);
void log_success(const str message, ...);
void log_warning(const str message, ...);

#endif
