#ifndef ZIPPER_SPINLOCK_H_
#define ZIPPER_SPINLOCK_H_

#include "Intrinsics.h"

class SpinLock {
public:
    void Lock() {
        while (!TryLock()) { Pause(); }
    }
    
    bool LockTimeout(UINT64 MaxIter) {
        UINT64 Iter = 0;
        bool Locked = false;
        while (!(Locked = TryLock()) && Iter < MaxIter) {
            Pause();
            ++Iter;
        }

        return Locked;
    }

    bool TryLock() {
        return AtomicFetchXchg(&m_Flag, 1, __ATOMIC_ACQUIRE) == 0;
    }

    void Unlock() {
        AtomicFetchXchg(&m_Flag, 0, __ATOMIC_RELEASE);
    }

private:
    UINT64 m_Flag{0};
};

#endif // ZIPPER_SPINLOCK_H_
