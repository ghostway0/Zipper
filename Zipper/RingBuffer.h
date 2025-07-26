#ifndef ZIPPER_RINGBUFFER_H_
#define ZIPPER_RINGBUFFER_H_

#include "Optional.h"
#include "Intrinsics.h"
#include "MMUtils.h"

template<typename T>
class MPSCRingBuffer {
public:
    MPSCRingBuffer(UINT64 Size) : m_Size(Size) {
        m_Buffer = AllocateVirtual((Size * sizeof(T) + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1));
    }

    ~MPSCRingBuffer() {
        FreeVirtual(m_Buffer, (m_Size * sizeof(T) + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1));
    }

    bool Add(T Item) {
        UINT64 Tail = AtomicLoad<UINT64>(&m_TailReserved, __ATOMIC_ACQ_REL);
        do {
            UINT64 Head = AtomicLoad<UINT64>(&m_Head, __ATOMIC_ACQUIRE);
            if (Tail - Head >= m_Size) {
                return false;
            }
        } while (AtomicExchangeWeak<UINT64>(&m_TailReserved, &Tail, Tail + 1,
                    __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE));

        m_Buffer[Tail % m_Size] = Item;

        while (AtomicLoad<UINT64>(&m_Tail, __ATOMIC_ACQUIRE) != Tail);
        AtomicStore<UINT64>(&m_Tail, Tail + 1, __ATOMIC_RELEASE);

        return true;
    }

    Optional<T> GetNoWait() {
        UINT64 Head = AtomicLoad<UINT64>(&m_Head, __ATOMIC_ACQ_REL);
        UINT64 Tail = AtomicLoad<UINT64>(&m_Tail, __ATOMIC_ACQUIRE);

        if (Head == Tail) {
            return Optional<T>::Null();
        }

        T Value = m_Buffer[Head % m_Size];
        AtomicFetchAdd<UINT64>(&m_Head, 1, __ATOMIC_RELEASE);
        return Value;
    }

    Optional<T> GetTimeout(UINT64 MaxIter = 512) {
        Optional<T> Value;
        UINT64 Iter = 0;
        do {
            Value = GetNoWait();
        } while (Value.Empty() && Iter++ < MaxIter);

        return Value;
    }

    T Get() {
        Optional<T> Value;
        do {
            Value = GetNoWait();
        } while (Value.Empty());

        return Value.Get();
    }

private:
    T *m_Buffer;
    alignas(64) UINT64 m_TailReserved{0};
    alignas(64) UINT64 m_Tail{0};
    alignas(64) UINT64 m_Head{0};
    UINT64 m_Size;
};

#endif // ZIPPER_RINGBUFFER_H_
