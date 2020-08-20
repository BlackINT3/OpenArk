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
#include <common/common.h>
#include <kdriver/kdriver.h>
#include <arkdrv-api/arkdrv-api.h>
#include "handle.h"

//https://www.cnblogs.com/Lamboy/p/3307012.html



#define KERNEL_HANDLE_MASK ((ULONG_PTR)((LONG)0x80000000))//
BOOLEAN ForceCloseHandle(HANDLE pid, HANDLE handle)
{
	BOOLEAN				ret = FALSE;
	PEPROCESS			eprocess;
	KAPC_STATE			apcstate;
	NTSTATUS			status;
	MODE				mode = UserMode;
	OBJECT_HANDLE_FLAG_INFORMATION objectinfo;

	PsLookupProcessByProcessId(pid, &eprocess);
	if (eprocess == NULL || !MmIsAddressValid(eprocess)) {
		return ret;
	}

	__try {
		KeStackAttachProcess(eprocess, &apcstate);
		//ObDereferenceObject(eprocess);
		if (PsGetCurrentProcess() == PsInitialSystemProcess) {
			handle = (HANDLE)((ULONG_PTR)handle | KERNEL_HANDLE_MASK);
			mode = KernelMode;
		}
		objectinfo.Inherit = 0;
		objectinfo.ProtectFromClose = 0;
		status = ObSetHandleAttributes(handle, &objectinfo, mode);
		status = ZwClose(handle);
		KeUnstackDetachProcess(&apcstate);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		KdPrint(("EXCEPTION_EXECUTE_HANDLER\n"));
	}
	ObDereferenceObject(eprocess);
	if (NT_SUCCESS(status)) {
		ret = TRUE;
	}
	return ret;
}


NTSTATUS EnumHandleInfoByPid(LPVOID buf, ULONG len, HANDLE pid)
{
	NTSTATUS	status = STATUS_UNSUCCESSFUL;
	ULONG		size = 0x10000;
	PVOID		buffer = NULL;
	UINT64		handlecount = 0;
	PSYSTEM_HANDLE_TABLE_ENTRY_INFO tableinfo = NULL;
	PHANDLE_INFO	info = (PHANDLE_INFO)buf;
	ULONG			count = 0;
	BOOLEAN			goon = TRUE;

	buffer = ExAllocatePoolWithTag(NonPagedPool, size, 'enhd');
	if (buffer == NULL) {
		return STATUS_UNSUCCESSFUL;
	}

	RtlZeroMemory(buffer, size);
	status = ZwQuerySystemInformation(SystemHandleInformation, buffer, size, 0);
	while (status == STATUS_INFO_LENGTH_MISMATCH){
		ExFreePoolWithTag(buffer, 'enhd');
		size = size * 2;
		buffer = ExAllocatePoolWithTag(NonPagedPool, size, 'enhd');
		if (buffer == NULL) {
			return STATUS_UNSUCCESSFUL;
		}
		RtlZeroMemory(buffer, size);
		status = ZwQuerySystemInformation(SystemHandleInformation, buffer, size, 0);
	}

	if (!NT_SUCCESS(status)) {
		return STATUS_UNSUCCESSFUL;
	}
	handlecount = (UINT64)(((SYSTEM_HANDLE_INFORMATION *)buffer)->NumberOfHandles);
	tableinfo = (SYSTEM_HANDLE_TABLE_ENTRY_INFO *)((SYSTEM_HANDLE_INFORMATION *)buffer)->Handles;

	for (int i = 0; i < handlecount; i++) {
		USHORT			processid = tableinfo[i].UniqueProcessId;
		HANDLE			handle = (HANDLE)tableinfo[i].HandleValue;
		ULONG			typeindex = (ULONG)tableinfo[i].ObjectTypeIndex;
		LPVOID			object = tableinfo[i].Object;

		if (pid == (HANDLE)processid) {
			CLIENT_ID					cid = {0};
			OBJECT_ATTRIBUTES			oa = {0};
			HANDLE						hprocess = NULL;
			HANDLE						hdupobj = NULL;
			OBJECT_BASIC_INFORMATION	basicinfo = {0};
			POBJECT_NAME_INFORMATION	nameinfo = NULL;
			POBJECT_TYPE_INFORMATION    typeinfo = NULL;
			ULONG						refcount = 0;
			ULONG						flag = 0;
			PHANDLE_ITEM				item = &(info->items[count]);

			cid.UniqueProcess = (HANDLE)processid;
			cid.UniqueThread = (HANDLE)0;
			InitializeObjectAttributes(&oa, NULL, 0, NULL, NULL);

			while (1) {
				status = ZwOpenProcess(&hprocess, PROCESS_DUP_HANDLE, &oa, &cid);
				if (!NT_SUCCESS(status)) {
					KdPrint(("ZwOpenProcess : Fail "));
					break;
				}
				status = ZwDuplicateObject(hprocess, handle, NtCurrentProcess(), &hdupobj, PROCESS_ALL_ACCESS, 0, DUPLICATE_SAME_ACCESS);
				if (!NT_SUCCESS(status)) {
					DbgPrint("ZwDuplicateObject : Fail ");
					break;
				}
				ZwQueryObject(hdupobj, ObjectBasicInformation, &basicinfo, sizeof(OBJECT_BASIC_INFORMATION), NULL);

				nameinfo = (POBJECT_NAME_INFORMATION)ExAllocatePoolWithTag(NonPagedPool, 1024, 'enhd');
				if (nameinfo == NULL) {
					break;
				}
				RtlZeroMemory(nameinfo, 1024);
				status = ZwQueryObject(hdupobj, (OBJECT_INFORMATION_CLASS)1, nameinfo, 1024, &flag); //ObjectNameInformation

				typeinfo = (POBJECT_TYPE_INFORMATION )ExAllocatePoolWithTag(NonPagedPool, 256, 'enhd');
				if (typeinfo == NULL) {
					break;
				}
				RtlZeroMemory(typeinfo, 256);
				status = ZwQueryObject(hdupobj, (OBJECT_INFORMATION_CLASS)2, typeinfo, 256, &flag); // ObjectTypeInformation

				refcount = basicinfo.ReferenceCount - basicinfo.HandleCount;

				if (((CHAR *)item + sizeof(HANDLE_ITEM)) > ((CHAR *)buf + len)) {
					status = STATUS_FLT_BUFFER_TOO_SMALL;
					info->count = count;
					goon = FALSE;
					break;
				}
				item->pid = pid;
				item->handle = handle;
				item->object = object;
				item->ref_count = refcount;
				item->type_index = typeindex;
				RtlCopyMemory(item->name, nameinfo->Name.Buffer, sizeof(WCHAR) * nameinfo->Name.Length);
				RtlCopyMemory(item->type_name, typeinfo->TypeName.Buffer, sizeof(WCHAR) * typeinfo->TypeName.Length);
				count += 1;
				KdPrint(("NAME:%wZ\t\t\tTYPE:%wZ\n", &(nameinfo->Name), &(typeinfo->TypeName)));
				break;
			}

			if (nameinfo) ExFreePoolWithTag(nameinfo, 'enhd');
			if (typeinfo) ExFreePoolWithTag(typeinfo, 'enhd');
			if(hdupobj) ZwClose(hdupobj);
			if(hprocess) ZwClose(hprocess);
			if (goon == FALSE ) {
				break;
			}
		}
	}

	info->count = count;

	if (buffer) {
		ExFreePoolWithTag(buffer, 'enhd');
	}

	return status;
}


NTSTATUS StorageUnlockEnum(PVOID inbuf, ULONG inlen, PVOID outbuf, ULONG outlen, PIRP irp)
{
	LPVOID buf = outbuf;
	ULONG len = outlen;
	auto path = (WCHAR*)inbuf;

	NTSTATUS	status = STATUS_UNSUCCESSFUL;
	ULONG		size = 0x10000;
	PVOID		buffer = NULL;
	UINT64		handlecount = 0;
	PSYSTEM_HANDLE_TABLE_ENTRY_INFO tableinfo = NULL;
	PHANDLE_INFO	info = (PHANDLE_INFO)buf;
	ULONG			count = 0;

	buffer = ExAllocatePoolWithTag(NonPagedPool, size, 'enhd');
	if (buffer == NULL) {
		return STATUS_MEMORY_NOT_ALLOCATED;
	}

	RtlZeroMemory(buffer, size);
	status = ZwQuerySystemInformation(SystemHandleInformation, buffer, size, 0);
	while (status == STATUS_INFO_LENGTH_MISMATCH) {
		ExFreePoolWithTag(buffer, 'enhd');
		size = size * 2;
		buffer = ExAllocatePoolWithTag(NonPagedPool, size, 'enhd');
		if (buffer == NULL) {
			return STATUS_MEMORY_NOT_ALLOCATED;
		}
		RtlZeroMemory(buffer, size);
		status = ZwQuerySystemInformation(SystemHandleInformation, buffer, size, 0);
	}

	if (!NT_SUCCESS(status)) return status;

	handlecount = (UINT64)(((SYSTEM_HANDLE_INFORMATION *)buffer)->NumberOfHandles);
	tableinfo = (SYSTEM_HANDLE_TABLE_ENTRY_INFO *)((SYSTEM_HANDLE_INFORMATION *)buffer)->Handles;

	// calculate size
	LONG exactcnt = 0;
	for (int i = 0; i < handlecount; i++) {
		ULONG typeindex = (ULONG)tableinfo[i].ObjectTypeIndex;
		if (typeindex == 28) exactcnt++;
	}
	ULONG exactsize = sizeof(HANDLE_INFO) + (exactcnt-1) * sizeof(HANDLE_ITEM);
	if (buf == NULL || len < exactsize) {
		irp->IoStatus.Information = exactsize + 10 * sizeof(HANDLE_ITEM);
		return STATUS_BUFFER_OVERFLOW;
	}

	//KdBreakPoint();

	for (int i = 0; i < handlecount; i++) {
		USHORT			processid = tableinfo[i].UniqueProcessId;
		HANDLE			handle = (HANDLE)tableinfo[i].HandleValue;
		ULONG			typeindex = (ULONG)tableinfo[i].ObjectTypeIndex;
		LPVOID			object = tableinfo[i].Object;

		if (typeindex != 28) continue;;

		CLIENT_ID					cid = { 0 };
		OBJECT_ATTRIBUTES			oa = { 0 };
		HANDLE						hprocess = NULL;
		HANDLE						hdupobj = NULL;
		OBJECT_BASIC_INFORMATION	basicinfo = { 0 };
		POBJECT_NAME_INFORMATION	nameinfo = NULL;
		POBJECT_TYPE_INFORMATION    typeinfo = NULL;
		ULONG						refcount = 0;
		ULONG						flag = 0;
		PHANDLE_ITEM				item = &(info->items[count]);

		cid.UniqueProcess = (HANDLE)processid;
		cid.UniqueThread = (HANDLE)0;
		InitializeObjectAttributes(&oa, NULL, 0, NULL, NULL);

		while (1) {
			status = ZwOpenProcess(&hprocess, PROCESS_DUP_HANDLE, &oa, &cid);
			if (!NT_SUCCESS(status)) {
				KdPrint(("ZwOpenProcess err:%d", status));
				break;
			}
			status = ZwDuplicateObject(hprocess, handle, NtCurrentProcess(), &hdupobj, PROCESS_ALL_ACCESS, 0, DUPLICATE_SAME_ACCESS);
			if (!NT_SUCCESS(status)) {
				KdPrint(("ZwDuplicateObject err:%d", status));
				break;
			}
			nameinfo = (POBJECT_NAME_INFORMATION)ExAllocatePoolWithTag(NonPagedPool, 1024, 'enhd');
			if (nameinfo == NULL) {
				status = STATUS_MEMORY_NOT_ALLOCATED;
				break;
			}
			RtlZeroMemory(nameinfo, 1024);
			status = ZwQueryObject(hdupobj, (OBJECT_INFORMATION_CLASS)1, nameinfo, 1024, &flag); //ObjectNameInformation
			if (nameinfo->Name.Length > 0 && wcsstr(_wcslwr(nameinfo->Name.Buffer), path)) {
				// filter the file path
				ZwQueryObject(hdupobj, ObjectBasicInformation, &basicinfo, sizeof(OBJECT_BASIC_INFORMATION), NULL);
				typeinfo = (POBJECT_TYPE_INFORMATION)ExAllocatePoolWithTag(NonPagedPool, 256, 'enhd');
				if (typeinfo == NULL) {
					status = STATUS_MEMORY_NOT_ALLOCATED;
					break;
				}
				RtlZeroMemory(typeinfo, 256);
				status = ZwQueryObject(hdupobj, (OBJECT_INFORMATION_CLASS)2, typeinfo, 256, &flag); // ObjectTypeInformation
				refcount = basicinfo.ReferenceCount - basicinfo.HandleCount; // maybe bug?
				item->pid = (HANDLE)processid;
				item->handle = handle;
				item->object = object;
				item->ref_count = refcount;
				item->type_index = typeindex;
				RtlCopyMemory(item->name, nameinfo->Name.Buffer, sizeof(WCHAR) * nameinfo->Name.Length);
				RtlCopyMemory(item->type_name, typeinfo->TypeName.Buffer, sizeof(WCHAR) * typeinfo->TypeName.Length);
				count++;
				KdPrint(("NAME:%wZ\t\t\tTYPE:%wZ\n", &(nameinfo->Name), &(typeinfo->TypeName)));
			}
			break;
		}

		if (nameinfo) ExFreePoolWithTag(nameinfo, 'enhd');
		if (typeinfo) ExFreePoolWithTag(typeinfo, 'enhd');
		if (hdupobj) ZwClose(hdupobj);
		if (hprocess) ZwClose(hprocess);
	}

	info->count = count; // set count of item
	if (buffer)  ExFreePoolWithTag(buffer, 'enhd');
	if (count > 0){
		irp->IoStatus.Information = sizeof(HANDLE_INFO) + (count - 1) * sizeof(HANDLE_ITEM);
	}else {
		irp->IoStatus.Information = sizeof(HANDLE_INFO);
	}
	return status;
}
