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
#include "../common/common.h"
#include "memory.h"

#ifndef MM_COPY_MEMORY_VIRTUAL
#define MM_COPY_MEMORY_VIRTUAL 0x2
#endif
#ifndef _MM_COPY_ADDRESS
typedef struct _MM_COPY_ADDRESS {
	union {
		PVOID VirtualAddress;
		PHYSICAL_ADDRESS PhysicalAddress;
	};
} MM_COPY_ADDRESS, *PMMCOPY_ADDRESS;
#endif
typedef NTSTATUS(NTAPI *__MmCopyMemory)(
	PVOID TargetAddress,
	MM_COPY_ADDRESS SourceAddress,
	SIZE_T NumberOfBytes,
	ULONG Flags,
	PSIZE_T NumberOfBytesTransferred
);

BOOLEAN MmReadKernelMemory(PVOID addr, PVOID buf, ULONG len)
{
	BOOLEAN ret = FALSE;
	if (addr <= MM_HIGHEST_USER_ADDRESS) return FALSE;
	if (ArkDrv.ver >= NTOS_WIN81) {
		PVOID data = ExAllocatePool(NonPagedPool, len);
		if (data) {
			auto pMmCopyMemory = (__MmCopyMemory)GetNtRoutineAddress(L"MmCopyMemory");
			if (pMmCopyMemory) {
				SIZE_T cplen;
				MM_COPY_ADDRESS cpaddr;
				cpaddr.VirtualAddress = addr;
				NTSTATUS status = pMmCopyMemory(data, cpaddr, len, MM_COPY_MEMORY_VIRTUAL, &cplen);
				if (NT_SUCCESS(status)) {
					RtlCopyMemory(buf, data, cplen);
					ret = true;
				}
			}
			ExFreePool(data);
		}
	} else {
		// [TDOO] BYTE_OFFSET PAGE_ALIGN
		PHYSICAL_ADDRESS pa;
		pa = MmGetPhysicalAddress(addr);
		if (pa.QuadPart) {
			PVOID va = MmMapIoSpace(pa, len, MmNonCached);
			if (va) {
				RtlCopyMemory(buf, va, len);
				MmUnmapIoSpace(va, len);
				ret = TRUE;
			}
		}
	}
	return ret;
}


BOOLEAN InitMemoryDispatcher()
{
	return TRUE;
}

NTSTATUS MemoryReadData(PMEMORY_IN inbuf, ULONG inlen, PVOID outbuf, ULONG outlen, PIRP irp)
{
	ULONG size = inbuf->size + 4;
	if (size > outlen) {
		irp->IoStatus.Information = size;
		return STATUS_BUFFER_OVERFLOW;
	}
	PVOID data = ExAllocatePool(NonPagedPool, size);
	if (!data) return STATUS_MEMORY_NOT_ALLOCATED;
	BOOLEAN ret = MmReadKernelMemory((PVOID)inbuf->addr, data, size);
	if (!ret) {
		ExFreePool(data);
		return STATUS_UNSUCCESSFUL;
	}
	PMEMORY_OUT memout = (PMEMORY_OUT)outbuf;
	memout->size = size;
	RtlCopyMemory(memout->readbuf, data, size);
	ExFreePool(data);
	irp->IoStatus.Information = size;
	return STATUS_SUCCESS;
}

NTSTATUS MemoryDispatcher(IN ULONG op, IN PDEVICE_OBJECT devobj, IN PIRP irp)
{
	//KdBreakPoint();

	NTSTATUS status;
	PIO_STACK_LOCATION irpstack;
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
	case MEMORY_READ:
		status = MemoryReadData((PMEMORY_IN)inbuf, inlen, outbuf, outlen, irp);
		break;
	default:
		break;
	}
	return status;
}