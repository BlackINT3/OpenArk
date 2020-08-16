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
#include "ops-hotkey.h"
#include "utils/utils.h"
#include <knone.h>
#include <ntifs.h>
#include <ntimage.h>
#include <ntintsafe.h>
#include <windef.h>

typedef PVOID PWND;
typedef UINT64 PADDING64;
typedef UINT32 PADDING32;
typedef struct _THREADINFO {
	PETHREAD thread;
} *PTHREADINFO;

typedef struct _WNDINFO {
	HWND wnd;
} *PWNDINFO;

typedef struct _HOT_KEY {
	PTHREADINFO thdinfo;
	PVOID callback;
	PWNDINFO wndinfo;
	UINT16 modifiers1;		//eg:MOD_CONTROL(0x0002)
	UINT16 modifiers2;		//eg:MOD_NOREPEAT(0x4000)
	UINT32 vk;
	UINT32 id;
#ifdef _AMD64_
	PADDING32 pad;
#endif
	struct _HOT_KEY *slist;
} HOT_KEY, * PHOT_KEY;

extern "C"
UCHAR* NTAPI
PsGetProcessImageFileName(
	__in PEPROCESS Process
);

#define MAX_VK	0xFF
#define VK_OEM_CLEAR 0xFE
#define MOD_ALT 0x0001
#define MOD_NOREPEAT 0x4000

__inline BOOLEAN CheckHotkeyValid(PUCHAR addr, UINT32 vk)
{
	if (MmIsAddressValid(addr)) {
		PHOT_KEY hk = (PHOT_KEY)addr;
		if ((hk->vk & 0x7f) == vk) {
			// Hotkey valid
			return TRUE;
		}
	}
	return FALSE;
}

BOOLEAN SearchHotkeyTable(PUCHAR* &htable)
{
	htable = NULL;
	
	// catch the imagebase of win32k£¨Win7 win32k.sys£¬Win10 win32kfull.sys£©
	PUCHAR win32k;
	ULONG win32ksize = 0;
	RTL_OSVERSIONINFOEXW info;
	OsGetVersionInfo(info); if (info.dwMajorVersion == 10) {
		win32k = (PUCHAR)GetSystemModuleBase("win32kfull.sys", &win32ksize);
	} else {
		win32k = (PUCHAR)GetSystemModuleBase("win32k.sys", &win32ksize);
	}
	if (!win32k) {
		return FALSE;
	}
	KdPrint(("win32k:%p, win32ksize:%x\n", win32k, win32ksize));

	// catch the region of data segment (global HashTable)
	NTSTATUS status;
	PUCHAR start;
	ULONG size;
	status = GetSectionRegion(win32k, ".data", start, size);
	if (!NT_SUCCESS(status)) {
		return FALSE;
	}
	KdPrint(("win32k-data start:%p, size:%x\n", start, size));

	// now searching...
	PUCHAR *ptr = (PUCHAR*)start;
	for (int i = 0, j = 0; i < size/sizeof(ptr); i++) {
		if (j == 0x80) {
			KdPrint(("start:%p\n", &ptr[i]));
			// the first place
			i -= j;

			// validate the Hotkeys
			INT vks[] = { 5, 10 ,15, 20, 25, 30, 35, 40, 45};
			for (INT vk : vks) {
				if (!CheckHotkeyValid(ptr[i + vk], vk)) {
					j = 0;
					break;
				}
			}
			// we catch it
			if (j != 0) {
				htable = &ptr[i];
				break;
			}
			continue;
		}
		// kernel address filter
		if (ptr[i] > MmSystemRangeStart) {
			j++;
			continue;
		}
		j = 0;
	}

	return 1;
}

VOID DumpHotkeyNode(PHOT_KEY hk, PHOTKEY_ITEM items, ULONG &pos)
{
	if (MmIsAddressValid(hk->slist)) {
		DumpHotkeyNode(hk->slist, items, pos);
	}

	if (hk->id >= HOTKEY_PLACEHOLDER_ID && hk->id <= HOTKEY_PLACEHOLDER_ID + HOTKEY_MAX_VK) return;

	UCHAR *name = (UCHAR *)"";
	PETHREAD thread = hk->thdinfo->thread;
	PEPROCESS process = NULL;
	HANDLE pid = NULL;
	HANDLE tid = NULL;
	if (thread != NULL) {
		process = IoThreadToProcess(thread);
		pid = PsGetProcessId(process);
		tid = PsGetThreadId(thread);
		UCHAR *temp = PsGetProcessImageFileName(process);
		if (temp) name = temp;
	}

	HWND wnd = NULL;
	if (hk->wndinfo && MmIsAddressValid(hk->wndinfo))
		wnd = hk->wndinfo->wnd;
	KdPrint(("HK:%x NAME:%s PROCESS:%d THREAD:%d HWND:%x MOD:%d VK:%d \n",
		hk, name, pid, tid, wnd, hk->modifiers1, hk->vk));

	items[pos].hkobj = (ULONG64)hk;
	items[pos].id = hk->id;
	items[pos].tid = (UINT32)tid;
	items[pos].pid = (UINT32)pid;
	items[pos].mod1 = hk->modifiers1;
	items[pos].mod2 = hk->modifiers2;
	items[pos].vk = hk->vk;
	items[pos].wnd = (UINT32)wnd;
	strncpy((char*)items[pos].name, (char*)name, 63);

	pos++;
}

VOID DumpHotkeyTable(PUCHAR* table, PHOTKEY_ITEM items, ULONG &pos)
{
	for (INT i = 0; i < 0x7f; i++)
	{
		PHOT_KEY hk = (PHOT_KEY)table[i];
		if (hk) {
			DumpHotkeyNode(hk, items, pos);
		}
	}
}

static PUCHAR *HotkeyTable = NULL;
NTSTATUS GetHotkeyInfo(PVOID inbuf, ULONG inlen, PVOID outbuf, ULONG outlen, PIRP irp)
{
	ULONG count = 0;
	PHOTKEY_ITEM items = NULL;
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	KdBreakPoint();

	//
	// Caller have to guarantee current Win32k Session
	//
	/*
	PEPROCESS csrss_proc = NULL;
	ULONG csrss_pid = GetSessionProcessId();
	status = PsLookupProcessByProcessId((HANDLE)csrss_pid, &csrss_proc);
	if (NT_SUCCESS(status)) {
		KAPC_STATE apc_state;
		KeStackAttachProcess(csrss_proc, &apc_state);
		status = STATUS_UNSUCCESSFUL;
		//
		// Code here....
		//
		KeUnstackDetachProcess(&apc_state);
		ObDereferenceObject(csrss_proc);
	}
	*/

		if (!HotkeyTable) SearchHotkeyTable(HotkeyTable);

		KdPrint(("HotkeyTable:%llx\n", HotkeyTable));

		if (HotkeyTable != NULL) {
			ULONG presize = sizeof(HOTKEY_ITEM) * 1024;
			items = (PHOTKEY_ITEM)ExAllocatePool(NonPagedPool, presize);
			RtlZeroMemory(items, presize);
			if (items) {
				DumpHotkeyTable(HotkeyTable, items, count);
				if (count) {
					ULONG size = sizeof(HOTKEY_INFO) + (count - 1) * sizeof(HOTKEY_ITEM);
					if (size > outlen) {
						irp->IoStatus.Information = size;
						ExFreePool(items);
						return STATUS_BUFFER_OVERFLOW;
					}
					auto info = (PHOTKEY_INFO)outbuf;
					info->count = count;
					for (ULONG i = 0; i < count; i++) {
						info->items[i] = items[i];
					}
					irp->IoStatus.Information = size;
					status = STATUS_SUCCESS;
				}
				ExFreePool(items);
			}
		}

	return status;
}

VOID RemoveHotkeyNode(PHOT_KEY *hk_addr, PHOT_KEY hk, PHOTKEY_ITEM item, BOOLEAN &result)
{
	if (result) return;
	PHOT_KEY slist = NULL;
	if (MmIsAddressValid(hk->slist)) {
		slist = hk->slist;
		RemoveHotkeyNode(&hk->slist, hk->slist, item, result);
	}
	PETHREAD thread = hk->thdinfo->thread; 
	PEPROCESS process = NULL;
	HANDLE pid = NULL;
	HANDLE tid = NULL;
	if (thread != NULL) {
		process = IoThreadToProcess(thread);
		pid = PsGetProcessId(process);
		tid = PsGetThreadId(thread);
	}
	if (item->id == hk->id && item->pid == (UINT32)pid && item->tid == (UINT32)tid) {
		InterlockedExchangePointer((PVOID*)hk_addr, slist);
		result = TRUE;
		return;
	}
}

NTSTATUS RemoveHotkeyInfo(PVOID inbuf, ULONG inlen, PVOID outbuf, ULONG outlen, PIRP irp)
{
	NTSTATUS status = STATUS_INVALID_PARAMETER;
	PHOTKEY_ITEM item = (PHOTKEY_ITEM)inbuf;
	if (inlen != sizeof(HOTKEY_ITEM)) return status;
	if (!HotkeyTable) return status;

	BOOLEAN removed = FALSE;
	for (INT i = 0; i < 0x7f; i++) {
		PHOT_KEY hk = (PHOT_KEY)HotkeyTable[i];
		if (hk) {
			RemoveHotkeyNode((PHOT_KEY*)&HotkeyTable[i], hk, item, removed);
		}
	}
	if (!removed) return STATUS_NOT_FOUND;

	irp->IoStatus.Information = 0;
	return STATUS_SUCCESS;
}

NTSTATUS HotkeyDispatcher(IN ULONG op, IN PDEVICE_OBJECT devobj, IN PIRP irp)
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
	case HOTKEY_ENUM:
		status = GetHotkeyInfo(inbuf, inlen, outbuf, outlen, irp);
		break;
	case HOTKEY_REMOVE:
		status = RemoveHotkeyInfo(inbuf, inlen, outbuf, outlen, irp);
		break;
	default:
		break;
	}
	return status;
}