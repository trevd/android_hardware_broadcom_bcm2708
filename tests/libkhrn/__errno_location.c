#include <errno.h>

volatile int *__errno_location(void)
{
    return (int *)&errno;
}
