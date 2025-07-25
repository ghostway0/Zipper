#include "Vmx.tmh"

#define __PACKED __pragma(pack(push, 1))

bool Init() {
    __vmx_vmlaunch();
}
