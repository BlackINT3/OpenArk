#pragma once
#include <ntifs.h>
#include "../iarkdrv/iarkdrv.h"

BOOLEAN InitNotifyDispatcher();
NTSTATUS NotifyDispatcher(IN ULONG op, IN PDEVICE_OBJECT devobj, IN PIRP irp);