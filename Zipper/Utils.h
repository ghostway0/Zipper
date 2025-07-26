#ifndef ZIPPER_UTILS_H_
#define ZIPPER_UTILS_H_

#include <ntddk.h>
#define ZIPPER_ASSERT(X)                                  \
    do {                                                  \
        if (!(X)) {                                       \
            DbgPrint("ZIPPER_ASSERT failed: %s (%s:%d)\n",\
                     #X, __FILE__, __LINE__);             \
            DbgBreakPoint();                              \
        }                                                 \
    } while (0)

#define ZIPPER_UNREACHABLE(M)                                       \
    do {                                                            \
        if (!(M)) {                                                 \
            DbgPrint("ZIPPER_UNREACHABLE was reached: %s (%s:%d)\n",\
                     #M, __FILE__, __LINE__);                       \
            DbgBreakPoint();                                        \
        }                                                           \
    } while (0)


#endif // ZIPPER_UTILS_H_
