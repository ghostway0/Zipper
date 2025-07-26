#ifndef ZIPPER_IA64_MM_H_
#define ZIPPER_IA64_MM_H_

struct PML4E {
    UINT64 Read : 1;
    UINT64 Write : 1;
    UINT64 Execute : 1;
    UINT64 Reserved1 : 5;
    UINT64 Accessed : 1;
    UINT64 Ignored1 : 1;
    UINT64 ExecuteForUserMode : 1;
    UINT64 Ignored2 : 1;
    UINT64 PhysicalAddressPFN : 36;
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
    UINT64 PhysicalAddressPFN : 36;
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
    UINT64 PhysicalAddressPFN : 36;
    UINT64 Reserved2 : 4;
    UINT64 Ignored3 : 12;
} __PACKED;

struct PTE {
    UINT64 Read : 1;
    UINT64 Write : 1;
    UINT64 Execute : 1;
    UINT64 MemoryType : 3;
    // Ignore MemoryType
    UINT64 IgnorePAT : 1;
    UINT64 Dirty : 1;
    UINT64 Accessed : 1;
    UINT64 ExecuteForUserMode : 1;
    UINT64 SuppressVE : 1;
    // Aligned to page size
    UINT64 PhysicalAddressPFN : 36;
    UINT64 Reserved : 4;
    UINT64 Ignored : 12;
} __PACKED;

class EPT {
public:
    EPT();

    void Initialize();

    void MapInto(UINT64 VirtFrom, UINT64 VirtTo, UINT64 PhysTo, 
            DWORD Prot, MemoryType Type = MEM_WB);

private:
    PML4E *m_TopLevel;
    UINT8 m_PageTableLevel{4};
};

struct EPTP {
    UINT64 MemoryType : 3;
    UINT64 PageWalkLength : 3;
    UINT64 EnableAccessAndDirtyFlags : 1;
    UINT64 Reserved1 : 5;
    UINT64 PageFrameNumber : 36;
    UINT64 Reserved2 : 16;
};

struct MTRRDefType {
    UINT64 MemoryType : 3;
    UINT64 Reserved1 : 7;
    UINT64 FixedRangeMTRREnable : 1;
    UINT64 MTRREnable : 1;
    UINT64 Reserved2 : 52;
} __PACKED;

void ReadMTRR(UINT64 Address);

#endif // ZIPPER_IA64_MM_H_
