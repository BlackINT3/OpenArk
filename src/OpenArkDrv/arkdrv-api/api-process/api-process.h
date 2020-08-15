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

enum PROCESS_OPS {
	PROCESS_OPEN,
	THREAD_OPEN,
};

#pragma pack(push, 1)
typedef struct _PROCESS_OPEN_INFO {
	DWORD access;
	BOOL inherit;
	DWORD pid;
} PROCESS_OPEN_INFO, *PPROCESS_OPEN_INFO;

typedef struct _THREAD_OPEN_INFO {
	DWORD access;
	BOOL inherit;
	DWORD tid;
} THREAD_OPEN_INFO, *PTHREAD_OPEN_INFO;
#pragma pack(pop)

//#undef _ARKDRV_
#ifdef _ARKDRV_
#include <ntifs.h>
#else
#include <unone.h>
#include <string>
#include <vector>
namespace ArkDrvApi {
namespace Process {
	HANDLE WINAPI OpenProcess(DWORD access, BOOL inherit, DWORD pid);
	HANDLE WINAPI OpenThread(DWORD access, BOOL inherit, DWORD tid);
} // namespace Process
} // namespace ArkDrvApi

#include <common/cpp-wrapper/cpp-wrapper.h>
#define EN_VID_PROCESS() \
	bool regok = UNONE::InterCreateTlsValue(ArkDrvApi::Process::OpenProcess, UNONE::PROCESS_VID);\
	ON_SCOPE_EXIT([&] {if (regok) UNONE::InterDeleteTlsValue(UNONE::PROCESS_VID); });

#endif //_NTDDK_