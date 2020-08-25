#pragma once
#include <ntifs.h>

EXTERN_C VOID MmEnableWP();
EXTERN_C VOID MmDisableWP();
EXTERN_C VOID MmWriteProtectOn(IN KIRQL Irql);
EXTERN_C KIRQL MmWriteProtectOff();