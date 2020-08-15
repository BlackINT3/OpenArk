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
#include "kprocess.h"
#include "../common/common.h"
#include <knone.h>
#include <ntifs.h>
#include <ntimage.h>
#include <ntintsafe.h>
#include <windef.h>

EXTERN_C NTSTATUS NTAPI ZwOpenThread(
	__out PHANDLE ThreadHandle,
	__in ACCESS_MASK DesiredAccess,
	__in POBJECT_ATTRIBUTES ObjectAttributes,
	__in_opt PCLIENT_ID ClientId
);


NTSTATUS ProcessOpenByInfo(PVOID inbuf, ULONG inlen, PVOID outbuf, ULONG outlen, IN PIRP irp)
{
	if (outlen < 4) {
		irp->IoStatus.Information = 4;
		return STATUS_BUFFER_OVERFLOW;
	}
	if (inlen < sizeof(PROCESS_OPEN_INFO)) return STATUS_UNSUCCESSFUL;

	PPROCESS_OPEN_INFO info = (PPROCESS_OPEN_INFO)inbuf;

	CLIENT_ID cid;
	cid.UniqueProcess = (HANDLE)info->pid;
	cid.UniqueThread = (HANDLE)0;

	DWORD attr = 0;
	HANDLE handle;
	OBJECT_ATTRIBUTES oa;
	if (info->inherit) attr |= OBJ_INHERIT;
	InitializeObjectAttributes(&oa, NULL, attr, NULL, NULL);
	NTSTATUS status = ZwOpenProcess(&handle, info->access, &oa, &cid);
	if (!NT_SUCCESS(status)) {
		return status;
	}
	RtlCopyMemory(outbuf, &handle, 4);
	irp->IoStatus.Information = 4;
	return status;
}

NTSTATUS ThreadOpenByInfo(PVOID inbuf, ULONG inlen, PVOID outbuf, ULONG outlen, IN PIRP irp)
{
	if (outlen < 4) {
		irp->IoStatus.Information = 4;
		return STATUS_BUFFER_OVERFLOW;
	}
	if (inlen < sizeof(THREAD_OPEN_INFO)) return STATUS_UNSUCCESSFUL;

	PTHREAD_OPEN_INFO info = (PTHREAD_OPEN_INFO)inbuf;

	CLIENT_ID cid;
	cid.UniqueProcess = (HANDLE)0;
	cid.UniqueThread = (HANDLE)info->tid;

	DWORD attr = 0;
	HANDLE handle;
	OBJECT_ATTRIBUTES oa;
	if (info->inherit) attr |= OBJ_INHERIT;
	InitializeObjectAttributes(&oa, NULL, attr, NULL, NULL);
	NTSTATUS status = ZwOpenThread(&handle, info->access, &oa, &cid);
	if (!NT_SUCCESS(status)) {
		return status;
	}
	RtlCopyMemory(outbuf, &handle, 4);
	irp->IoStatus.Information = 4;
	return status;
}


NTSTATUS ProcessDispatcher(IN ULONG op, IN PDEVICE_OBJECT devobj, PVOID inbuf, ULONG inlen, PVOID outbuf, ULONG outlen, IN PIRP irp)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	switch (op) {
	case PROCESS_OPEN:
		status = ProcessOpenByInfo(inbuf, inlen, outbuf, outlen, irp);
		break;
	case THREAD_OPEN:
		status = ThreadOpenByInfo(inbuf, inlen, outbuf, outlen, irp);
		break;
	default:
		break;
	}

	return status;
}