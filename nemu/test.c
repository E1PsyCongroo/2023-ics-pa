#include <string.h>
#include <stdio.h>
// test if a macro of ANY type is defined
// NOTE1: it ONLY works inside a function, since it calls `strcmp()`
/// macro stringizing
#define A
#define str_temp(x) #x
#define str(x) str_temp(x)
// NOTE2: macros defined to themselves (#define A A) will get wrong results
#define isdef(macro) (strcmp("" #macro, "" str(macro)) != 0)
isdef(A)
