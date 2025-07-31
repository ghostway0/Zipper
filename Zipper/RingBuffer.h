#ifndef ZIPPER_RINGBUFFER_H_
#define ZIPPER_RINGBUFFER_H_

#include <atomic>

#include "Optional.h"
#include "Intrinsics.h"
#include "MMUtils.h"

template<typename T>
class MPSCRingBuffer {
public:
    MPSCRingBuffer(UINT64 Size) : m_Size(Size) {
        m_Buffer = (T *)AllocateVirtual(Size * sizeof(T));
        m_Committed = (T *)AllocateVirtualAligned(Size * sizeof(std::atomic<UINT64>),
                _AlignOf(std::atomic<UINT64>));
        RtlZeroMemory(m_Buffer, Size * sizeof(T));
        RtlZeroMemory(m_Committed, Size * sizeof(std::atomic<UINT64>));
    }

    ~MPSCRingBuffer() {
        FreeVirtual(m_Buffer, Size * sizeof(T));
        FreeVirtual(m_Committed, Size * sizeof(std::atomic<UINT64>));
    }

    BOOL Add(T Item) {
        UINT64 Tail = m_TailReserved.load(std::memory_order_acquire);
        do {
            UINT64 Head = m_Head.load(std::memory_order_acquire);
            if (Tail - Head >= m_Size) {
                return FALSE;
            }
        } while (!m_TailReserved.compare_exchange_weak(Tail, Tail + 1,
                    std::memory_order_acq_rel, std::memory_order_acquire));

        UINT64 Idx = Tail % m_Size;
        m_Buffer[Idx] = Item;

        std::atomic_thread_fence(std::memory_order_release);

        m_Committed[Idx].fetch_add(1, std::memory_order_acq_rel);
        return TRUE;
    }

    Optional<T> GetNoWait() {
        UINT64 Head = m_Head.load(std::memory_order_acquire);
        UINT64 Idx = Head % m_Size;

        UINT64 HeadSeq = m_Committed[Idx].load(std::memory_order_acquire);
        UINT64 ExpectedSeq = m_SeqNumber * 2 + 1;

        if (HeadSeq != ExpectedSeq) {
            return Optional<T>::Null();
        }

        T Value = m_Buffer[Idx];

        if (Idx == m_Size - 1) {
            ++m_SeqNumber;
        }

        m_Committed[Idx].fetch_add(1, std::memory_order_release);
        m_Head.fetch_add(1, std::memory_order_release);
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
    std::atomic<UINT64> *m_Committed;
    alignas(64) std::atomic<UINT64> m_SeqNumber{0};
    alignas(64) std::atomic<UINT64> m_TailReserved{0};
    alignas(64) std::atomic<UINT64> m_Head{0};
    UINT64 m_Size;
};

#endif // ZIPPER_RINGBUFFER_H_
