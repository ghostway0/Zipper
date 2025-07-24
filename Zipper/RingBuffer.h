#include "Types.h"

#include "Optional.h"

#define PAGE_SIZE (UINT64_T)4096

template<typename T>
class MPSCRingBuffer {
public:
    MPSCRingBuffer(UINT64 Size) : m_Size(Size), m_Tail(0), m_Head(0) {
        m_Buffer = AllocateVirtual((Size * sizeof(T) + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1));
    }

    bool Add(T Item) {
        UINT64 Tail = AtomicFetchAdd<UINT64>(&m_Tail, 1, __ATOMIC_ACQ_REL);
        UINT64 Head = AtomicLoad<UINT64>(&m_Head, __ATOMIC_ACQUIRE);

        if (Tail - Head >= m_Size) {
            return false;
        }

        m_Buffer[Tail % m_Size] = Item;
        return true;
    }

    Optional<T> GetNoWait() {
        UINT64 Head = AtomicLoad<UINT64>(&m_Head, __ATOMIC_ACQ_REL);
        UINT64 Tail = AtomicLoad<UINT64>(&m_Tail, __ATOMIC_ACQUIRE);

        if (Head == Tail) {
            return Optional<T>::Null();
        }

        AtomicFetchAdd<UINT64>(&m_Head, 1, __ATOMIC_ACQ_REL);
        return m_Buffer[Head % m_Size];
    }

    Optional<T> GetTimeout(UINT64 MaxIter = 512) {
        Optional<T> Value;
        UINT64 Iter = 0;
        do {
            Value = GetNoWait();
        } while (Value.Empty() && Iter++ < MaxIter);

        return Value.Get();
    }

    T Get() {
        Optional<T> Value;
        do {
            Value = GetNoWait();
        } while (Value.Empty());

        return Value.Get();
    }

private:
    T* m_Buffer;
    UINT64 m_Tail;
    UINT64 m_Head;
    UINT64 m_Size;
};
