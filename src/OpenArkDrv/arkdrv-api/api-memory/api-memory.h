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

//#undef _ARKDRV_
#ifdef _ARKDRV_
#include <ntifs.h>
#else
#include <unone.h>
#include <string>
#include <vector>
namespace ArkDrvApi {
namespace Memory {
	bool MemoryRead(ULONG64 addr, ULONG size, std::string &readbuf);
	bool MemoryWrite(std::string &writebuf, ULONG64 addr);
} // namespace Memory
} // namespace ArkDrvApi
#endif //_NTDDK_