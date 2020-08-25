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
#define ARK_HEADER_SIZE(st) (sizeof(st) - sizeof(UCHAR))

#include "../arkdrv-api.h"
#ifdef _ARKDRV_
#include <ntifs.h>
#include <windef.h>
#else
#include <Windows.h>
#endif //_NTDDK_

// Memory
enum ARK_MEMORY_OPS {
	ARK_MEMORY_READ,
	ARK_MEMORY_WRITE,
};
#pragma pack(push, 1)
typedef struct _ARK_MEMORY_IN {
	ULONG pid;
	ULONG64 addr;
	ULONG size;
	union {
		UCHAR dummy[1];
		UCHAR writebuf[1];
	} u;
} ARK_MEMORY_IN, *PARK_MEMORY_IN;
typedef struct _ARK_MEMORY_OUT {
	ULONG pid;
	ULONG size;
	UCHAR readbuf[1];
} ARK_MEMORY_OUT, *PARK_MEMORY_OUT;

typedef struct _ARK_MEMORY_RANGE {
	ULONG64 r3start;
	ULONG64 r3end;
	ULONG64 r0start;
	ULONG64 r0end;
} ARK_MEMORY_RANGE, *PARK_MEMORY_RANGE;
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
	ARK_MEMORY_RANGE MemoryRange();
	bool IsKernelAddress(ULONG64 addr);
	bool MemoryRead(ULONG pid, ULONG64 addr, ULONG size, std::string &readbuf);
	bool MemoryWrite(ULONG pid, ULONG64 addr, std::string &writebuf);
} // namespace Memory
} // namespace ArkDrvApi
#endif //_NTDDK_