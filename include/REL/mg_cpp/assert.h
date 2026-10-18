#ifndef MGCPP_ASSERT_H
#define MGCPP_ASSERT_H


#define ASSERT_LINE(x, line) ((x) ? ((void)0) : (void)(__assert_fail(#x, __FILE__, line)))

extern "C" void __assert_fail(char *str, char *file, int line);

#endif