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
#include "arkdrv-api.h"
#ifdef _ARKDRV_
#else
namespace ArkDrvApi {
HANDLE arkdrv = INVALID_HANDLE_VALUE;

bool ConnectDriver()
{
	if (arkdrv != INVALID_HANDLE_VALUE) return true;
	arkdrv = CreateFileW(
		ARK_USER_SYMBOLINK,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
	if (arkdrv == INVALID_HANDLE_VALUE) {
		return false;
	}
	return true;
}

bool IoControlDriver(DWORD ctlcode, DWORD op, PVOID inbuf, DWORD inlen, PVOID *outbuf, DWORD *outlen)
{
	DWORD retlen = 0;

	if (arkdrv == INVALID_HANDLE_VALUE)
		return false;

	DWORD wrap_inlen = sizeof(op) + inlen;
	PUCHAR wrap_inbuf = (PUCHAR)malloc(wrap_inlen);
	if (!wrap_inbuf) return false;
	memcpy(wrap_inbuf, &op, sizeof(op));
	if (inbuf) memcpy(wrap_inbuf + sizeof(op), inbuf, inlen);

	bool ret = DeviceIoControl(
				arkdrv,
				ctlcode,
				wrap_inbuf,
				wrap_inlen,
				NULL,
				0,
				&retlen,
				NULL);
	if (ret) {
		free(wrap_inbuf);
		return true;
	}

	if (GetLastError() != ERROR_MORE_DATA) {
		free(wrap_inbuf);
		return false;
	}

	*outbuf = NULL;
	auto bufsize = retlen;
	auto buf = (PDRIVER_INFO)calloc(bufsize, 1);
	if (!buf) return false;
	if (!DeviceIoControl(
		arkdrv,
		ctlcode,
		wrap_inbuf,
		wrap_inlen,
		buf,
		bufsize,
		&retlen,
		NULL)) {
		free(buf);
		free(wrap_inbuf);
		return false;
	}
	*outbuf = buf;
	*outlen = retlen;
	free(wrap_inbuf);
	return true;
}

bool HeartBeatPulse()
{
	if (!ConnectDriver()) return false;

	PVOID outbuf;
	DWORD outlen;
	bool ret = IoControlDriver(IOCTL_ARK_HEARTBEAT, 0, NULL, 0, &outbuf, &outlen);
	return ret;
}

bool DriverEnumInfo(std::vector<DRIVER_ITEM> &infos)
{
	infos.clear();
	DWORD op = DRIVER_ENUM_INFO;
	PDRIVER_INFO drivers;
	DWORD outlen;
	bool ret = IoControlDriver(IOCTL_ARK_DRIVER, op, NULL, 0, (PVOID*)&drivers, &outlen);
	if (!ret) return false;
	for (int i = 0; i < drivers->count; i++) {
		infos.push_back(drivers->items[i]);
	}
	free(drivers);
	return true;
}
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
bool MemoryRead(ULONG64 addr, ULONG size, std::string &readbuf)
{
	if (!size) return false;
	MEMORY_IN memin;
	memin.addr = addr;
	memin.size = size;
	DWORD outlen;
	PMEMORY_OUT memout;
	bool ret = IoControlDriver(IOCTL_ARK_MEMORY, MEMORY_READ, &memin, sizeof(memin), (PVOID*)&memout, &outlen);
	if (!ret)	 return false;
	readbuf.resize(memout->size);
	memcpy(&readbuf[0], memout->readbuf, memout->size);
	free(memout);
	return true;
}
} // namespace IArkDrv
#endif