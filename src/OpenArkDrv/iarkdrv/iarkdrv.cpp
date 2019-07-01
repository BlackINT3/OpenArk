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

bool HeartBeatPulse()
{
	DWORD retlen = 0;
	DWORD dummy = 0;
	if (arkdrv == INVALID_HANDLE_VALUE)
		return false;
	if (!DeviceIoControl(
		arkdrv,
		IOCTL_ARK_HEARTBEAT,
		&dummy,
		sizeof(dummy),
		&dummy,
		sizeof(dummy),
		&retlen,
		NULL)) {
		return false;
	}
	return true;
}
bool DriverEnumInfo(std::vector<DRIVER_ITEM> &info)
{
	DWORD retlen = 0;
	DWORD op = DRIVER_ENUM_INFO;
	if (arkdrv == INVALID_HANDLE_VALUE)
		return false;
	
	if (!DeviceIoControl(
		arkdrv,
		IOCTL_ARK_DRIVER,
		&op,
		sizeof(op),
		NULL,
		0,
		&retlen,
		NULL)) {
		if (GetLastError() != ERROR_MORE_DATA) {
			return false;
		}
	}
	auto bufsize = retlen;
	auto buf = (PDRIVER_INFO)calloc(bufsize, 1);
	if (!buf) return false;
	if (!DeviceIoControl(
		arkdrv,
		IOCTL_ARK_DRIVER,
		&op,
		sizeof(op),
		buf,
		bufsize,
		&retlen,
		NULL)) {
		free(buf);
		return false;
	}
	for (int i = 0; i < buf->count; i++) {
		info.push_back(buf->items[i]);
	}
	free(buf);
	return true;
}
bool NotifyPatch(NOTIFY_TYPE type, ULONG64 routine);
bool NotifyPatchRegularly(NOTIFY_TYPE type, ULONG64 routine, int interval);
bool NotifyRemove(NOTIFY_TYPE type, ULONG64 routine);
bool NotifyRemoveRegularly(NOTIFY_TYPE type, ULONG64 routine, int interval);
bool NotifyEnumProcess(std::vector<ULONG64> &routines);
bool NotifyEnumThread(std::vector<ULONG64> &routines);
bool NotifyEnumImage(std::vector<ULONG64> &routines);
bool NotifyEnumRegistry(std::vector<ULONG64> &routines);
} // namespace IArkDrv
#endif