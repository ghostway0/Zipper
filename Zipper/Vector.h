#ifndef ZIPPER_VECTOR_H_
#define ZIPPER_VECTOR_H_

#include "MMUtils.h"

template<typename T>
class Vector {
public:
    Vector() = default;

    explicit Vector(ULONG InitialCapacity);

    void EnsureCapacity(ULONG Capacity);

    void EnsureCapacityExact(ULONG NewCapacity) {
        if (Capacity <= m_Capacity) {
            return;
        }

        T *NewBuffer = AllocateVirtualAligned(sizeof(T) * NewCapacity, alignof(T));
        memcpy(NewBuffer, m_Buffer, m_Size);
        m_Capacity = NewCapacity;
    }

    void Push(T const &Value) {
        EnsureCapacity(m_Size + 1);
        m_Buffer[m_Size - 1] = Value;
        ++m_Size;
    }
    
    BOOL Insert(T const &Value, ULONG Index) {
        ZIPPER_ASSERT(Index <= m_Size);

        EnsureCapacity(m_Size + 1);
        memmove(m_Buffer + Index, m_Buffer + Index + 1, m_Size - Index);
        m_Buffer[Index] = Value;
        ++m_Size;
        return TRUE;
    }

    T operator[](ULONG Index) {
        ZIPPER_ASSERT(Index < m_Size);
        return m_Buffer[Index];
    }

private:
    ULONG m_Capacity{0};
    ULONG m_Size{0};
    T *m_Buffer{nullptr};
};

#endif // ZIPPER_VECTOR_H_
