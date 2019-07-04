/****************************************************************************
**
** Copyright (C) 2019 BlackINT3
** Contact: https://github.com/BlackINT3/OpenArk
**
** GNU Lesser General Public License Usage (LGPL)
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
****************************************************************************/
#pragma once
#ifdef _ARKDRV_
#include <ntifs.h>
#else
#include <Windows.h>
#endif //_NTDDK_

#define ARK_NTDEVICE_NAME L"\\Device\\OpenArkDrv"
#define ARK_DOSDEVICE_NAME L"\\DosDevices\\OpenArkDrv"
#define ARK_USER_SYMBOLINK L"\\\\.\\OpenArkDrv"

#define ARK_DRV_TYPE 41827

#define IOCTL_ARK_HEARTBEAT CTL_CODE(ARK_DRV_TYPE, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_ARK_DRIVER CTL_CODE(ARK_DRV_TYPE, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_ARK_NOTIFY CTL_CODE(ARK_DRV_TYPE, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_ARK_MEMORY CTL_CODE(ARK_DRV_TYPE, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)

// Driver
enum DRIVER_OPS {
	DRIVER_ENUM_INFO,
};
#pragma pack(push, 1)
typedef struct _DRIVER_ITEM {
	ULONG64 base;
	ULONG size;
	ULONG flags;
	USHORT load_seq;
	USHORT init_seq;
	UCHAR  path[256];
} DRIVER_ITEM, *PDRIVER_ITEM;
typedef struct _DRIVER_INFO {
	ULONG count;
	DRIVER_ITEM items[1];
} DRIVER_INFO, *PDRIVER_INFO;
#pragma pack(pop)

// Notify
enum NOTIFY_OPS {
	NOTIFY_PATCH,
	NOTIFY_PATCH_REGULARLY,
	NOTIFY_REMOVE,
	NOTIFY_REMOVE_REGULARLY,
	NOTIFY_ENUM_PROCESS,
	NOTIFY_ENUM_THREAD,
	NOTIFY_ENUM_IMAGE,
	NOTIFY_ENUM_REGISTRY,
};
enum NOTIFY_TYPE {
	CREATE_PROCESS,
	CREATE_THREAD,
	LOAD_IMAGE,
	CM_REGISTRY,
};
#pragma pack(push, 1)
typedef struct _NOTIFY_INFO {
	ULONG count;
	NOTIFY_TYPE type;
	ULONG64 items[1];
} NOTIFY_INFO, *PNOTIFY_INFO;
typedef struct _NOTIFY_REMOVE_INFO {
	NOTIFY_TYPE type;
	ULONG64 item;
} NOTIFY_REMOVE_INFO, *PNOTIFY_REMOVE_INFO;
#pragma pack(pop)

// Memory
enum MEMORY_OPS {
	MEMORY_READ,
	MEMORY_WRITE,
};
#pragma pack(push, 1)
typedef struct _MEMORY_IN {
	ULONG64 addr;
	ULONG size;
	union {
		UCHAR dummy[1];
		UCHAR writebuf[1];
	} u;
} MEMORY_IN, *PMEMORY_IN;
typedef struct _MEMORY_OUT {
	ULONG size;
	UCHAR readbuf[1];
} MEMORY_OUT, *PMEMORY_OUT;
#pragma pack(pop)


#ifdef _ARKDRV_
#include <ntifs.h>
#else
#include <unone.h>
#include <string>
#include <vector>
namespace ArkDrvApi {
bool ConnectDriver();
bool HeartBeatPulse();
bool DriverEnumInfo(std::vector<DRIVER_ITEM> &infos);
bool NotifyPatch(NOTIFY_TYPE type, ULONG64 routine);
bool NotifyPatchRegularly(NOTIFY_TYPE type, ULONG64 routine, int interval);
bool NotifyRemove(NOTIFY_TYPE type, ULONG64 routine);
bool NotifyRemoveRegularly(NOTIFY_TYPE type, ULONG64 routine, int interval);
bool NotifyEnumProcess(std::vector<ULONG64> &routines);
bool NotifyEnumThread(std::vector<ULONG64> &routines);
bool NotifyEnumImage(std::vector<ULONG64> &routines);
bool NotifyEnumRegistry(std::vector<ULONG64> &routines);
bool MemoryRead(ULONG64 addr, ULONG size, std::string &readbuf);
bool MemoryWrite(std::string &writebuf, ULONG64 addr);
} // namespace ArkDrvApi
#endif //_NTDDK_