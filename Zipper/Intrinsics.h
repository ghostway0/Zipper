#ifndef ZIPPER_INTRINSICS_H_
#define ZIPPER_INTRINSICS_H_

template<typename T>
inline T AtomicLoad(const T *Ptr, int MemOrder = __ATOMIC_SEQ_CST) {
    T Val;
    __atomic_load(Ptr, &Val, MemOrder);
    return Val;
}

template<typename T>
inline void AtomicStore(T *Ptr, T Value, int MemOrder = __ATOMIC_SEQ_CST) {
    __atomic_store(Ptr, &Value, MemOrder);
}

template<typename T>
inline T AtomicFetchAdd(T *Ptr, T Value, int MemOrder = __ATOMIC_SEQ_CST) {
    return __atomic_fetch_add(Ptr, Value, MemOrder);
}

template<typename T>
inline T AtomicFetchXchg(T *Ptr, T Value, int MemOrder = __ATOMIC_SEQ_CST) {
    T Old;
    __atomic_exchange(Ptr, &Value, &Old, MemOrder);
    return Old;
}

template<typename T>
inline bool AtomicCompareExchangeWeak(T *Ptr, T *Expected, T Desired,
        int SuccsMemOrder, int FailMemOrder) {
    return __atomic_compare_exchange_n(Ptr, Expected, Desired, /*weak=*/true,
            SuccsMemOrder, FailMemOrder);
}

inline void MemoryFence(int MemOrder) {
    __atomic_thread_fence(MemOrder);
}

extern void Pause();

extern void InterruptContinue();

extern void InterruptStop();

#if defined(__x86_64__) || defined(__i386__) || defined(_M_IX86) || defined(_M_X64)

#include "X86/Intrinsics.h"

#endif // __x86_64__

#endif // ZIPPER_INTRINSICS_H_
