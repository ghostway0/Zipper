#include "Vmx.tmh"

#define __PACKED __pragma(pack(push, 1))

bool Init() {
    __vmx_vmlaunch();
}

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
    UINT64 Cr2;
    UINT64 Cr3;
    UINT64 Cr4;

    UINT64 Dr0;
    UINT64 Dr1;
    UINT64 Dr2;
    UINT64 Dr3;
    UINT64 Dr6;
    UINT64 Dr7;

    UINT16 Cs;
    UINT16 Ds;
    UINT16 Es;
    UINT16 Fs;
    UINT16 Gs;
    UINT16 Ss;

    UINT64 FsBase;
    UINT64 GsBase;

    UINT64 Efer;
} __PACKED;

struct CpuState {
    RegisterState Regs;
    VMExitReason ExitReason;
    bool InterruptPending;
    DWORD LastExceptionVector;
};

enum Prot {
    PROT_READ = 1 << 0,
    PROT_WRITE = 1 << 1,
    PROT_EXEC = 1 << 2,
};

enum EptMemoryType : UINT8 {
    // Uncacheable
    EPT_MEM_UC = 0,
    // Write Combining
    EPT_MEM_WC,
    EPT_MEM_RSV1,
    EPT_MEM_RSV2,
    // Write Through
    EPT_MEM_WT,
    // Write Protected
    EPT_MEM_WP,
    // Write Back
    EPT_MEM_WB,
};

struct PML4E {
    UINT64 Read : 1;
    UINT64 Write : 1;
    UINT64 Execute : 1;
    UINT64 Reserved1 : 5;
    UINT64 Accessed : 1;
    UINT64 Ignored1 : 1;
    UINT64 ExecuteForUserMode : 1;
    UINT64 Ignored2 : 1;
    UINT64 PhysicalAddress : 36;
    UINT64 Reserved2 : 4;
    UINT64 Ignored3 : 12;
} __PACKED;

struct PDPTE {
    UINT64 Read : 1;
    UINT64 Write : 1;
    UINT64 Execute : 1;
    UINT64 Reserved1 : 5;
    UINT64 Accessed : 1;
    UINT64 Ignored1 : 1;
    UINT64 ExecuteForUserMode : 1;
    UINT64 Ignored2 : 1;
    UINT64 PhysicalAddress : 36;
    UINT64 Reserved2 : 4;
    UINT64 Ignored3 : 12;
} __PACKED;

struct PDE {
    UINT64 Read : 1;
    UINT64 Write : 1;
    UINT64 Execute : 1;
    UINT64 Reserved1 : 5;
    UINT64 Accessed : 1;
    UINT64 Ignored1 : 1;
    UINT64 ExecuteForUserMode : 1;
    UINT64 Ignored2 : 1;
    UINT64 PhysicalAddress : 36;
    UINT64 Reserved2 : 4;
    UINT64 Ignored3 : 12;
} __PACKED;

struct PTE {
    UINT64 Read : 1;
    UINT64 Write : 1;
    UINT64 Execute : 1;
    EPTMemoryType MemoryType : 3;
    // Ignore MemoryType
    UINT64 IgnorePAT : 1;
    UINT64 Dirty : 1;
    UINT64 Accessed : 1;
    UINT64 ExecuteForUserMode : 1;
    UINT64 SuppressVE : 1;
    // Aligned to page size
    UINT64 PhysicalAddress : 36;
    UINT64 Reserved : 4;
    UINT64 Ignored : 12;
} __PACKED;

class EPT {
public:
    void MapInto(UINT64 VirtFrom, UINT64 VirtTo, UINT64 PhysTo, 
            DWORD Prot, EptMemoryType MemType = EPT_MEM_WB);

private:
    PML4E *m_TopLevel;
    UINT8 m_PageTableLevel;
};

struct VMState {
    UINT64 VmmStack;
    EPT *Ept;
};

bool EPT::MapInto(UINT64 VirtFrom, UINT64 VirtTo, UINT64 PhysTo, 
        DWORD Prot, EptMemoryType MemType = EPT_MEM_WB) {
    if (!AlignedTo(PAGE_SIZE, VirtFrom) ||
        !AlignedTo(PAGE_SIZE, VirtTo) ||
        !AlignedTo(PAGE_SIZE, PhysTo)) {
        return false;
    }

    auto WalkPage = [](auto *Entry) -> UINT64 {
        if (!Entry->PhysicalAddress) {
            PHYSICAL_ADDRESS HighestAddr;
            HighestAddr.QuadPart = ~0ull;

            UINT64 Allocated = MmAllocateContiguousMemory(PAGE_SIZE, HighestAddr);
            ZIPPER_ASSERT(Allocated);

            RtlZeroMemory(Allocated, PAGE_SIZE);
            Entry->PhysicalAddress = GetPhysicalAddress(Allocated) >> 12;
            Entry->Read = 1;
            Entry->Write = 1;
            Entry->Execute = 1;
        }
        return Entry->PhysicalAddress << 12;
    };

    UINT64 CurrentVirt = VirtFrom;
    UINT64 CurrentPhys = PhysTo;
    for (; CurrentVirt < VirtTo; CurrentVirt += PAGE_SIZE, CurrentPhys += PAGE_SIZE) {
        UINT64 PML4I = (CurrentVirt >> 39) & 0x1FF;
        UINT64 PDPTI = (CurrentVirt >> 30) & 0x1FF;
        UINT64 PDI = (CurrentVirt >> 21) & 0x1FF;
        UINT64 PTI = (CurrentVirt >> 12) & 0x1FF;

        PDPTE *PDPT = (PDPTE *)WalkPage(m_TopLevel + PML4I);
        PDE *PD = (PDE *)WalkPage(PDPT + PDPTI);
        PTE *PT = (PTE *)WalkPage(PD + PDI) + PTI;

        PT->PhysicalAddress = CurrentPhys >> 12;
        PT->Execute = (Prot & PROT_EXEC) != 0;
        PT->Write = (Prot & PROT_WRITE) != 0;
        PT->Read = (Prot & PROT_READ) != 0;
        PT->MemoryType = MemType;
        PT->Accessed = 1;
        PT->Dirty = 1;
    }

    return true;
}

#define MAX_NUM_CPUS 64
#define CPU_CURRENT -1

static VMState VMStates[MAX_NUM_CPUS];

VMState *GetVMStateForCpu(DWORD Cpu = CPU_CURRENT) {
    if (Cpu < 0) {
        Cpu = KeGetCurrentProcessorNumber();
    }

    if (Cpu >= MAX_NUM_CPUS) {
        return nullptr;
    }

    return &VMStates[Cpu];
}
