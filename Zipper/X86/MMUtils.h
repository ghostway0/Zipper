#ifndef ZIPPER_X86_MMUTILS_H_
#define ZIPPER_X86_MMUTILS_H_

#define PAGE_SIZE (UINT64)4096
#define PAGE_SHIFT (UINT64)12

enum Prot {
    PROT_READ = 1 << 0,
    PROT_WRITE = 1 << 1,
    PROT_EXEC = 1 << 2,
    PROT_RW = PROT_READ | PROT_WRITE,
};

enum CachePolicy : UINT8 {
    // Uncacheable
    CACHE_UC = 0,
    // Write Combining
    CACHE_WC,
    CACHE_RSV1,
    CACHE_RSV2,
    // Write Through
    CACHE_WT,
    // Write Protected
    CACHE_WP,
    // Write Back
    CACHE_WB,
};

PVOID AllocateLockedPages(UINT64 Size, ULONG ProtectFlags = PROT_RW) {
    PVOID Ptr = ExAllocatePoolWithTag(NonPagedPoolNx, Size, "Zipper");
    if (!Ptr) {
        return nullptr;
    }

    PMDL Mdl = IoAllocateMdl(Ptr, Size, FALSE, FALSE, nullptr);
    if (!Mdl) {
        ExFreePool(Ptr);
        return nullptr;
    }

    MmBuildMdlForNonPagedPool(Mdl);
    MmProtectMdlSystemAddress(Mdl, ProtectFlags);

    IoFreeMdl(Mdl);
    return Ptr;
}

PVOID AllocateContiguousPageAligned(UINT64 Size) {
    PHYSICAL_ADDRESS HighestAcceptable;
    HighestAcceptable.QuadPart = MAXULONG64;

    PVOID Allocated = MmAllocateContiguousMemory(Size, HighestAcceptable);
    ZIPPER_ASSERT(Allocated);

    return Allocated;
}

static inline void FreeContiguous(PVOID ptr) {
    if (ptr) {
        MmFreeContiguousMemory(ptr);
    }
}

#endif // ZIPPER_X86_MMUTILS_H_
