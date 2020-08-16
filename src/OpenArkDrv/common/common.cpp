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

BOOLEAN InitArkDriver(PDRIVER_OBJECT drvobj, PDEVICE_OBJECT devobj)
{
	ArkDrv.drvobj = drvobj;
	ArkDrv.devobj = devobj;
	
	ArkDrv.ver = KNONE::OsNtVersion();
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