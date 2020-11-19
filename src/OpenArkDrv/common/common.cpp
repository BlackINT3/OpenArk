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
#include "common.h"
#include "../kdriver/kdriver.h"
#include "../knotify/knotify.h"

ARK_DRIVER ArkDrv;

/*++
Description:
	get os version
Arguments:
	void
Return:
	NTOS_VERSION
--*/
NTOS_VERSION_X OsNtVersion()
{
	RTL_OSVERSIONINFOEXW info;
	if (!KNONE::OsGetVersionInfo(info)) return _NTOS_UNKNOWN;
	
	switch (info.dwMajorVersion) {
	case 5: {
		if (info.dwMinorVersion == 1) {
			if (info.wServicePackMajor == 1) return _NTOS_WINXPSP1;
			if (info.wServicePackMajor == 2) return _NTOS_WINXPSP2;
			if (info.wServicePackMajor == 3) return _NTOS_WINXPSP3;
			return _NTOS_WINXP;
		}
		if (info.dwMinorVersion == 2) {
			if (info.wServicePackMajor == 1) return _NTOS_WIN2003SP1;
			if (info.wServicePackMajor == 2) return _NTOS_WIN2003SP2;
			return _NTOS_WIN2003;
		}
		break;
	} case 6: {
		if (info.dwMinorVersion == 0) {
			if (info.wServicePackMajor == 1) return _NTOS_WINVISTASP1;
			if (info.wServicePackMajor == 2) return _NTOS_WINVISTASP2;
			return _NTOS_WINVISTA;
		}
		if (info.dwMinorVersion == 1) {
			if (info.wServicePackMajor == 1) return _NTOS_WIN7SP1;
			return _NTOS_WIN7;
		}
		if (info.dwMinorVersion == 2) {
			return _NTOS_WIN8;
		}
		if (info.dwMinorVersion == 3) {
			return _NTOS_WIN81;
		}
		break;
	} case 10: {
		if (info.dwBuildNumber == 10240) return _NTOS_WIN10_1507;
		if (info.dwBuildNumber == 10586) return _NTOS_WIN10_1511;
		if (info.dwBuildNumber == 14393) return _NTOS_WIN10_1607;
		if (info.dwBuildNumber == 15063) return _NTOS_WIN10_1703;
		if (info.dwBuildNumber == 16299) return _NTOS_WIN10_1709;
		if (info.dwBuildNumber == 17134) return _NTOS_WIN10_1803;
		if (info.dwBuildNumber == 17763) return _NTOS_WIN10_1809;
		if (info.dwBuildNumber == 18362) return _NTOS_WIN10_1903;
		if (info.dwBuildNumber == 18363) return _NTOS_WIN10_1909;
		if (info.dwBuildNumber == 19041) return _NTOS_WIN10_2004;
		if (info.dwBuildNumber == 19042) return _NTOS_WIN10_20H2;
	}
	default:
		break;
	}
	return _NTOS_UNKNOWN;
}


BOOLEAN InitArkDriver(PDRIVER_OBJECT drvobj, PDEVICE_OBJECT devobj)
{
	ArkDrv.drvobj = drvobj;
	ArkDrv.devobj = devobj;
	
	ArkDrv.ver = OsNtVersion();
	ArkDrv.major = KNONE::OsMajorVersion();
	ArkDrv.minor = KNONE::OsMinorVersion();
	ArkDrv.build = KNONE::OsBuildNumber();

	InitDriverDispatcher();
	InitNotifyDispatcher();
	
	return TRUE;
}

PVOID GetNtRoutineAddress(IN PCWSTR name)
{
	UNICODE_STRING ustr;
	RtlInitUnicodeString(&ustr, name);
	return MmGetSystemRoutineAddress(&ustr);
}

NTSTATUS DuplicateInputBuffer(IN PIRP irp, PVOID &inbuf)
{
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION	irpstack; 
	PVOID inbuf_dup = NULL;
	PVOID outbuf = NULL;
	ULONG inlen = 0;
	irpstack = IoGetCurrentIrpStackLocation(irp);
	inlen = irpstack->Parameters.DeviceIoControl.InputBufferLength - 4;
	if (inbuf && inlen) {
		inbuf_dup = ExAllocatePoolWithTag(NonPagedPool, inlen, ARK_POOLTAG);
		if (!inbuf_dup) return STATUS_MEMORY_NOT_ALLOCATED;
		RtlCopyMemory(inbuf_dup, inbuf, inlen);
		inbuf = inbuf_dup;
	}
	return status;
}

NTSTATUS ReleaseInputBuffer(IN PIRP irp, PVOID &inbuf)
{
	NTSTATUS status = STATUS_SUCCESS;
	if (inbuf) {
		inbuf = NULL;
		ExFreePoolWithTag(inbuf, ARK_POOLTAG);
	}
	return status;
}
