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
inline T AtomicFetchOr(T *Ptr, T Value, int MemOrder = __ATOMIC_SEQ_CST) {
    __sync_fetch_and_or(Ptr, Value, MemOrder);
}

inline void MemoryFence(int MemOrder = __ATOMIC_SEQ_CST) {
    __atomic_thread_fence(MemOrder);
}

template<typename T>
inline bool AtomicCompareExchangeWeak(T *Ptr, T *Expected, T Desired,
        int SuccsMemOrder, int FailMemOrder) {
    return __atomic_compare_exchange_n(Ptr, Expected, Desired, /*weak=*/true,
            SuccsMemOrder, FailMemOrder);
}

#if defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64)
#include <immintrin.h> // or <xmmintrin.h> for older compatibility

inline void Pause() {
    _mm_pause();
}
#endif

inline void InterruptContinue() {
    asm volatile ("sti");
}

inline void InterruptStop() {
    asm volatile ("cli");
}

#endif // ZIPPER_INTRINSICS_H_
