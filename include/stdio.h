#ifndef _STDIO_H_
#define _STDIO_H_

#include "stdarg.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/file_io.h" // IWYU pragma: export

#ifdef __cplusplus
extern "C" {
#endif

int puts(const char* s);
int printf(const char*, ...);
int fprintf(FILE *, const char*, ...);
int sprintf(char* s, const char* format, ...);
int vprintf(const char* format, va_list arg);
int vsprintf(char* s, const char* format, va_list arg);

#ifdef __cplusplus
};
#endif

#define stdin  (&__files[0])
#define stdout (&__files[1])
#define stderr (&__files[2])

#endif