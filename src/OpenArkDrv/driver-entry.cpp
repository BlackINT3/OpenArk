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
#include <ntifs.h>
#include "arkdrv-api/arkdrv-api.h"
#include "common/common.h"
#include "driver/driver.h"
#include "notify/notify.h"
#include "memory/memory.h"

EXTERN_C NTSTATUS DriverEntry(PDRIVER_OBJECT drvobj, PUNICODE_STRING registry);
NTSTATUS MainDispatcher(PDEVICE_OBJECT devobj, PIRP irp);
NTSTATUS DefaultDispatcher(PDEVICE_OBJECT devobj, PIRP irp);
VOID DriverUnload(PDRIVER_OBJECT drvobj);

NTSTATUS DriverEntry(PDRIVER_OBJECT drvobj, PUNICODE_STRING registry)
{
	NTSTATUS status;
	UNICODE_STRING devname, symlnk;
	PDEVICE_OBJECT devobj;

	UNREFERENCED_PARAMETER(registry);
	KdPrint(("OpenArkDrv loading..."));

	RtlInitUnicodeString(&devname, ARK_NTDEVICE_NAME);
	RtlInitUnicodeString(&symlnk, ARK_DOSDEVICE_NAME);

	status = IoCreateDevice(drvobj,
													0,
													&devname,
													FILE_DEVICE_UNKNOWN,
													FILE_DEVICE_SECURE_OPEN,
													FALSE,
													&devobj);
	if (!NT_SUCCESS(status)) {
		KdPrint(("IoCreateDevice err:%x", status));
		return status;
	}

	drvobj->DriverUnload = DriverUnload;
	drvobj->MajorFunction[IRP_MJ_CREATE] = DefaultDispatcher;
	drvobj->MajorFunction[IRP_MJ_CLOSE] = DefaultDispatcher;
	drvobj->MajorFunction[IRP_MJ_DEVICE_CONTROL] = MainDispatcher;

	status = IoCreateSymbolicLink(&symlnk, &devname);
	if (!NT_SUCCESS(status)) {
		IoDeleteDevice(devobj);
		KdPrint(("IoCreateSymbolicLink err:%x", status));
		return status;
	}

	if (!InitArkDriver(drvobj, devobj)) {
		KdPrint(("InitArkDriver err"));
		IoDeleteSymbolicLink(&symlnk);
		IoDeleteDevice(devobj);
		return STATUS_UNSUCCESSFUL;
	}

	status = STATUS_SUCCESS;
	return status;
}

NTSTATUS MainDispatcher(PDEVICE_OBJECT devobj, PIRP irp)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PIO_STACK_LOCATION	irpstack;
	PVOID inbuf = NULL;
	PVOID outbuf = NULL;
	ULONG inlen = 0;
	ULONG outlen = 0;
	ULONG ctlcode = 0;
	ULONG op = 0;

	irpstack = IoGetCurrentIrpStackLocation(irp);
	ctlcode = irpstack->Parameters.DeviceIoControl.IoControlCode;

	// [TODO] try except ProbeForRead/Write
	inlen = irpstack->Parameters.DeviceIoControl.InputBufferLength;
	if (inlen < 4) return STATUS_INVALID_PARAMETER;
	inbuf = irp->AssociatedIrp.SystemBuffer;
	if (!inbuf) return STATUS_INVALID_PARAMETER;
	op = *(ULONG*)inbuf;

	switch (ctlcode) {
	case IOCTL_ARK_HEARTBEAT:
		status = STATUS_SUCCESS;
		break;
	case IOCTL_ARK_DRIVER:
		status = DriverDispatcher(op, devobj, irp);
		break;
	case IOCTL_ARK_NOTIFY:
		status = NotifyDispatcher(op, devobj, irp);
		break;
	case IOCTL_ARK_MEMORY:
		status = MemoryDispatcher(op, devobj, irp);
		break;
	default:
		status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}
	irp->IoStatus.Status = status;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return status;
}

NTSTATUS DefaultDispatcher(PDEVICE_OBJECT devobj, PIRP irp)
{
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

VOID DriverUnload(PDRIVER_OBJECT drvobj)
{
	UNICODE_STRING symlnk;
	PDEVICE_OBJECT dev = drvobj->DeviceObject;
	if (dev) {
		RtlInitUnicodeString(&symlnk, ARK_DOSDEVICE_NAME);
		IoDeleteSymbolicLink(&symlnk);
		IoDeleteDevice(dev);
	}
}