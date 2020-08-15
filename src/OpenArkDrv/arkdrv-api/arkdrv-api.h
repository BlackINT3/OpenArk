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
#include <windef.h>
#else
#include <Windows.h>
#endif //_NTDDK_

#include "api-storage/api-storage.h"
#include "api-memory/api-memory.h"
#include "api-wingui/api-wingui.h"
#include "api-driver/api-driver.h"
#include "api-notify/api-notify.h"
#include "api-object/api-object.h"
#include "api-process/api-process.h"


#define ARK_NTDEVICE_NAME L"\\Device\\OpenArkDrv"
#define ARK_DOSDEVICE_NAME L"\\DosDevices\\OpenArkDrv"
#define ARK_USER_SYMBOLINK L"\\\\.\\OpenArkDrv"

#define ARK_DRV_TYPE 41827

#define IOCTL_ARK_HEARTBEAT CTL_CODE(ARK_DRV_TYPE, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_ARK_DRIVER CTL_CODE(ARK_DRV_TYPE, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_ARK_NOTIFY CTL_CODE(ARK_DRV_TYPE, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_ARK_MEMORY CTL_CODE(ARK_DRV_TYPE, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_ARK_HOTKEY CTL_CODE(ARK_DRV_TYPE, 0x900, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_ARK_STORAGE CTL_CODE(ARK_DRV_TYPE, 0x920, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_ARK_OBJECT CTL_CODE(ARK_DRV_TYPE, 0x940, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_ARK_PROCESS CTL_CODE(ARK_DRV_TYPE, 0x960, METHOD_BUFFERED, FILE_ANY_ACCESS)

//#undef _ARKDRV_
#ifdef _ARKDRV_
#include <ntifs.h>
#else
#include <unone.h>
#include <string>
#include <vector>
namespace ArkDrvApi {
extern HANDLE arkdrv;
bool ConnectDriver();
bool DisconnectDriver();
bool HeartBeatPulse();
bool IoControlDriver(DWORD ctlcode, DWORD op, PVOID inbuf, DWORD inlen, PVOID *outbuf, DWORD *outlen);
bool IoControlDriver(DWORD ctlcode, DWORD op, const std::wstring &indata, std::string &outdata);
bool IoControlDriver(DWORD ctlcode, DWORD op, const std::string &indata, std::string &outdata);

#define TO_STREAM(st) std::string((char*)&st, sizeof(st))
#define TO_STREAM_P(pst, size) std::string((char*)pst, size)
#define TO_STRUCT(str, type) ((type)str.c_str())

} // namespace ArkDrvApi
#endif //_NTDDK_