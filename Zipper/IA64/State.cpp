#include "IA64/State.h"

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
