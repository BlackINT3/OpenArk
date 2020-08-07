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

// WinGUI
#define HOTKEY_MAX_VK	0x80
#define HOTKEY_PLACEHOLDER_ID 0x99887766
enum HOTKEY_OPS {
	HOTKEY_ENUM,
	HOTKEY_REMOVE,
};
#pragma pack(push, 1)
typedef struct _HOTKEY_ITEM {
	UCHAR name[64];
	UINT32 wnd;
	UINT16 mod1;
	UINT16 mod2;
	UINT32 vk;
	UINT32 id;
	UINT32 pid;
	UINT32 tid;
	ULONG64 hkobj;
} HOTKEY_ITEM, *PHOTKEY_ITEM;
typedef struct _HOTKEY_INFO {
	ULONG count;
	HOTKEY_ITEM items[1];
} HOTKEY_INFO, *PHOTKEY_INFO;
#pragma pack(pop)

//#undef _ARKDRV_
#ifdef _ARKDRV_
#include <ntifs.h>
#else
#include <unone.h>
#include <string>
#include <vector>
namespace ArkDrvApi {
namespace WinGUI {
	bool HotkeyEnumInfo(std::vector<HOTKEY_ITEM> &hotkeys);
	bool HotkeyRemoveInfo(HOTKEY_ITEM &item);
} // namespace Memory
} // namespace ArkDrvApi
#endif //_NTDDK_