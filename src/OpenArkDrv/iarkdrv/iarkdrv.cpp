#include "iarkdrv.h"
#ifdef _ARKDRV_
#else
namespace IArkDrv {
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

bool IoControlDriver(DWORD ctlcode, PVOID inbuf, DWORD inlen, PVOID *outbuf, DWORD *outlen)
{
	DWORD retlen = 0;

	if (arkdrv == INVALID_HANDLE_VALUE)
		return false;

	bool ret = DeviceIoControl(
				arkdrv,
				ctlcode,
				inbuf,
				inlen,
				NULL,
				0,
				&retlen,
				NULL);
	if (ret) return true;

	if (GetLastError() != ERROR_MORE_DATA)
		return false;

	*outbuf = NULL;
	auto bufsize = retlen;
	auto buf = (PDRIVER_INFO)calloc(bufsize, 1);
	if (!buf) return false;
	if (!DeviceIoControl(
		arkdrv,
		ctlcode,
		inbuf,
		inlen,
		buf,
		bufsize,
		&retlen,
		NULL)) {
		free(buf);
		return false;
	}
	*outbuf = buf;
	*outlen = retlen;
	return true;
}

bool HeartBeatPulse()
{
	if (!ConnectDriver()) return false;

	DWORD dummy = 0;
	PVOID outbuf;
	DWORD outlen;
	bool ret = IoControlDriver(IOCTL_ARK_HEARTBEAT, &dummy, sizeof(dummy), &outbuf, &outlen);
	return ret;
}

bool DriverEnumInfo(std::vector<DRIVER_ITEM> &infos)
{
	DWORD op = DRIVER_ENUM_INFO;
	PDRIVER_INFO drivers;
	DWORD outlen;
	bool ret = IoControlDriver(IOCTL_ARK_DRIVER, &op, sizeof(op), (PVOID*)&drivers, &outlen);
	if (!ret) return false;
	for (int i = 0; i < drivers->count; i++) {
		infos.push_back(drivers->items[i]);
	}
	free(drivers);
	return true;
}
bool NotifyPatch(NOTIFY_TYPE type, ULONG64 routine);
bool NotifyPatchRegularly(NOTIFY_TYPE type, ULONG64 routine, int interval);
bool NotifyRemove(NOTIFY_TYPE type, ULONG64 routine);
bool NotifyRemoveRegularly(NOTIFY_TYPE type, ULONG64 routine, int interval);
bool NotifyEnumProcess(std::vector<ULONG64> &routines)
{
	DWORD op = NOTIFY_ENUM_PROCESS;
	PNOTIFY_INFO notify;
	DWORD outlen;
	bool ret = IoControlDriver(IOCTL_ARK_NOTIFY, &op, sizeof(op), (PVOID*)&notify, &outlen);
	if (!ret) return false;
	for (int i = 0; i < notify->count; i++) {
		routines.push_back(notify->items[i]);
	}
	free(notify);
	return true;
}
bool NotifyEnumThread(std::vector<ULONG64> &routines);
bool NotifyEnumImage(std::vector<ULONG64> &routines);
bool NotifyEnumRegistry(std::vector<ULONG64> &routines);
} // namespace IArkDrv
#endif