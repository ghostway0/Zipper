#include "Vector.h"

namespace {

ULONG FindCapacity(ULONG Old, ULONG Needed) {
    if (Old > Needed) {
        return Old;
    }

    return Max(Needed, Old + Old / 2);
}

} // namespace

Vector::Vector(ULONG InitialCapacity) {
    EnsureCapacity(InitialCapacity);
}

Vector::EnsureCapacity(ULONG Capacity) {
    EnsureCapacityExact(FindCapacity(m_Capacity, Capacity));
}
