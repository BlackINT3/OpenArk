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
#include "notify-lib.h"
#include "../common/common.h"
#include "../arkdrv-api/arkdrv-api.h"

#if defined (_WIN64)
#define MAX_FAST_REFS 15
#else
#define MAX_FAST_REFS 7
#endif

typedef struct _EX_FAST_REF
{
	union
	{
		PVOID Object;
#if defined (_WIN64)
		ULONG_PTR RefCnt : 4;
#else
		ULONG_PTR RefCnt : 3;
#endif
		ULONG_PTR Value;
	};
} EX_FAST_REF, *PEX_FAST_REF;

typedef NTSTATUS (NTAPI *__PEX_CALLBACK_FUNCTION)(IN PVOID CallbackContext, IN PVOID Argument1, IN PVOID Argument2);
typedef struct _EX_CALLBACK_ROUTINE_BLOCK
{
	EX_RUNDOWN_REF        RundownProtect;
	__PEX_CALLBACK_FUNCTION Function;
	PVOID                 Context;
} EX_CALLBACK_ROUTINE_BLOCK, *PEX_CALLBACK_ROUTINE_BLOCK;
typedef struct _EX_CALLBACK
{
	EX_FAST_REF RoutineBlock;
} EX_CALLBACK, *PEX_CALLBACK;

typedef struct _CM_CALLBACK_CONTEXT_BLOCKEX
{
	LIST_ENTRY		ListEntry;
	ULONG           Unknown1;
	ULONG			Unknown2;
	LARGE_INTEGER	Cookie;
	PVOID           CallerContext;
	PVOID			Function;
	UNICODE_STRING	Altitude;
	LIST_ENTRY		ObjectContextListHead;
} CM_CALLBACK_CONTEXT_BLOCKEX, *PCM_CALLBACK_CONTEXT_BLOCKEX;

BOOLEAN ExFastRefCanBeReferenced(PEX_FAST_REF ref)
{
	return ref->RefCnt != 0;
}

BOOLEAN ExFastRefObjectNull(PEX_FAST_REF ref)
{
	return (BOOLEAN)(ref->Value == 0);
}

PVOID ExFastRefGetObject(PEX_FAST_REF ref)
{
	return (PVOID)(ref->Value & ~MAX_FAST_REFS);
}

PEX_CALLBACK_ROUTINE_BLOCK ExReferenceCallBackBlock(PEX_FAST_REF ref)
{
	if (ExFastRefObjectNull(ref)) {
		return NULL;
	}

	if (!ExFastRefCanBeReferenced(ref)) {
		return NULL;
	}

	return (PEX_CALLBACK_ROUTINE_BLOCK)ExFastRefGetObject(ref);
}

FORCEINLINE ULONG GetProcessNotifyMaximum()
{
#ifdef _AMD64_
	return 64;
#else
	if (ArkDrv.major >= 6) return 64;
	else return 8;
#endif
}
FORCEINLINE ULONG GetThreadNotifyMaximum()
{
#ifdef _AMD64_
	return 64;
#else
	if (ArkDrv.major >= 6) return 64;
	else return 8;
#endif
}
FORCEINLINE ULONG GetImageNotifyMaximum()
{
	if (ArkDrv.ver >= NTOS_WIN7SP1) return 64;
	else return 8;
}
FORCEINLINE ULONG GetRegistryNotifyMaximum()
{
	return 100;
}

// Process Notify
PEX_CALLBACK GetProcessNotifyCallback()
{
	PUCHAR routine = (PUCHAR)GetNtRoutineAddress(L"PsSetCreateProcessNotifyRoutine");
	if (!routine) return NULL;

	PEX_CALLBACK callback = NULL;
#ifdef _AMD64_
	if (ArkDrv.ver >= NTOS_WINVISTA && ArkDrv.ver < NTOS_WIN7) {
		for (PUCHAR ptr1 = routine; ptr1 <= routine + 0x10; ptr1++) {
			// e9 jmp
			if (*ptr1 == 0xe9) {
				PUCHAR psp_routine = *(LONG*)(ptr1 + 1) + ptr1 + 5;
				if (!MmIsAddressValid((PVOID)psp_routine)) break;
				//Win Vista			4c 8d 25   lea r12
				//Win Vista SP1 4c 8d 35   lea r14
				//Win Vista SP2 4c 8d 35   lea r14
				for (PUCHAR ptr2 = psp_routine; ptr2 <= psp_routine + 0x50; ptr2++) {
					if (*ptr2 == 0x4c && *(ptr2 + 1) == 0x8d && (*(ptr2 + 2) == 0x25 || *(ptr2 + 2) == 0x35)) {
						callback = (PEX_CALLBACK)(ptr2 + (*(LONG*)(ptr2 + 3)) + 7);
						if (!MmIsAddressValid(callback))  callback = NULL;
						break;
					}
				}
			}
		}
	} else if (ArkDrv.ver >= NTOS_WIN7 && ArkDrv.ver < NTOS_WIN8) {
		for (PUCHAR ptr1 = routine; ptr1 <= routine + 0x10; ptr1++) {
			// e9 jmp
			if (*ptr1 == 0xe9) {
				PUCHAR psp_routine = *(LONG*)(ptr1 + 1) + ptr1 + 5;
				if (!MmIsAddressValid(psp_routine)) break;
				// lea r14
				for (PUCHAR ptr2 = psp_routine; ptr2 <= psp_routine + 0x50; ptr2++) {
					if (*ptr2 == 0x4c && *(ptr2 + 1) == 0x8d && *(ptr2 + 2) == 0x35) {
						callback = (PEX_CALLBACK)(ptr2 + (*(LONG*)(ptr2 + 3)) + 7);
						if (!MmIsAddressValid(callback))  callback = NULL;
						break;
					}
				}
			}
		}
	} else if (ArkDrv.ver >= NTOS_WIN8 && ArkDrv.ver < NTOS_WIN10_1507) {
		for (PUCHAR ptr1 = routine; ptr1 <= routine + 0x10; ptr1++) {
			PUCHAR psp_routine = NULL;
			//Win8 eb jmp 
			if (*ptr1 == 0xeb) {
				psp_routine = *(UCHAR*)(ptr1 + 1) + ptr1 + 2;	
			}	else if (*ptr1 == 0xe9) { //Win8.1 e9 jmp
				psp_routine = *(LONG*)(ptr1 + 1) + ptr1 + 5;
			}
			if (!psp_routine || !MmIsAddressValid(psp_routine)) break;
			// 4c 8d 3d lea r15
			for (PUCHAR ptr2 = psp_routine; ptr2 <= psp_routine + 0x60; ptr2++) {
				if (*ptr2 == 0x4c && *(ptr2 + 1) == 0x8d && *(ptr2 + 2) == 0x3d) {
					callback = (PEX_CALLBACK)(ptr2 + (*(LONG*)(ptr2 + 3)) + 7);
					if (!MmIsAddressValid(callback))  callback = NULL;
					break;
				}
			}
		}
	} else if (ArkDrv.ver >= NTOS_WIN10_1507) {
		//Win10 1903 0xe8   call
		//Win10 1809 0xe8   call
		//Win10 1803 0xe8   call
		//Win10 1709 0xe8   call
		//Win10 1703 0xe9   jmp
		//Win10 1607 0xe9   jmp
		//Win10 1511 0xe9   jmp
		//Win10 1507 0xe9   jmp
		for (PUCHAR ptr1 = routine; ptr1 <= routine + 0x10; ptr1++) {
			PUCHAR psp_routine = NULL;
			if (ptr1[0] == 0xe8 || ptr1[0] == 0xe9) {
				psp_routine = *(LONG*)(ptr1 + 1) + ptr1 + 5;
				if (!MmIsAddressValid((PVOID)psp_routine)) break;
				//Win10 1903 4c 8d 2d   lea  r13
				//Win10 1809 4c 8d 2d   lea  r13
				//Win10 1803 4c 8d 2d   lea  r13   48 8d 0d   lea  rcx
				//Win10 1709 4c 8d 2d   lea  r13
				//Win10 1703 4c 8d 25   lea  r12
				//Win10 1607 4c 8d 25   lea  r12
				//Win10 1511 4c 8d 3d   lea  r15
				//Win10 1507 4c 8d 3d   lea  r15
				for (PUCHAR ptr2 = psp_routine; ptr2 <= psp_routine + 0x100; ptr2++) {
					if (ptr2[0] == 0x4c && ptr2[1] == 0x8d &&
						(ptr2[2] == 0x2d || ptr2[2] == 0x25 || ptr2[2]  == 0x3d)) {
						callback = (PEX_CALLBACK)(ptr2 + (*(LONG*)(ptr2 + 3)) + 7);
						if (!MmIsAddressValid(callback))  callback = NULL;
						break;
					}
				}
			}
		}
	}
#else
	if (ArkDrv.major == 5) {
		for (PUCHAR ptr1 = routine; ptr1 <= routine + 0x20; ptr1++) {
			if (*ptr1 == 0xbf) {
				callback = (PEX_CALLBACK)*(LONG*)(ptr1 + 1);
				if (!MmIsAddressValid(callback))  callback = NULL;
				break;
			}
		}
	} else if (ArkDrv.major == 6) {
		for (PUCHAR ptr1 = routine; ptr1 <= routine + 0x20; ptr1++) {
			if (*ptr1 == 0xe8) {
				PUCHAR psp_routine = *(LONG*)(ptr1 + 1) + ptr1 + 5;
				if (!MmIsAddressValid((PVOID)psp_routine)) break;
				for (PUCHAR ptr2 = psp_routine; ptr2 <= psp_routine + 0x30; ptr2++) {
					if (*ptr2 == 0xc7) {
						callback = (PEX_CALLBACK)*(LONG*)(ptr2 + 3);
						if (!MmIsAddressValid(callback))  callback = NULL;
						break;
					}
				}
			}
		}
	}
#endif

	return callback;
}
BOOLEAN GetProcessNotifyInfo(ULONG &count, PULONG64 &items)
{
	if (!ArkDrv.process_notify) {
		ArkDrv.process_notify = GetProcessNotifyCallback();
	}
	PEX_CALLBACK callback = (PEX_CALLBACK)ArkDrv.process_notify;
	if (!callback) return FALSE;
	ULONG maxinum = GetProcessNotifyMaximum();
	if (!maxinum) return FALSE;

	auto bufsize = maxinum * sizeof(ULONG64);
	auto buf = (PULONG64)ExAllocatePool(NonPagedPool, bufsize);
	if (!buf) return FALSE;

	count = 0;
	for (ULONG i = 0; i < maxinum; i++) {
		if (!MmIsAddressValid(callback)) break;
		auto block = (PEX_CALLBACK_ROUTINE_BLOCK)ExReferenceCallBackBlock(&callback->RoutineBlock);
		if (block != NULL) {
			buf[count] = (ULONG64)block->Function;
			count++;
		}
		callback++;
	}
	items = buf;
	
	if (count <= 0) {
		ExFreePool(buf);
		return FALSE;
	}
	return TRUE;
}
BOOLEAN RemoveProcessNotify(ULONG64 routine)
{
	NTSTATUS status;
	if (!MmIsAddressValid((PVOID)routine)) return false;
	status = PsSetCreateProcessNotifyRoutine((PCREATE_PROCESS_NOTIFY_ROUTINE)routine, TRUE);
	return NT_SUCCESS(status);
}

// Thread Notify
PEX_CALLBACK GetThreadNotifyCallback()
{
	PUCHAR routine = (PUCHAR)GetNtRoutineAddress(L"PsSetCreateThreadNotifyRoutine");
	if (!routine) return NULL;

	PEX_CALLBACK callback = NULL;
#ifdef _AMD64_
	if (ArkDrv.ver >= NTOS_WINVISTA && ArkDrv.ver <= NTOS_WIN81) {
		// lea rcx
		for (PUCHAR ptr1 = routine; ptr1 <= routine + 0x30; ptr1++) {
			if (*ptr1 == 0x48 && *(ptr1 + 1) == 0x8d && *(ptr1 + 2) == 0x0d) {
				callback = (PEX_CALLBACK)(ptr1 + (*(LONG*)(ptr1 + 3)) + 7);
				if (!MmIsAddressValid(callback))  callback = NULL;
				break;
			}
		}
	} else if (ArkDrv.ver >= NTOS_WIN10_1507) {
		//Win10 1903 e8   call
		//Win10 1809 e8   call
		//Win10 1803 e8   call
		//Win10 1709 e8   call
		//Win10 1703 e9   jmp
		//Win10 1607 e9   jmp
		//Win10 1511 e9   jmp
		//Win10 1507 e9   jmp
		for (PUCHAR ptr1 = routine; ptr1 <= routine + 0x10; ptr1++) {
			PUCHAR psp_routine = NULL;
			if (*ptr1 == 0xe8 || *ptr1 == 0xe9) {
				psp_routine = *(LONG*)(ptr1 + 1) + ptr1 + 5;
				if (!MmIsAddressValid((PVOID)psp_routine)) break;
				//Win10 1903 48 8d 0d   lea  rcx
				//Win10 1809 48 8d 0d   lea  rcx
				//Win10 1803 48 8d 0d   lea  rcx
				//Win10 1709 48 8d 0d   lea  rcx
				//Win10 1703 48 8d 0d   lea  rcx
				//Win10 1607 48 8d 0d   lea  rcx
				//Win10 1511 48 8d 0d   lea  rcx
				//Win10 1507 48 8d 0d   lea  rcx
				for (PUCHAR ptr2 = psp_routine; ptr2 <= psp_routine + 0x40; ptr2++) {
					if (*ptr2 == 0x48 && *(ptr2 + 1) == 0x8d && *(ptr2 + 2) == 0x0d) {
						callback = (PEX_CALLBACK)(ptr2 + (*(LONG*)(ptr2 + 3)) + 7);
						if (!MmIsAddressValid(callback))  callback = NULL;
						break;
					}
				}
			}
		}
	}
#else
	if (ArkDrv.major <= 6) {
		for (PUCHAR ptr1 = routine; ptr1 <= routine + 0x30; ptr1++) {
			if (*ptr1 == 0x56 && *(ptr1 + 1) == 0xbe) {
				callback = (PEX_CALLBACK)*(LONG*)(ptr1 + 2);
				if (!MmIsAddressValid(callback))  callback = NULL;
				break;
			}
		}
	}
#endif
	return callback;
}
BOOLEAN GetThreadNotifyInfo(ULONG &count, PULONG64 &items)
{
	if (!ArkDrv.thread_notify) {
		ArkDrv.thread_notify = GetThreadNotifyCallback();
	}
	PEX_CALLBACK callback = (PEX_CALLBACK)ArkDrv.thread_notify;
	if (!callback) return FALSE;
	ULONG maxinum = GetThreadNotifyMaximum();
	if (!maxinum) return FALSE;

	auto bufsize = maxinum * sizeof(ULONG64);
	auto buf = (PULONG64)ExAllocatePool(NonPagedPool, bufsize);
	if (!buf) return FALSE;

	count = 0;
	for (ULONG i = 0; i < maxinum; i++) {
		if (!MmIsAddressValid(callback)) break;
		auto block = (PEX_CALLBACK_ROUTINE_BLOCK)ExReferenceCallBackBlock(&callback->RoutineBlock);
		if (block != NULL) {
			buf[count] = (ULONG64)block->Function;
			count++;
		}
		callback++;
	}
	items = buf;
	if (count <= 0) {
		ExFreePool(buf);
		return FALSE;
	}
	return TRUE;
}
BOOLEAN RemoveThreadNotify(ULONG64 routine)
{
	NTSTATUS status;
	if (!MmIsAddressValid((PVOID)routine)) return false;
	status = PsRemoveCreateThreadNotifyRoutine((PCREATE_THREAD_NOTIFY_ROUTINE)routine);
	return NT_SUCCESS(status);
}

// Image Notify
PEX_CALLBACK GetImageNotifyCallback()
{
	PUCHAR routine = NULL;
	if (ArkDrv.ver >= NTOS_WINXP && ArkDrv.ver <= NTOS_WIN10_1703) {
		routine = (PUCHAR)GetNtRoutineAddress(L"PsSetLoadImageNotifyRoutine");
	} else if (ArkDrv.ver >= NTOS_WIN10_1709 && ArkDrv.ver <= NTOS_WIN10_1903) {
		routine = (PUCHAR)GetNtRoutineAddress(L"PsSetLoadImageNotifyRoutineEx");
	}
	if (!routine) return NULL;

	PEX_CALLBACK callback = NULL;
#ifdef _AMD64_
	// lea rcx
	for (PUCHAR ptr1 = routine; ptr1 <= routine + 0x60; ptr1++) {
		if (*ptr1 == 0x48 && *(ptr1 + 1) == 0x8d && *(ptr1 + 2) == 0x0d) {
			callback = (PEX_CALLBACK)(ptr1 + (*(LONG*)(ptr1 + 3)) + 7);
			if (!MmIsAddressValid(callback))  callback = NULL;
			break;
	}
}
#else
	if (ArkDrv.major == 5) {
		for (PUCHAR ptr1 = routine; ptr1 <= routine + 0x30; ptr1++) {
			if (*ptr1 == 0x56 && *(ptr1 + 1) == 0xbe) {
				callback = (PEX_CALLBACK)*(LONG*)(ptr1 + 2);
				if (!MmIsAddressValid(callback)) callback = NULL;
				break;
			}
		}
	} else if (ArkDrv.major == 6) {
		for (PUCHAR ptr1 = routine; ptr1 <= routine + 0x30; ptr1++) {
			if ((*ptr1 == 0x25 || *ptr1 == 0x28) && *(ptr1 + 1) == 0xbe) {
				callback = (PEX_CALLBACK)*(LONG*)(ptr1 + 2);
				if (!MmIsAddressValid(callback)) callback = NULL;
				break;
			}
		}
	}
#endif
	return callback;
}
BOOLEAN GetImageNotifyInfo(ULONG &count, PULONG64 &items)
{
	if (!ArkDrv.image_notify) {
		ArkDrv.image_notify = GetImageNotifyCallback();
	}
	PEX_CALLBACK callback = (PEX_CALLBACK)ArkDrv.image_notify;
	if (!callback) return FALSE;
	ULONG maxinum = GetImageNotifyMaximum();
	if (!maxinum) return FALSE;

	auto bufsize = maxinum * sizeof(ULONG64);
	auto buf = (PULONG64)ExAllocatePool(NonPagedPool, bufsize);
	if (!buf) return FALSE;

	count = 0;
	for (ULONG i = 0; i < maxinum; i++) {
		if (!MmIsAddressValid(callback)) break;
		auto block = (PEX_CALLBACK_ROUTINE_BLOCK)ExReferenceCallBackBlock(&callback->RoutineBlock);
		if (block != NULL) {
			buf[count] = (ULONG64)block->Function;
			count++;
		}
		callback++;
	}
	items = buf;
	if (count <= 0) {
		ExFreePool(buf);
		return FALSE;
	}
	return TRUE;
}
BOOLEAN RemoveImageNotify(ULONG64 routine)
{
	NTSTATUS status;
	if (!MmIsAddressValid((PVOID)routine)) return false;
	status = PsRemoveLoadImageNotifyRoutine((PLOAD_IMAGE_NOTIFY_ROUTINE)routine);
	return NT_SUCCESS(status);
}

// Registry Notify
PVOID GetRegistryNotifyCallback()
{
	PUCHAR routine = (PUCHAR)GetNtRoutineAddress(L"CmUnRegisterCallback");
	if (!routine) return NULL;

	PVOID callback = NULL;

#ifdef _AMD64_
	if (ArkDrv.ver >= NTOS_WINVISTA) {
		// xor r8d, r8d
		// lea rcx, CallbackListHead
		for (PUCHAR ptr1 = routine; ptr1 <= routine + 0x100; ptr1++) {
			if (*ptr1 == 0x45 && *(ptr1 + 1) == 0x33 && *(ptr1 + 2) == 0xc0 &&
				*(ptr1 + 8) == 0x48 && *(ptr1 + 9) == 0x8d && *(ptr1 + 10) == 0x0d) {
				callback = (PLIST_ENTRY)((ptr1 + 8) + (*(LONG*)((ptr1 + 8) + 3)) + 7);
				if (!MmIsAddressValid(callback)) callback = NULL;
				break;
			}
		}
	}
#else
	if (ArkDrv.major == 5) {
		for (PUCHAR ptr1 = routine; ptr1 <= routine + 0x20; ptr1++) {
			if (*ptr1 == 0x57 && *(ptr1 + 1) == 0xbb) {
				callback = (PEX_CALLBACK)*(LONG*)(ptr1 + 2);
				if (!MmIsAddressValid(callback)) callback = NULL;
				break;
			}
		}
	} else if (ArkDrv.major == 6) {
		for (PUCHAR ptr1 = routine; ptr1 <= routine + 0x100; ptr1++) {
			if (*ptr1 == 0x4d && *(ptr1 + 1) == 0xd4 && *(ptr1 + 1) == 0xbf) {
				callback = (PEX_CALLBACK)*(LONG*)(ptr1 + 3);
				if (!MmIsAddressValid(callback)) callback = NULL;
				break;
			}
		}
	}
#endif
	return callback;
}
BOOLEAN GetRegistryNotifyInfo(ULONG &count, PULONG64 &items)
{
	if (!ArkDrv.registry_notify) {
		ArkDrv.registry_notify = GetRegistryNotifyCallback();
	}
	PEX_CALLBACK callback = (PEX_CALLBACK)ArkDrv.registry_notify;
	if (!callback) return FALSE;
	ULONG maxinum = GetRegistryNotifyMaximum();
	if (!maxinum) return FALSE;

	auto bufsize = maxinum * sizeof(ULONG64);
	auto buf = (PULONG64)ExAllocatePool(NonPagedPool, bufsize);
	if (!buf) return FALSE;

	count = 0;
	if (ArkDrv.major >= 6) {
		PLIST_ENTRY head = (PLIST_ENTRY)callback;
		PCM_CALLBACK_CONTEXT_BLOCKEX ctx;
		for (PLIST_ENTRY entry = head->Flink; entry != head;) {
			if (count >= maxinum)	break;
			ctx = CONTAINING_RECORD(entry, CM_CALLBACK_CONTEXT_BLOCKEX, ListEntry);
			entry = entry->Flink;
			if (ctx->Function == NULL) continue;
			if (MmIsAddressValid(ctx->Function)) {
				buf[count] = (ULONG64)ctx->Function;
				count++;
			}
		}
	} else {
		for (ULONG i = 0; i < maxinum; i++) {
			if (!MmIsAddressValid(callback)) break;
			auto block = (PEX_CALLBACK_ROUTINE_BLOCK)ExReferenceCallBackBlock(&callback->RoutineBlock);
			if (block != NULL) {
				buf[count] = (ULONG64)block->Function;
				count++;
			}
			callback++;
		}
	}
	items = buf;
	if (count <= 0) {
		ExFreePool(buf);
		return FALSE;
	}
	return TRUE;
}
BOOLEAN RemoveRegistryNotify(ULONG64 routine)
{
	// [TODO] NT5 cookie
	if (ArkDrv.major <= 5) return false;

	if (!ArkDrv.registry_notify) {
		ArkDrv.registry_notify = GetRegistryNotifyCallback();
	}
	PEX_CALLBACK callback = (PEX_CALLBACK)ArkDrv.registry_notify;
	if (!callback) return FALSE;
	ULONG maxinum = GetRegistryNotifyMaximum();
	if (!maxinum) return FALSE;

	PLIST_ENTRY head = (PLIST_ENTRY)callback;
	PCM_CALLBACK_CONTEXT_BLOCKEX ctx;
	for (PLIST_ENTRY entry = head->Flink; entry != head;) {
		ctx = CONTAINING_RECORD(entry, CM_CALLBACK_CONTEXT_BLOCKEX, ListEntry);
		entry = entry->Flink;
		if (routine == (ULONG64)ctx->Function) {
			CmUnRegisterCallback(ctx->Cookie);
			return TRUE;
		}
	}
	return FALSE;
}