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
		break;
	case LOAD_IMAGE:
		break;
	case CM_REGISTRY:
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

BOOLEAN InitNotifyDispatcher()
{
	ArkDrv.ps_notify = NULL;
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
		break;
	case NOTIFY_REMOVE_REGULARLY:
		break;
	case NOTIFY_ENUM_PROCESS:
		status = GetNotifyInfo(CREATE_PROCESS, inbuf, inlen, outbuf, outlen, irp);
		break;
	case NOTIFY_ENUM_THREAD:
		break;
	case NOTIFY_ENUM_IMAGE:
		break;
	case NOTIFY_ENUM_REGISTRY:
		break;
	default:
		break;
	}
	return status;
}