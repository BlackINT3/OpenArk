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

//#undef _ARKDRV_
#ifdef _ARKDRV_
#include <ntifs.h>
#else
#include <unone.h>
#include <string>
#include <vector>
namespace ArkDrvApi {
namespace Driver {
	bool DriverEnumInfo(std::vector<DRIVER_ITEM> &infos);
} // namespace Memory
} // namespace ArkDrvApi
#endif //_NTDDK_