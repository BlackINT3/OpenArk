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

bool DisconnectDriver()
{
	if (arkdrv == INVALID_HANDLE_VALUE) return true;
	CloseHandle(arkdrv);
	arkdrv = INVALID_HANDLE_VALUE;
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
	auto buf = (PVOID)calloc(bufsize, 1);
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

bool IoControlDriver(DWORD ctlcode, DWORD op, const std::wstring &indata, std::string &outdata)
{
	if (!ConnectDriver()) return false;
	DWORD outlen;
	CHAR *info;
	WCHAR *tempdata = NULL;
	DWORD tempsize = 0;
	if (indata.size()) {
		tempdata = (WCHAR*)indata.c_str();
		tempsize = (indata.size() + 1) * 2;
	}
	bool ret = IoControlDriver(ctlcode, op, (PVOID)tempdata, tempsize, (PVOID*)&info, &outlen);
	if (!ret) return false;
	outdata.assign(info, outlen);
	free(info);
	return true;
}
bool IoControlDriver(DWORD ctlcode, DWORD op, const std::string &indata, std::string &outdata)
{
	if (!ConnectDriver()) return false;
	DWORD outlen;
	CHAR *info;
	CHAR *tempdata = NULL;
	DWORD tempsize = 0;
	if (indata.size()) {
		tempdata = (CHAR*)indata.c_str();
		tempsize = indata.size();
	}
	bool ret = IoControlDriver(ctlcode, op, (PVOID)tempdata, tempsize, (PVOID*)&info, &outlen);
	if (!ret) return false;
	outdata.assign(info, outlen);
	free(info);
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

} // namespace IArkDrv
#endif