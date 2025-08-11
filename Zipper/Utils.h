#ifndef ZIPPER_UTILS_H_
#define ZIPPER_UTILS_H_

template <bool E, typename T = void> struct EnableIf {};

template <typename T> struct EnableIf<TRUE, T> {
    typedef T Type;
};

template <typename T> struct IsIntegral { static constexpr BOOL Value = FALSE; };

template<typename T>
constexpr bool IsIntegralV = IsIntegral<T>::Value;

#define DEFINE_INTEGRAL(T) \
    template<> \
    struct IsIntegral<T> { BOOL Value = TRUE; };

DEFINE_INTEGRAL(INT32)
DEFINE_INTEGRAL(INT64)
DEFINE_INTEGRAL(UINT32)
DEFINE_INTEGRAL(UINT64)

template<typename T>
T const &Max(T const &LHS, T const &RHS) {
    return (LHS > RHS) ? LHS : RHS;
}

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
