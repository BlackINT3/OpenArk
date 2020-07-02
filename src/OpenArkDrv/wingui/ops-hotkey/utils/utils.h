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
#include <ntifs.h>
#include <windef.h>

extern "C" {
	typedef BOOLEAN(NTAPI* __NtUserRegisterHotKey)(HWND hWnd, int id, UINT fsModifiers, UINT vk);
	typedef BOOLEAN(NTAPI* __NtUserUnregisterHotKey)(HWND hWnd, int id);
}

NTSTATUS GetSectionRegion(PUCHAR base, CHAR* name, PUCHAR& start, ULONG& size);

PVOID GetSystemModuleBase(IN char* modname, OUT PULONG imagesize);

BOOLEAN OsGetVersionInfo(IN OUT RTL_OSVERSIONINFOEXW& info);