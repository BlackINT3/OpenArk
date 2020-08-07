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
#include "api-notify.h"
#ifdef _ARKDRV_
#else
namespace ArkDrvApi {
namespace Notify {

bool NotifyPatch(NOTIFY_TYPE type, ULONG64 routine);
bool NotifyPatchRegularly(NOTIFY_TYPE type, ULONG64 routine, int interval);
bool NotifyRemove(NOTIFY_TYPE type, ULONG64 routine)
{
	if (routine == 0) return false;
	NOTIFY_REMOVE_INFO info;
	info.type = type;
	info.item = routine;
	bool ret = IoControlDriver(IOCTL_ARK_NOTIFY, NOTIFY_REMOVE, &info, sizeof(info), NULL, 0);
	return ret;
}
bool NotifyRemoveRegularly(NOTIFY_TYPE type, ULONG64 routine, int interval);
bool NotifyEnum(DWORD op, std::vector<ULONG64> &routines)
{
	routines.clear();
	PNOTIFY_INFO notify;
	DWORD outlen;
	bool ret = IoControlDriver(IOCTL_ARK_NOTIFY, op, NULL, 0, (PVOID*)&notify, &outlen);
	if (!ret) return false;
	for (int i = 0; i < notify->count; i++) {
		routines.push_back(notify->items[i]);
	}
	free(notify);
	return true;
}
bool NotifyEnumProcess(std::vector<ULONG64> &routines)
{
	return NotifyEnum(NOTIFY_ENUM_PROCESS, routines);
}
bool NotifyEnumThread(std::vector<ULONG64> &routines)
{
	return NotifyEnum(NOTIFY_ENUM_THREAD, routines);
}
bool NotifyEnumImage(std::vector<ULONG64> &routines)
{
	return NotifyEnum(NOTIFY_ENUM_IMAGE, routines);
}
bool NotifyEnumRegistry(std::vector<ULONG64> &routines)
{
	return NotifyEnum(NOTIFY_ENUM_REGISTRY, routines);
}
} // namespace Memory
} // namespace ArkDrvApi
#endif