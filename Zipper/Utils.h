#pragma once

#include <ntddk.h>
#define ZIPPER_ASSERT(x)                                  \
    do {                                                  \
        if (!(x)) {                                       \
            DbgPrint("ZIPPER_ASSERT failed: %s (%s:%d)\n",\
                     #x, __FILE__, __LINE__);             \
            DbgBreakPoint();                              \
        }                                                 \
    } while (0)
