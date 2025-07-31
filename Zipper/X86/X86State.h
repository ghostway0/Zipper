#ifndef ZIPPER_IA64_STATE_H_
#define ZIPPER_IA64_STATE_H_

#define MAX_NUM_CPUS 64
#define CPU_CURRENT -1

// https://xem.github.io/minix86/manual/intel-x86-and-64-manual-vol3/o_fe12b1e2a880e0ce-1961.html
enum VMExitReason {
    VMEXIT_EXCEPTION_NMI = 0,
    VMEXIT_EXTERNAL_INTERRUPT,
    VMEXIT_TRIPLE_FAULT,
    VMEXIT_INIT,
    VMEXIT_SIPI,
    VMEXIT_IO,
    VMEXIT_SMI,
    VMEXIT_INTERRUPT_WINDOW,
    VMEXIT_NMI_WINDOW,
    VMEXIT_TASK_SWITCH,
    VMEXIT_CPUID,
    VMEXIT_GETSEC,
    VMEXIT_HLT,
    VMEXIT_INVD,
    VMEXIT_INVLPG,
    VMEXIT_RDPMC,
    VMEXIT_RDTSC,
    VMEXIT_RDRSM,
    VMEXIT_VMCALL,
    VMEXIT_VMCLEAR,
    VMEXIT_VMLAUNCH,
    VMEXIT_VMPTRLD,
    VMEXIT_VMPTRST,
    VMEXIT_VMREAD,
};

struct RegisterState {
    UINT64 Rax;
    UINT64 Rbx;
    UINT64 Rcx;
    UINT64 Rdx;
    UINT64 Rsi;
    UINT64 Rdi;
    UINT64 Rsp;
    UINT64 Rbp;
    UINT64 R8;
    UINT64 R9;
    UINT64 R10;
    UINT64 R11;
    UINT64 R12;
    UINT64 R13;
    UINT64 R14;
    UINT64 R15;

    UINT64 Rip;
    UINT64 Rflags;

    UINT64 Cr0;
    UINT64 Cr3;
    UINT64 Cr4;

    UINT64 Dr6;
    UINT64 Dr7;

    UINT16 Es;
    UINT16 EsAttrib;
    UINT32 EsLimit;
    UINT64 EsBase;

    UINT16 Cs;
    UINT16 CsAttrib;
    UINT32 CsLimit;
    UINT64 CsBase;

    UINT16 Ss;
    UINT16 SsAttrib;
    UINT32 SsLimit;
    UINT64 SsBase;

    UINT16 Ds;
    UINT16 DsAttrib;
    UINT32 DsLimit;
    UINT64 DsBase;

    UINT16 Fs;
    UINT16 FsAttrib;
    UINT32 FsLimit;
    UINT64 FsBase;

    UINT16 Gs;
    UINT16 GsAttrib;
    UINT32 GsLimit;
    UINT64 GsBase;

    UINT16 GdtrLimit;
    UINT64 GdtrBase;

    UINT16 IdtrLimit;
    UINT64 IdtrBase;

    UINT16 Ldtr;
    UINT16 LdtrAttrib;
    UINT32 LdtrLimit;
    UINT64 LdtrBase;

    UINT16 Tr;
    UINT16 TrAttrib;
    UINT32 TrLimit;
    UINT64 TrBase;

    UINT64 Efer;
    UINT64 SysenterCs;
    UINT64 SysenterEsp;
    UINT64 SysenterEip;
    UINT64 Star;
    UINT64 Lstar;
    UINT64 Cstar;
    UINT64 Sfmask;
    UINT64 KernelGsBase;
    UINT64 Pat;
} __PACKED;

struct CpuState {
    RegisterState Regs;
    VMExitReason ExitReason;
    BOOL InterruptPending;
    DWORD LastExceptionVector;
};

VMState *GetVMStateForCpu(DWORD Cpu = CPU_CURRENT);

#endif // ZIPPER_IA64_STATE_H_
