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
#include "../common/common.h"
#include "notify.h"
#include "notify-lib.h"

NTSTATUS GetNotifyInfo(NOTIFY_TYPE type, PVOID inbuf, ULONG inlen, PVOID outbuf, ULONG outlen, PIRP irp)
{
	ULONG count = 0;
	PULONG64 items = NULL;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	
	BOOLEAN ret = FALSE;
	switch (type) {
	case CREATE_PROCESS:
		ret= GetProcessNotifyInfo(count, items);
		break;
	case CREATE_THREAD:
		ret = GetThreadNotifyInfo(count, items);
		break;
	case LOAD_IMAGE:
		ret = GetImageNotifyInfo(count, items);
		break;
	case CM_REGISTRY:
		ret = GetRegistryNotifyInfo(count, items);
		break;
	default:
		break;
	}
	if (!ret) return STATUS_UNSUCCESSFUL;

	ULONG size = sizeof(NOTIFY_INFO) + (count-1) * sizeof(ULONG64);
	if (size > outlen) {
		irp->IoStatus.Information = size + 10 * sizeof(ULONG64);
		ExFreePool(items);
		return STATUS_BUFFER_OVERFLOW;
	}
	auto info = (PNOTIFY_INFO)outbuf;
	info->count = count;
	info->type = type;
 	for (ULONG i = 0; i < count; i++) {
		info->items[i] = items[i];
	}
	ExFreePool(items);
	irp->IoStatus.Information = size;
	return STATUS_SUCCESS;
}

NTSTATUS RemoveNotifyInfo(PVOID inbuf, ULONG inlen, PVOID outbuf, ULONG outlen, PIRP irp)
{
	auto removed = (PNOTIFY_REMOVE_INFO)inbuf;
	auto type = removed->type;
	auto routine = removed->item;
	BOOLEAN ret = FALSE;
	switch (type) {
	case CREATE_PROCESS:
		ret = RemoveProcessNotify(routine);
		break;
	case CREATE_THREAD:
		ret = RemoveThreadNotify(routine);
		break;
	case LOAD_IMAGE:
		ret = RemoveImageNotify(routine);
		break;
	case CM_REGISTRY:
		ret = RemoveRegistryNotify(routine);
		break;
	default:
		break;
	}
	if (!ret) return STATUS_UNSUCCESSFUL;
	irp->IoStatus.Information = 0;
	return STATUS_SUCCESS;
}

BOOLEAN InitNotifyDispatcher()
{
	ArkDrv.process_notify = NULL;
	ArkDrv.thread_notify = NULL;
	ArkDrv.image_notify = NULL;
	ArkDrv.registry_notify = NULL;
	return TRUE;
}

NTSTATUS NotifyDispatcher(IN ULONG op, IN PDEVICE_OBJECT devobj, IN PIRP irp)
{
	//KdBreakPoint();
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PIO_STACK_LOCATION	irpstack;
	PVOID	inbuf = NULL;
	PVOID outbuf = NULL;
	ULONG inlen = 0;
	ULONG outlen = 0;
	irpstack = IoGetCurrentIrpStackLocation(irp);
	inbuf = (UCHAR*)irp->AssociatedIrp.SystemBuffer + 4;
	inlen = irpstack->Parameters.DeviceIoControl.InputBufferLength - 4;
	outbuf = irp->AssociatedIrp.SystemBuffer;
	outlen = irpstack->Parameters.DeviceIoControl.OutputBufferLength;
	switch (op) {
	case NOTIFY_PATCH:
		break;
	case NOTIFY_PATCH_REGULARLY:
		break;
	case NOTIFY_REMOVE:
		status = RemoveNotifyInfo(inbuf, inlen, outbuf, outlen, irp);
		break;
	case NOTIFY_REMOVE_REGULARLY:
		break;
	case NOTIFY_ENUM_PROCESS:
		status = GetNotifyInfo(CREATE_PROCESS, inbuf, inlen, outbuf, outlen, irp);
		break;
	case NOTIFY_ENUM_THREAD:
		status = GetNotifyInfo(CREATE_THREAD, inbuf, inlen, outbuf, outlen, irp);
		break;
	case NOTIFY_ENUM_IMAGE:
		status = GetNotifyInfo(LOAD_IMAGE, inbuf, inlen, outbuf, outlen, irp);
		break;
	case NOTIFY_ENUM_REGISTRY:
		status = GetNotifyInfo(CM_REGISTRY, inbuf, inlen, outbuf, outlen, irp);
		break;
	default:
		break;
	}
	return status;
}