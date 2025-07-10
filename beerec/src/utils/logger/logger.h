#ifndef LOGGER_H
#define LOGGER_H

void println(const char* str[], const int size);

void parser_error(const char* str);
void parser_info(const char* str);
void parser_alert(const char* str);

void lexer_error(const char* str[]);
void lexer_info(const char* str[]);
void lexer_alert(const char* str[]);

#endif
