#ifndef ZIPPER_X86_AMD64_MM_H_
#define ZIPPER_X86_AMD64_MM_H_

struct NPTE {
    UINT64 Present : 1;
    UINT64 Write : 1;
    UINT64 User : 1;
    UINT64 PWT : 1;
    UINT64 PCD : 1;
    UINT64 Accessed : 1;
    UINT64 Dirty : 1;
    UINT64 PageSize : 1;
    UINT64 Global : 1;
    UINT64 Available : 3;
    UINT64 PAT : 1;
    UINT64 NX : 1;
    UINT64 PhysicalAddressPFN : 40;
    UINT64 Reserved : 12;
};

class NPT {
public:
    NPT();

    void Initialize();

    void MapInto(UINT64 VirtFrom, UINT64 VirtTo, UINT64 PhysTo, 
            DWORD Prot, CachePolicy Policy = CACHE_WB);

private:
    NPTE *m_TopLevel;
};

#endif // ZIPPER_X86_AMD64_MM_H_
