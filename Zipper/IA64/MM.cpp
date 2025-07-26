#include "MM.h"
#include "INtrinsics.h"

EPT::EPT() {
    Initialize();
}

void EPT::Initialize() {
    PHYSICAL_ADDRESS HighestAddr;
    HighestAddr.QuadPart = ~0ull;
    UINT64 Allocated = MmAllocateContiguousMemory(PAGE_SIZE, HighestAddr);
    ZIPPER_ASSERT(Allocated && AlignedTo(Allocated, PAGE_SIZE));

    RtlZeroMemory(Allocated, PAGE_SIZE);


    EPTP EptPointer{
        .MemoryType = MEM_WB,
        .PageWalkLength = m_PageTableLevel - 1,
        .EnableAccessAndDirtyFlags = 1,
        .TopLevelPfn = Allocated >> 12,
    };

    __vmx_vmwrite(VMX_EPT_POINTER, *(UINT64 *)&EptPointer);
}

bool EPT::MapInto(UINT64 VirtFrom, UINT64 VirtTo, UINT64 PhysTo, 
        DWORD Prot, MemoryType Type = MEM_WB) {
    if (!AlignedTo(PAGE_SIZE, VirtFrom) ||
        !AlignedTo(PAGE_SIZE, VirtTo) ||
        !AlignedTo(PAGE_SIZE, PhysTo)) {
        return false;
    }

    InterruptStop(); // lol this is _probably_ needed

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
        PTE *PT = (PTE *)WalkPage(PD + PDI);
        PTE *PageEntry = PT + PTI;

        PageEntry->PhysicalAddress = CurrentPhys >> 12;
        PageEntry->Execute = (Prot & PROT_EXEC) != 0;
        PageEntry->Write = (Prot & PROT_WRITE) != 0;
        PageEntry->Read = (Prot & PROT_READ) != 0;
        PageEntry->MemoryType = Type;
        PageEntry->Accessed = 1;
        PageEntry->Dirty = 1;
    }
    
    InterruptContinue();

    return true;
}
