#ifndef ZIPPER_SPINLOCK_H_
#define ZIPPER_SPINLOCK_H_

#include "Intrinsics.h"

class SpinLock {
public:
    void Lock() {
        while (!TryLock()) { Pause(); }
    }
    
    BOOL LockTimeout(UINT64 MaxIter) {
        UINT64 Iter = 0;
        BOOL Locked = FALSE;
        while (!(Locked = TryLock()) && Iter < MaxIter) {
            Pause();
            ++Iter;
        }

        return Locked;
    }

    BOOL TryLock() {
        return AtomicFetchXchg(&m_Flag, 1, MO_ACQUIRE) == 0;
    }

    void Unlock() {
        AtomicFetchXchg(&m_Flag, 0, MO_RELEASE);
    }

private:
    UINT64 m_Flag{0};
};

#endif // ZIPPER_SPINLOCK_H_
