
#include <thread>
#include <vector>
#include <chrono>
#include <iostream>
#include <atomic>
#include <cassert>
#include <cstdlib>

#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <thread>

#define UINT64 uint64_t
#define UINT8 uint8_t
#define PAGE_SIZE 4096

#include "Zipper/Optional.h"
#include "Zipper/Intrinsics.h"

#define alignas(x)

template<typename T>
class MPSCRingBuffer {
public:
    MPSCRingBuffer(UINT64 Size) : m_Size(Size) {
        m_Buffer = (T*)malloc((Size * sizeof(T) + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1));
        m_Committed = (UINT64*)calloc(Size, sizeof(UINT64));
    }

    ~MPSCRingBuffer() {
        free(m_Buffer);
        free(m_Committed);
    }

    bool Add(T Item) {
        UINT64 Tail = AtomicLoad<UINT64>(&m_TailReserved, __ATOMIC_ACQUIRE);
        do {
            UINT64 Head = AtomicLoad<UINT64>(&m_Head, __ATOMIC_ACQUIRE);
            if (Tail - Head >= m_Size) {
                return false;
            }
        } while (!AtomicCompareExchangeWeak<UINT64>(&m_TailReserved, &Tail, Tail + 1,
                    __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE));

        UINT64 Idx = Tail % m_Size;
        m_Buffer[Idx] = Item;

        // MemoryFence(__ATOMIC_RELEASE);

        // We can't overrun Head - we checked that earlier. Just add 1.
        AtomicFetchAdd(m_Committed + Idx, (UINT64)1, __ATOMIC_ACQ_REL);

        // UINT64 SeqNumber = AtomicLoad(m_Committed + Idx, __ATOMIC_ACQUIRE);
        // while (SeqNumber % 2 == 0 &&
        //        !AtomicCompareExchangeWeak(m_Committed + Idx, &SeqNumber, SeqNumber + 1,
        //                                   __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE));

        // 1. fastest but incorrect
        // while (!AtomicCompareExchangeWeak(&m_Tail, &Tail, Tail + 1,
        //             __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE));

        // 2. faster than (3) for some reason??
        // UINT64 Tmp = Tail;
        // while (!AtomicCompareExchangeWeak(&m_Tail, &Tmp, Tail + 1,
        //             __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)) {
        //     Tmp = Tail;
        // }
        // for (UINT64 Current = Tail; !AtomicCompareExchangeWeak<UINT64>(&m_Tail, &Current,
        //             Tail + 1, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE); Current = Tail) {
        //     Pause();
        //     // __asm__ __volatile__ ("yield");
        // }
        // UINT64 CurrentTail = Tail;
        // while (!AtomicCompareExchangeWeak<UINT64>(&m_Tail, &CurrentTail, Tail + 1,
        //             __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)) {
        //     CurrentTail = AtomicLoad<UINT64>(&m_Tail, __ATOMIC_ACQUIRE);
        // }

        // 3. ACQ_REL is faster here than ACQ. 3x slower, slower when aligned to cache line?
        // while (AtomicLoad<UINT64>(&m_Tail, __ATOMIC_ACQUIRE) != Tail);
        // AtomicStore<UINT64>(&m_Tail, Tail + 1, __ATOMIC_RELEASE);
        return true;
    }

    Optional<T> GetNoWait() {
        UINT64 Head = AtomicLoad(&m_Head, __ATOMIC_ACQUIRE);
        UINT64 Idx = Head % m_Size;

        UINT64 HeadSeq = AtomicLoad(m_Committed + Idx, __ATOMIC_ACQUIRE);
        UINT64 ExpectedSeq = m_SeqNumber * 2 + 1;

        // std::cout << ExpectedSeq << " "<< HeadSeq << " " << Head << "\n";
        if (HeadSeq != ExpectedSeq) {
            return Optional<T>::Null();
        }

        T Value = m_Buffer[Idx];

        if (Idx == m_Size - 1) {
            AtomicFetchAdd(&m_SeqNumber, (UINT64)1, __ATOMIC_ACQ_REL);
        }

        AtomicFetchAdd(m_Committed + Idx, (UINT64)1, __ATOMIC_RELEASE);
        AtomicFetchAdd(&m_Head, (UINT64)1, __ATOMIC_RELEASE);

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
    UINT64 *m_Committed;
    alignas(64) UINT64 m_SeqNumber{0};
    alignas(64) UINT64 m_TailReserved{0};
    // alignas(64) UINT64 m_Tail{0};
    alignas(64) UINT64 m_Head{0};
    UINT64 m_Size;
};

constexpr size_t ITEMS_PER_PRODUCER = 400000;
constexpr int NUM_PRODUCERS = 5;
constexpr size_t RING_SIZE = 4096;

MPSCRingBuffer<int> rb(RING_SIZE);
std::atomic<int> consumed_count{0};

void producer(int start) {
    for (int i = 0; i < ITEMS_PER_PRODUCER; ++i) {
        while (!rb.Add(start + i)) {
        }
    }
}

void consumer() {
    int expected_total = ITEMS_PER_PRODUCER * NUM_PRODUCERS;
    while (consumed_count.load(std::memory_order_acquire) < expected_total) {
        auto val = rb.GetNoWait();
        if (val.HasValue()) {
            consumed_count.fetch_add(1, std::memory_order_release);
        }
    }
}

int main() {
    using namespace std::chrono;
    std::cout << "Starting MPSC ring buffer benchmark...\n";

    auto start_time = high_resolution_clock::now();

    std::vector<std::thread> producers;
    for (int i = 0; i < NUM_PRODUCERS; ++i) {
        producers.emplace_back(producer, i * ITEMS_PER_PRODUCER);
    }

    std::thread cons(consumer);

    for (auto &p : producers) p.join();
    cons.join();

    auto end_time = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end_time - start_time).count();

    std::cout << "Consumed " << consumed_count.load() << " items in " << duration << " ms\n";
    std::cout << "Throughput: " << (consumed_count.load() * 1000ull) / duration << " items/sec\n";

    return 0;
}
