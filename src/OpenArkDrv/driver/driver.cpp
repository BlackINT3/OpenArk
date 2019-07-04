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
#include "../arkdrv-api/arkdrv-api.h"
#include "driver.h"

PRTL_PROCESS_MODULES QueryModuleInformation()
{
	NTSTATUS status = STATUS_SUCCESS;
	PRTL_PROCESS_MODULES system_information = NULL;
	ULONG retlen = 0;

	status = ZwQuerySystemInformation(SystemModuleInformation, NULL, 0, &retlen);
	if (status != STATUS_INFO_LENGTH_MISMATCH) {
		return NULL;
	}
	ULONG bufsize = retlen + 10 * sizeof(RTL_PROCESS_MODULE_INFORMATION);
	PRTL_PROCESS_MODULES modules = (PRTL_PROCESS_MODULES)ExAllocatePool(NonPagedPool, bufsize);
	if (modules == NULL) {
		return NULL;
	}
	status = ZwQuerySystemInformation(SystemModuleInformation, modules, bufsize, &retlen);
	if (!NT_SUCCESS(status)) {
		ExFreePool(modules);
		return NULL;
	}

	return modules;
}

NTSTATUS DriverEnumInfo(PVOID	inbuf, ULONG inlen, PVOID outbuf, ULONG outlen, PIRP irp)
{
	PRTL_PROCESS_MODULES mods = QueryModuleInformation();
	if (!mods) {
		return STATUS_UNSUCCESSFUL;
	}
	ULONG count = mods->NumberOfModules;
	ULONG size = count * sizeof(DRIVER_ITEM) + 4;
	if (size > outlen) {
		irp->IoStatus.Information = size + 10 * sizeof(DRIVER_ITEM);
		ExFreePool(mods);
		return STATUS_BUFFER_OVERFLOW;
	}
	PDRIVER_INFO info = (PDRIVER_INFO)outbuf;
	info->count = count;
	for (ULONG i = 0; i < count; i++) {
		auto &item = info->items[i];
		auto &mod = mods->Modules[i];
#ifdef _AMD64_
		item.base = (ULONG64)mod.ImageBase;
#else
		item.base = (ULONG64)(ULONG)mod.ImageBase;
#endif
		item.size = mod.ImageSize;
		item.flags = mod.Flags;
		item.init_seq = mod.InitOrderIndex;
		item.load_seq = mod.LoadOrderIndex;
		RtlCopyMemory(item.path, mod.FullPathName, sizeof(mod.FullPathName));
	}
	ExFreePool(mods);
	irp->IoStatus.Information = size;
	return STATUS_SUCCESS;
}

BOOLEAN InitDriverDispatcher()
{
	return TRUE;
}

NTSTATUS DriverDispatcher(IN ULONG op, IN PDEVICE_OBJECT devobj, IN PIRP irp)
{
	//KdBreakPoint();

	NTSTATUS status;
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
	case DRIVER_ENUM_INFO:
		status = DriverEnumInfo(inbuf, inlen, outbuf, outlen, irp);
		break;
	default:
		break;
	}
	return status;
}