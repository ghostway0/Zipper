#ifndef ZIPPER_UTILS_H_
#define ZIPPER_UTILS_H_

#include <ntddk.h>
#define ZIPPER_ASSERT(x)                                  \
    do {                                                  \
        if (!(x)) {                                       \
            DbgPrint("ZIPPER_ASSERT failed: %s (%s:%d)\n",\
                     #x, __FILE__, __LINE__);             \
            DbgBreakPoint();                              \
        }                                                 \
    } while (0)

#endif // ZIPPER_UTILS_H_
