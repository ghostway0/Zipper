#ifndef ZIPPER_RINGBUFFER_H_
#define ZIPPER_RINGBUFFER_H_

#include "Optional.h"
#include "Intrinsics.h"
#include "MMUtils.h"

template<typename T>
class MPSCRingBuffer {
public:
    MPSCRingBuffer(UINT64 Size) : m_Size(Size) {
        m_Buffer = (T *)AllocateVirtual(Size * sizeof(T));
        m_Committed = (T *)AllocateVirtualAligned(Size * sizeof(UINT64),
                _AlignOf(UINT64));
        RtlZeroMemory(m_Buffer, Size * sizeof(T));
        RtlZeroMemory(m_Committed, Size * sizeof(UINT64));
    }

    ~MPSCRingBuffer() {
        FreeVirtual(m_Buffer, Size * sizeof(T));
        FreeVirtual(m_Committed, Size * sizeof(UINT64));
    }

    BOOL Add(T Item) {
        UINT64 Tail = AtomicLoad(&m_TailReserved, MO_ACQUIRE);
        do {
            UINT64 Head = AtomicLoad(&m_Head, MO_ACQUIRE);
            if (Tail - Head >= m_Size) {
                return FALSE;
            }
        } while (AtomicCompareExchangeWeak(&m_TailReserved, Tail, Tail + 1,
                    MO_ACQ_REL, MO_ACQUIRE));

        UINT64 Idx = Tail % m_Size;
        m_Buffer[Idx] = Item;

        AtomicFence(MO_RELEASE);

        AtomicFetchAdd(&m_Committed + Idx, (UINT64)1, MO_ACQ_REL)
        return TRUE;
    }

    Optional<T> GetNoWait() {
        UINT64 Head = AtomicLoad(&m_Head, MO_ACQUIRE);
        UINT64 Idx = Head % m_Size;

        UINT64 HeadSeq = AtomicLoad(m_Committed + Idx, MO_ACQUIRE);
        UINT64 ExpectedSeq = m_SeqNumber * 2 + 1;

        if (HeadSeq != ExpectedSeq) {
            return Optional<T>::Null();
        }

        T Value = m_Buffer[Idx];

        if (Idx == m_Size - 1) {
            ++m_SeqNumber;
        }

        AtomicFetchAdd(m_Committed + Idx, (UINT64)1, MO_RELEASE);
        AtomicFetchAdd(m_Head, (UINT64)1, MO_RELEASE);
        return Value;
    }

    Optional<T> GetTimeout(UINT64 MaxIter = 512) {
        Optional<T> Value;
        UINT64 Iter = 0;
        do {
            Value = GetNoWait();
            Pause();
        } while (Value.Empty() && Iter++ < MaxIter);

        return Value;
    }

    T Get() {
        const UINT64 MaxBackoff = 64;
        Optional<T> Value;
        UINT64 Backoff = 1;

        do {
            for (UINT64 Iter = 1; Iter < Backoff; Iter++) {
                Pause();
            }
            Backoff = (Backoff * 2 < MaxBackoff) ? Backoff * 2 : MaxBackoff;

            Value = GetNoWait();
        } while (Value.Empty());
        return Value.Get();
    }

private:
    T *m_Buffer;
    UINT64 *m_Committed;
    alignas(64) UINT64 m_SeqNumber{0};
    alignas(64) UINT64 m_TailReserved{0};
    alignas(64) UINT64 m_Head{0};
    UINT64 m_Size;
};

#endif // ZIPPER_RINGBUFFER_H_
