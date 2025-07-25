#ifndef ZIPPER_INTRINSICS_H_
#define ZIPPER_INTRINSICS_H_

template<typename T>
inline T AtomicLoad(const T* Ptr, int MemOrder = __ATOMIC_SEQ_CST) {
    T Val;
    __atomic_load(Ptr, &Val, MemOrder);
    return Val;
}

template<typename T>
inline T AtomicFetchAdd(T* Ptr, T Value, int MemOrder = __ATOMIC_SEQ_CST) {
    return __atomic_fetch_add(Ptr, Value, MemOrder);
}

template<typename T>
inline T AtomicFetchXchg(T* Ptr, T Value, int MemOrder = __ATOMIC_SEQ_CST) {
    T Old;
    __atomic_exchange(Ptr, &Value, &Old, MemOrder);
    return Old;
}

inline void Pause() {
    _mm_pause();
}

#endif // ZIPPER_INTRINSICS_H_
