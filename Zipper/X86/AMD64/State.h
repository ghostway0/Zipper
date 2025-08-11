#ifndef ZIPPER_X86_AMD64_STATE_H_
#define ZIPPER_X86_AMD64_STATE_H_

#include "MM.h"
#include "X86State.h"

struct ControlArea {
    UINT64 InterceptCr;
    UINT32 InterceptExceptions;
    UINT32 InterceptInstructions1;
    UINT32 InterceptInstructions2;
    UINT32 InterceptInstructions3;
    UINT8 Reserved1[36];
    UINT16 PauseFilterThreshold;
    UINT16 PauseFilterCount;
    UINT64 IopmBasePa;
    UINT64 MsrpmBasePa;
    UINT64 TscOffset;
    UINT32 GuestASID;
    UINT8 TlbControl;
    UINT8 Reserved2[3];
    UINT64 VirtualIntrControl;
    UINT64 InterruptShadow;
    UINT64 ExitCode;
    UINT64 ExitInfo1;
    UINT64 ExitInfo2;
    UINT64 ExitIntInfo;
    UINT64 NestedPagingEnable;
    UINT64 AvicApicBar;
    UINT64 GuestGhcbPa;
    UINT64 EventInj;
    UINT64 HostCr3;
    UINT64 LbrVirtEnable;
    UINT32 CleanBits;
    UINT32 Reserved3;
    UINT64 NextRip;
    UINT8 GuestBytesFetched;
    UINT8 GuestInstructionBytes[15];
    UINT64 AvicApicBackingPage;
    UINT64 Reserved4;
    UINT64 AvicLogicalTable;
    UINT64 AvicPhysicalTable;
    UINT64 Reserved5;
    UINT64 VmsaPointer;
    UINT8 Reserved6[720];
    UINT32 EnlightenedHypercall;
    UINT32 EnlightenedMsrBitmap;
    UINT32 EnlightenedNptTlb;
    UINT32 EnlightenedClean;
    UINT8 Reserved7[16];
} __PACKED;

struct VMCB {
    ControlArea Control;
    RegisterState SaveState;
} __PACKED;

struct VMState {
    VMCB *GuestControl;
    VMCB *HostControl;
    NPT PageTable;
};

#endif // ZIPPER_X86_AMD64_STATE_H_
