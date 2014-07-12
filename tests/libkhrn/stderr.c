#include <stdio.h>

#define stderr_internal stderr
#undef stderr  

extern FILE* stderr();

extern FILE* stderr()
{
		return stderr_internal;
}
