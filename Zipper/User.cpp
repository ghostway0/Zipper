#define IOCTL_PROTECT_REGION CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_WRITE_DATA)
#define IOCTL_LOCK_CONFIG CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_WRITE_DATA)

#define FILTER_CODE_SIZE 64

struct ProtectRequest {
    UINT64 Address;
    UINT64 Size;
    UINT32 Protections;
    UINT8 Filter[FILTER_CODE_SIZE];
};

struct ProtectedRegion {
    UINT64 Start;
    UINT64 End;
};

struct ProtTable {
    ProtectedRegion *Regions;
    ULONG Count;
    UCHAR *FiltersBlob;
    ULONG FiltersBlobSize;
};

enum Opcode {
    OP_STOP,
    OP_PUSHI,
    OP_PUSHR,
    OP_ADD,
    OP_SUB,
    OP_SHL,
    OP_SHR,
};

struct Operation {
    DWORD Op : 8;
    DWORD Imm : 24;
};

template <typename T, ULONG Size>
class Stack {
    Stack() = default;

    Optional<T> Pop() {
        if (m_Cursor == 0) {
            return Optional<T>::Null();
        }

        --m_Cursor;
        return m_Buffer[m_Cursor];
    }

    BOOL Push(T Value) {
        if (m_Cursor >= Size) {
            return FALSE;
        }

        m_Buffer[m_Cursor] = Value;
        ++m_Cursor;

        return TRUE;
    }

private:
    T m_Buffer[Size];
    ULONG m_Cursor{0};
};

BOOL RunFilterVM(Operation *Code, PVOID EndPtr = NULL) {
    BOOL Running = TRUE;

    Stack<UINT64> VMStack;

    do {
        Operation Current = *Code;

        switch (Current.Op) {
            case OP_STOP:
                Running = FALSE;
                break;
            case OP_PUSHI:
                VMStack.Push((UINT64)Current.Imm);
                break;
            case OP_PUSHR:
                // push guest register value
                break;
        }

        ++Code;
    } while ((EndPtr == NULL || Code < (Operation *)EndPtr) && Running);

    return VMStack.Pop().ValueOr(FALSE);
}

ProtTable RegionTable;
UINT64 ConfigLocked = FALSE;

ProtectedRegion *FindRegion(UINT64 Address) {
    if (RegionTable.Count == 0) {
        return NULL;
    }

    ULONG Min = 0, Max = RegionTable.Count - 1;

    while (Min <= Max) {
        ULONG Mid = (Min + Max) / 2;
        ProtectedRegion *Region = &RegionTable.Regions[Mid];

        if (Address < Region->Start) {
            Max = Mid - 1;
        } else if (Address >= Region->End) {
            Min = Mid + 1;
        } else {
            return Region;
        }
    }

    return NULL;
}

NTSTATUS DeviceControl(PDEVICE_OBJECT /*DeviceObject*/, PIRP Irp) {
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS Status = STATUS_SUCCESS;

    UINT64 InputSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
    UINT64 IoControlCode = IrpSp->Parameters.DeviceIoControl.IoControlCode;
    switch (IoControlCode) {
        case IOCTL_PROTECT_REGION:
            if (InputSize < sizeof(ProtectRequest)) {
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            if (AtomicLoad(&ConfigLocked)) {
                Status = STATUS_WAS_LOCKED;
                break;
            }

            ProtectRequest *Request = (ProtectRequest *)Irp->AssociatedIrp.SystemBuffer;
            break;
        case IOCTL_LOCK_CONFIG:
            AtomicStore(&ConfigLocked, TRUE);
            break;
        default:
            Status = STATUS_INVALID_DEVICE_REQUEST;
    }

    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Status;
}
