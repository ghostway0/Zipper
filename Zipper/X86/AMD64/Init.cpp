#include "State.h"
#include "MMUtils.h"

template<typename Callback, typename T>
BOOL RunPerCpuGlobalContext(T Context) {
    BOOL Failed = FALSE;

    ULONG CpuCount = KeQueryActiveProcessorCount(NULL);
    for (ULONG Cpu = 0; Cpu < CpuCount; Cpu++) {
        KAFFINITY AffinityMask = ((KAFFINITY)1) << Cpu;
        KeSetSystemAffinityThread(AffinityMask);

        Failed |= !Callback(Context);

        KeRevertToUserAffinityThread();
    }

    return !Failed;
}

template<typename Callback, typename T>
BOOL RunPerCpuLocalContext(T Contexts[], ULONG NumContexts) {
    BOOL Failed = FALSE;

    ULONG CpuCount = KeQueryActiveProcessorCount(NULL);
    for (ULONG Cpu = 0; Cpu < CpuCount; Cpu++) {
        KAFFINITY AffinityMask = ((KAFFINITY)1) << Cpu;
        KeSetSystemAffinityThread(AffinityMask);

        PVOID Context = (Cpu < NumContexts) ? Contexts[Cpu] : NULL;
        Failed |= !Callback(Context);

        KeRevertToUserAffinityThread();
    }

    return !Failed;
}

struct SegmentDescriptor {
    UINT16 LimitLow;
    UINT16 BaseLow;
    UINT8 BaseMiddle;
    UINT8 Type : 4;
    UINT8 System : 1;
    UINT8 Dpl : 2;
    UINT8 Present : 1;
    UINT8 LimitHigh : 4;
    UINT8 Avl : 1;
    UINT8 LongMode : 1;
    UINT8 DefaultBit : 1;
    UINT8 Granularity : 1;
    UINT8 BaseHigh;
} __PACKED;

union SegmentAttribute {
    struct {
        UINT16 Type : 4;
        UINT16 System : 1;
        UINT16 Dpl : 2;
        UINT16 Present : 1;
        UINT16 Avl : 1;
        UINT16 LongMode : 1;
        UINT16 DefaultBit : 1;
        UINT16 Granularity : 1;
        UINT16 Reserved1 : 4;
    } __PACKED;
    UINT16 AsUInt16;
} __PACKED;

SegmentAttribute GetSegmentRegisterAccess(UINT16 SegmentSelector, ULONG* GdtBase) {
    UINT16 SelectorIndex = SegmentSelector & ~0x7;
    SegmentDescriptor *Descriptor = reinterpret_cast<SegmentDescriptor *>(
            reinterpret_cast<ULONG_PTR>(GdtBase) + SelectorIndex);

    SegmentAttribute Attribute = {0};
    Attribute.Fields.Type        = Descriptor->Fields.Type;
    Attribute.Fields.System      = Descriptor->Fields.System;
    Attribute.Fields.Dpl         = Descriptor->Fields.Dpl;
    Attribute.Fields.Present     = Descriptor->Fields.Present;
    Attribute.Fields.Avl         = Descriptor->Fields.Avl;
    Attribute.Fields.LongMode    = Descriptor->Fields.LongMode;
    Attribute.Fields.DefaultBit  = Descriptor->Fields.DefaultBit;
    Attribute.Fields.Granularity = Descriptor->Fields.Granularity;

    return Attribute;
}

RegisterState CaptureRegisterState() {
    DESCRIPTOR_TABLE_REGISTER Gdtr, Idtr;
    RegisterState State = {0};

    PCONTEXT ContextRecord = RtlCaptureContext();

    _sgdt(&Gdtr);
    __sidt(&Idtr);

    Save.GdtrLimit = Gdtr.Limit;
    State.GdtrBase = Gdtr.Base;
    State.IdtrLimit = Idtr.Limit;
    State.IdtrBase = Idtr.Base;

    State.CsLimit = GetSegmentLimit(ContextRecord->SegCs);
    State.DsLimit = GetSegmentLimit(ContextRecord->SegDs);
    State.EsLimit = GetSegmentLimit(ContextRecord->SegEs);
    State.SsLimit = GetSegmentLimit(ContextRecord->SegSs);

    State.Cs = ContextRecord->SegCs;
    State.Ds = ContextRecord->SegDs;
    State.Es = ContextRecord->SegEs;
    State.Ss = ContextRecord->SegSs;

    State.CsAttrib = GetSegmentRegisterAccess(ContextRecord->SegCs, Gdtr.Base);
    State.DsAttrib = GetSegmentRegisterAccess(ContextRecord->SegDs, Gdtr.Base);
    State.EsAttrib = GetSegmentRegisterAccess(ContextRecord->SegEs, Gdtr.Base);
    State.SsAttrib = GetSegmentRegisterAccess(ContextRecord->SegSs, Gdtr.Base);

    State.Rflags = ContextRecord->EFlags;
    State.Rsp    = ContextRecord->Rsp;
    State.Rip    = ContextRecord->Rip;

    State.Cr0 = __readcr0();
    State.Cr2 = __readcr2();
    State.Cr3 = __readcr3();
    State.Cr4 = __readcr4();

    State.Efer = __readmsr(IA32_MSR_EFER);
    State.Pat  = __readmsr(IA32_MSR_PAT);

    State.SysenterCs  = __readmsr(IA32_SYSENTER_CS);
    State.SysenterEsp = __readmsr(IA32_SYSENTER_ESP);
    State.SysenterEip = __readmsr(IA32_SYSENTER_EIP);

    State.Star         = __readmsr(IA32_STAR);
    State.Lstar        = __readmsr(IA32_LSTAR);
    State.Cstar        = __readmsr(IA32_CSTAR);
    State.Sfmask       = __readmsr(IA32_FMASK);
    State.KernelGsBase = __readmsr(IA32_KERNEL_GS_BASE);

    return State;
}

void SetupGuestVMCB(VMState *VM) {
    VMCB *ControlBlock = VM->GuestControl;

    ControlBlock->Control.InterceptMisc2 |= SVM_INTERCEPT_MISC2_VMRUN;

    // We have one guest
    ControlBlock->Control.GuestAsid = 1;

    ControlBlock->Control.NestedPagingEnable |= SVM_NP_ENABLE_NP_ENABLE;
    ControlBlock->Control.HostCr3 = VM->PageTable.TopLevelPhysicalAddress();

    ControlBlock->SaveState = CaptureRegisterState();

    __svm_vmsave();
}

BOOL Init() {
    // Allocates and is trivially copyable.
    NPT PageTable;

    RunPerCpuLocalContext([](PVOID /*Context*/) {
        VMState *State = GetVMStateForCpu();

        State->GuestControl = (VMCB *)AllocateContiguousPageAligned(PAGE_SIZE);
        State->HostControl = (VMCB *)AllocateContiguousPageAligned(PAGE_SIZE);
        State->PageTable = PageTable;

        SetupGuestVMCB(State->GuestControl);
        // SetupHostVMCB(State->HostControl);
    });
}
