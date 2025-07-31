#ifndef ZIPPER_INTRINSICS_H_
#define ZIPPER_INTRINSICS_H_

#define _AlignOf(x) __alignof(x)

extern void Pause();

extern void InterruptContinue();

extern void InterruptStop();

#if defined(__x86_64__) || defined(__i386__) || defined(_M_IX86) || defined(_M_X64)

#include "X86/Intrinsics.h"

#endif // __x86_64__

#endif // ZIPPER_INTRINSICS_H_
