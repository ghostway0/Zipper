#ifndef ZIPPER_X86_INTRINSICS_H_
#define ZIPPER_X86_INTRINSICS_H_

#include <emmintrin.h>

inline void Pause() {
    _mm_pause();
}

inline void InterruptContinue() {
    asm volatile ("sti");
}

inline void InterruptStop() {
    asm volatile ("cli");
}

#endif // ZIPPER_X86_INTRINSICS_H_
