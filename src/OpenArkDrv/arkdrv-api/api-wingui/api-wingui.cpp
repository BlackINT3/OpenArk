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
#include "api-wingui.h"
#ifdef _ARKDRV_
#else
namespace ArkDrvApi {
namespace WinGUI {

bool HotkeyEnumInfo(std::vector<HOTKEY_ITEM> &hotkeys)
{
	if (!ConnectDriver()) return false;
	DWORD op = HOTKEY_ENUM;
	PHOTKEY_INFO info;
	DWORD outlen;
	int hkmarks[HOTKEY_MAX_VK+1] = { 0 };
	for (int i = 1; i <= HOTKEY_MAX_VK; i++) {
		if (RegisterHotKey(NULL, HOTKEY_PLACEHOLDER_ID + i, MOD_ALT | MOD_NOREPEAT, i)) {
			hkmarks[i] = ~i;
		} else {
			OutputDebugStringA(UNONE::StrFormatA("Register err:%s\n", UNONE::OsDosErrorMsgA(GetLastError()).c_str()).c_str());
		}
	}
	bool ret = IoControlDriver(IOCTL_ARK_HOTKEY, op, NULL, 0, (PVOID*)&info, &outlen);
	for (int i = 1; i <= HOTKEY_MAX_VK; i++) {
		if (hkmarks[i]) {
			UnregisterHotKey(NULL, HOTKEY_PLACEHOLDER_ID + i);
		}
	}
	if (!ret) return false;
	for (int i = 0; i < info->count; i++) {
		hotkeys.push_back(info->items[i]);
	}
	free(info);
	return true;
}
bool HotkeyRemoveInfo(HOTKEY_ITEM &item)
{
	DWORD op = HOTKEY_REMOVE;
	DWORD out;
	DWORD outlen;
	bool ret = IoControlDriver(IOCTL_ARK_HOTKEY, op, &item, sizeof(item), (PVOID*)&out, &outlen);
	if (!ret) return false;
	return true;
}
} // namespace WinGUI
} // namespace ArkDrvApi
#endif