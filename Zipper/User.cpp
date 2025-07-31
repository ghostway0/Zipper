#define IOCTL_PROTECT_REGION CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_WRITE_DATA)
#define IOCTL_LOCK_CONFIG CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_WRITE_DATA)

#define FILTER_CODE_SIZE 64

struct ProtectRequest {
    UINT64 Address;
    UINT64 Size;
    UINT32 Protections;
    UINT8 Filter[FILTER_CODE_SIZE];
};

struct ProtTable {
    PROTECTED_REGION* Regions;
    ULONG Count;
    UCHAR *FilterBlob;
    ULONG FilterBlobSize;
};

ProtTable RegionTable;

PROTECTED_REGION* FindRegion(UINT64 address) {
    UINT64 Min = 0, Max = RegionTable.Count - 1;

    while (Min <= Max) {
        LONG mid = (Min + Max) / 2;
        PROTECTED_REGION* region = &g_RegionTable.Regions[mid];

        if (address < region->Start)
            right = mid - 1;
        else if (address >= region->End)
            left = mid + 1;
        else
            return region; // address ∈ [Start, End)
    }

    return NULL;
}

NTSTATUS DeviceControl(PDEVICE_OBJECT /*DeviceObject*/, PIRP Irp) {
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status = STATUS_SUCCESS;
    ULONG_PTR info = 0;

    UINT64 InputSize = irpSp->Parameters.DeviceIoControl.InputBufferLength;
    UINT64 IoControlCode = irpSp->Parameters.DeviceIoControl.IoControlCode;
    switch (IoControlCode) {
        case IOCTL_PROTECT_REGION:
            if (InputSize < sizeof(PROTECT_REQUEST)) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }
            ProtectRequest *req = (ProtectRequest *)Irp->AssociatedIrp.SystemBuffer;
            break;
        case IOCTL_LOCK_CONFIG:
            if (InputSize < sizeof(PROTECT_REQUEST)) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }
            AtomicStore(&ConfigLocked, TRUE);
            break;
        default:
            status = STATUS_INVALID_DEVICE_REQUEST;
    }

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = info;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}
