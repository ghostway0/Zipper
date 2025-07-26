#include "Vmx.tmh"

#include "Init.h"

#define __PACKED __pragma(pack(push, 1))

bool Init() {
    __vmx_vmlaunch();
}
