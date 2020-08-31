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
#include "api-storage.h"
#ifdef _ARKDRV_
#else
namespace ArkDrvApi {
namespace Storage {
bool UnlockEnum(const std::wstring &path, std::vector<HANDLE_ITEM> &items)
{
	std::string outdata;
	bool ret = IoControlDriver(IOCTL_ARK_STORAGE, STORAGE_UNLOCK_ENUM, path, outdata);
	if (!ret) return false;
	PHANDLE_INFO info = (PHANDLE_INFO)outdata.c_str();
	for (int i = 0; i < info->count; i++) {
		items.push_back(info->items[i]);
	}
	return true;
}

bool UnlockClose(HANDLE_ITEM &item)
{
	std::string indata;
	std::string outdata;
	indata.assign((char *)&item, sizeof(HANDLE_ITEM));
	bool ret = IoControlDriver(IOCTL_ARK_STORAGE, STORAGE_UNLOCK_CLOSE, indata, outdata);
	if (!ret) return false;
	return true;
}


bool HotkeyRemoveInfo(HOTKEY_ITEM &item)
{
	return false;
}
} // namespace Storage
} // namespace ArkDrvApi
#endif