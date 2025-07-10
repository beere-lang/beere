#include <stdio.h>
#include <stdlib.h>

void printn(char* str)
{
    printf("%s", str);
}

void* mallocn(int size)
{
    return malloc(size);
}