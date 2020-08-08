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
#include "../arkdrv-api.h"
#ifdef _ARKDRV_
#include <ntifs.h>
#include <windef.h>
#else
#include <Windows.h>
#endif //_NTDDK_

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

//#undef _ARKDRV_
#ifdef _ARKDRV_
#include <ntifs.h>
#else
#include <unone.h>
#include <string>
#include <vector>
namespace ArkDrvApi {
namespace Notify {
	bool NotifyPatch(NOTIFY_TYPE type, ULONG64 routine);
	bool NotifyPatchRegularly(NOTIFY_TYPE type, ULONG64 routine, int interval);
	bool NotifyRemove(NOTIFY_TYPE type, ULONG64 routine);
	bool NotifyRemoveRegularly(NOTIFY_TYPE type, ULONG64 routine, int interval);
	bool NotifyEnumProcess(std::vector<ULONG64> &routines);
	bool NotifyEnumThread(std::vector<ULONG64> &routines);
	bool NotifyEnumImage(std::vector<ULONG64> &routines);
	bool NotifyEnumRegistry(std::vector<ULONG64> &routines);
} // namespace Notify
} // namespace ArkDrvApi
#endif //_NTDDK_