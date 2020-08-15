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
#include "kstorage.h"
#include "unlock/handle.h"
#include "../common/common.h"
#include <knone.h>
#include <ntifs.h>
#include <ntimage.h>
#include <ntintsafe.h>
#include <windef.h>

NTSTATUS StorageUnlockClose(PVOID inbuf, ULONG inlen, PVOID outbuf, ULONG outlen, PIRP irp)
{
	return 0;
}

NTSTATUS StorageDispatcher(IN ULONG op, IN PDEVICE_OBJECT devobj, IN PIRP irp)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PIO_STACK_LOCATION	irpstack;
	PVOID	inbuf_dup = NULL;
	PVOID	inbuf = NULL;
	PVOID outbuf = NULL;
	ULONG inlen = 0;
	ULONG outlen = 0;
	irpstack = IoGetCurrentIrpStackLocation(irp);
	inlen = irpstack->Parameters.DeviceIoControl.InputBufferLength - 4;
	inbuf = (UCHAR*)irp->AssociatedIrp.SystemBuffer + 4;
	KdBreakPoint();
	status = DuplicateInputBuffer(irp, inbuf);
	if (!NT_SUCCESS(status)) return status;

	outbuf = irp->AssociatedIrp.SystemBuffer;
	outlen = irpstack->Parameters.DeviceIoControl.OutputBufferLength;
	switch (op) {
	case STORAGE_UNLOCK_ENUM:
		status = StorageUnlockEnum(inbuf, inlen, outbuf, outlen, irp);
		break;
	case STORAGE_UNLOCK_CLOSE:
		status = StorageUnlockClose(inbuf, inlen, outbuf, outlen, irp);
		break;
	default:
		break;
	}

	ReleaseInputBuffer(irp, inbuf_dup);
	return status;
}