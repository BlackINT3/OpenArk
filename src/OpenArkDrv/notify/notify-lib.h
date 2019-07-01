#pragma once
#include <ntifs.h>

BOOLEAN GetProcessNotifyInfo(ULONG &count, PULONG64 &items);