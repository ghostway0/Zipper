#ifndef ZIPPER_INTRINSICS_H_
#define ZIPPER_INTRINSICS_H_

#include <wdm.h>

#include "Utils.h"

#define _AlignOf(x) __alignof(x)

enum MemoryOrder {
    MO_RELAXED = 0,
    MO_SEQ_CST,
    MO_ACQUIRE,
    MO_RELEASE,
    MO_ACQ_REL,
};

inline void AtomicFence(int MemOrder = MO_SEQ_CST) {
    switch (MemOrder) {
    case MO_SEQ_CST:
        KeMemoryBarrier();
        break;
    case MO_ACQUIRE:
        KeMemoryBarrierAcquire();
        break;
    case MO_RELEASE:
        KeMemoryBarrierRelease();
        break;
    case MO_ACQ_REL:
        ZIPPER_UNREACHABLE("MO_ACQ_REL is not valid for standalone fences.");
    default:
        break;
    }
}

template<typename T>
inline typename EnableIf<IsIntegralV<T> && sizeof(T) == 4, T>::Type
AtomicLoad(const T* Ptr, int MemOrder = MO_SEQ_CST) {
    T Tmp = *Ptr;
    InterlockedCompareExchange((volatile INT32 *)&Tmp, 0, 0);
    return Tmp;
}

template<typename T>
inline typename EnableIf<IsIntegralV<T> && sizeof(T) == 8, T>::Type
AtomicLoad(const T* Ptr, int MemOrder = MO_SEQ_CST) {
    T Tmp = *Ptr;
    InterlockedCompareExchange64((volatile INT64 *)&Tmp, 0, 0);
    return Tmp;
}

template<typename T>
inline typename EnableIf<IsIntegralV<T> && sizeof(T) == 4>::Type
AtomicStore(T* Ptr, T Value, int MemOrder = MO_SEQ_CST) {
    InterlockedExchange((volatile INT32 *)Ptr, Value);
}

template<typename T>
inline typename EnableIf<IsIntegralV<T> && sizeof(T) == 8>::Type
AtomicStore(T* Ptr, T Value, int MemOrder = MO_SEQ_CST) {
    InterlockedExchange64((volatile INT64 *)Ptr, Value);
}

template<typename T>
inline typename EnableIf<IsIntegralV<T> && sizeof(T) == 4, bool>::Type
AtomicCompareExchangeWeak(T* Ptr, T* Expected, T Desired, int MemOrder = MO_SEQ_CST) {
    T Prev = InterlockedCompareExchange((volatile INT32 *)Ptr, Desired, *Expected);
    if (Prev == *Expected) {
        return TRUE;
    }
    *Expected = Prev;
    return FALSE;
}

template<typename T>
inline typename EnableIf<IsIntegralV<T> && sizeof(T) == 8, bool>::Type
AtomicCompareExchangeWeak(T* Ptr, T* Expected, T Desired, int MemOrder = MO_SEQ_CST) {
    T Prev = InterlockedCompareExchange64((volatile INT64 *)Ptr, Desired, *Expected);
    if (Prev == *Expected) {
        return TRUE;
    }
    *Expected = Prev;
    return FALSE;
}

template<typename T>
inline typename EnableIf<IsIntegralV<T> && sizeof(T) == 4, T>::Type
AtomicFetchAdd(T* Ptr, T Value, int MemOrder = MO_SEQ_CST) {
    return InterlockedExchangeAdd((volatile INT32 *)Ptr, Value);
}

template<typename T>
inline typename EnableIf<IsIntegralV<T> && sizeof(T) == 8, T>::Type
AtomicFetchAdd(T* Ptr, T Value, int MemOrder = MO_SEQ_CST) {
    return InterlockedExchangeAdd64((volatile INT64 *)Ptr, Value);
}

extern void Pause();

extern void InterruptContinue();

extern void InterruptStop();

#if defined(__x86_64__) || defined(__i386__) || defined(_M_IX86) ||            \
    defined(_M_X64)

#include "X86/Intrinsics.h"

#endif // __x86_64__

#endif // ZIPPER_INTRINSICS_H_
