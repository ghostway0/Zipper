
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
    }

    ~MPSCRingBuffer() {
        free(m_Buffer);
    }

    bool Add(T Item) {
        UINT64 Tail = AtomicLoad<UINT64>(&m_TailReserved, __ATOMIC_ACQ_REL);
        do {
            UINT64 Head = AtomicLoad<UINT64>(&m_Head, __ATOMIC_ACQUIRE);
            if (Tail - Head >= m_Size) {
                return false;
            }
        } while (!AtomicCompareExchangeWeak<UINT64>(&m_TailReserved, &Tail, Tail + 1,
                    __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE));

        m_Buffer[(Tail - 1) % m_Size] = Item;

        while (!AtomicCompareExchangeWeak(&m_Tail, &Tail, Tail + 1,
                    __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE));

        // while (AtomicLoad<UINT64>(&m_Tail, __ATOMIC_ACQUIRE) != Tail);
        // AtomicStore<UINT64>(&m_Tail, Tail + 1, __ATOMIC_RELEASE);
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

constexpr size_t ITEMS_PER_PRODUCER = 1000000;
constexpr int NUM_PRODUCERS = 8;
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
