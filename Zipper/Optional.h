#ifndef ZIPPER_OPTIONAL_H_
#define ZIPPER_OPTIONAL_H_

#include <new>

#include "Utils.h"

template<typename T>
class Optional {
public:
    Optional() : m_HasValue(false) {}

    Optional(T &&Value) : m_HasValue(true) {
        new (&m_Storage) T(std::move(Value));
    }
    
    Optional(T &Value) : m_HasValue(true) {
        new (&m_Storage) T(Value);
    }

    Optional(const Optional &Other) : m_HasValue(Other.m_HasValue) {
        if (Other.m_HasValue) {
            new (&m_Storage) T(Other.Get());
        }
    }

    constexpr static Optional Null() {
        return Optional<T>();
    }

    const T &Get() {
        ZIPPER_ASSERT(HasValue());
        return *reinterpret_cast<const T*>(&m_Storage);
    }

    const T &Get() const {
        ZIPPER_ASSERT(HasValue());
        return *reinterpret_cast<const T*>(&m_Storage);
    }

    bool HasValue() { return m_HasValue; }

    T ValueOr(const T &Fallback) const {
        return m_HasValue ? Get() : Fallback;
    }

    ~Optional() {
        if (!m_HasValue) {
            return;
        }

        reinterpret_cast<T*>(&m_Storage)->~T();
        m_HasValue = false;
    }

private:
    alignas(T) UINT8 m_Storage[sizeof(T)];
    bool m_HasValue;
};

#endif // ZIPPER_OPTIONAL_H_
