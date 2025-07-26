#ifndef ZIPPER_INTRINSICS_H_
#define ZIPPER_INTRINSICS_H_

template<typename T>
inline T AtomicLoad(const T *Ptr, int MemOrder = __ATOMIC_SEQ_CST) {
    T Val;
    __atomic_load(Ptr, &Val, MemOrder);
    return Val;
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
inline bool AtomicExchangeWeak(T *Ptr, T *Expected, T Desired,
        int SuccsMemOrder, int FailMemOrder) {
    return __atomic_compare_exchange_n(Ptr, Expected, Desired, /*weak=*/true,
            SuccsMemOrder, FailMemOrder);
}

inline void Pause() {
    _mm_pause();
}


inline void InterruptContinue() {
    __asm {
        STI
    }
}

inline void InterruptStop() {
    __asm {
        CLI
    }
}

#endif // ZIPPER_INTRINSICS_H_
