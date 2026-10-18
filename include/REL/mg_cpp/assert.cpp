#include "REL/mg_cpp/assert.h"
#include "stdio.h"
#include "dolphin.h"

extern "C" void __assert_fail(char *str, char *file, int line)
{
    char buf[1024];
    sprintf(buf, "\nASSERTION FAILED at %s:%d\n%s", file, line, str);
    OSPanic(__FILE__, 18, buf);
}

extern "C" int fprintf(FILE *file, const char *s, ...)
{
    va_list list;
    int n;
    va_start(list, s);
    n = vprintf(s, list);
    va_end(list);
    return n;
}
