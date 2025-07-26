NPT::NPT() {
    Initialize();
}

void NPT::Initialize() {
    UINT64 Allocated = AllocateContiguousPageAligned(PAGE_SIZE);
    ZIPPER_ASSERT(Allocated && AlignedTo(PAGE_SIZE, Allocated));

    RtlZeroMemory(Allocated, PAGE_SIZE);

    VMState *VMState = GetVMStateForCpu();
    VMState->Control.NPEnable = 1;
    VMState->Control.HCr3 = Allocated >> PAGE_SHIFT;

    Control.FlushInvalidation = 1;
}

bool NPT::MapInto(UINT64 VirtFrom, UINT64 VirtTo, UINT64 PhysTo, 
        DWORD Prot, MemoryType Type) {
    if (!AlignedTo(PAGE_SIZE, VirtFrom) ||
        !AlignedTo(PAGE_SIZE, VirtTo) ||
        !AlignedTo(PAGE_SIZE, PhysTo)) {
        return false;
    }

    InterruptStop();

    auto WalkPage = [](auto *Entry) -> UINT64 {
        if (!Entry->Present) {
            UINT64 Allocated = AllocateContiguousPageAligned(PAGE_SIZE);
            ZIPPER_ASSERT(Allocated);

            RtlZeroMemory(Allocated, PAGE_SIZE);
            Entry->PhysicalAddressPFN = GetPhysicalAddress(Allocated) >> 12;
            Entry->Present = 1;
            Entry->Write = 1;
            Entry->User = 1; // ?
            Entry->NX = 0;
        }

        return Entry->PhysicalAddressPFN << 12;
    };

    UINT64 CurrentVirt = VirtFrom;
    UINT64 CurrentPhys = PhysTo;

    for (; CurrentVirt < VirtTo; CurrentVirt += PAGE_SIZE, CurrentPhys += PAGE_SIZE) {
        UINT64 I0 = (CurrentVirt >> 39) & 0x1FF;
        UINT64 I1 = (CurrentVirt >> 30) & 0x1FF;
        UINT64 I2 = (CurrentVirt >> 21) & 0x1FF;
        UINT64 I3 = (CurrentVirt >> 12) & 0x1FF;

        NPTE *L1 = (NPTE *)WalkPage(m_TopLevel + I0);
        NPTE *L2 = (NPTE *)WalkPage(L1 + I1);
        NPTE *L3 = (NPTE *)WalkPage(L2 + I2);
        NPTE *PageEntry = L3 + I3;

        PageEntry->PhysicalAddressPFN = CurrentPhys >> 12;
        PageEntry->Present = (Prot & PROT_READ) != 0;
        PageEntry->Write = (Prot & PROT_WRITE) != 0;
        PageEntry->Execute = !(Prot & PROT_EXEC);
        PageEntry->User = 1;
        PageEntry->MemoryType = Type;
        PageEntry->Accessed = 1;
        PageEntry->Dirty = 1;

        switch (Type) {
            case CACHE_WB:
                PageEntry.PWT = 0; 
                PageEntry.PCD = 0;
                PageEntry.PAT = 0;
                break;
            case CACHE_WC:
                PageEntry.PWT = 1;
                PageEntry.PCD = 0;
                PageEntry.PAT = 0;
                break;
            case CACHE_UC:
                PageEntry.PWT = 1;
                PageEntry.PCD = 1;
                PageEntry.PAT = 0;
                break;
            default:
                ZIPPER_UNREACHABLE("Unhandled cache type");
        }
    }

    InterruptContinue();
    return true;
}
