#include "notify-lib.h"
#include "../common/common.h"
#include "../iarkdrv/iarkdrv.h"

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

PVOID GetNtRoutineAddress(IN PCWSTR FunctionName)
{
	UNICODE_STRING UsFunctionName;
	RtlInitUnicodeString(&UsFunctionName, FunctionName);
	return MmGetSystemRoutineAddress(&UsFunctionName);
}

ULONG GetProcessNotifyMaximum()
{
#ifdef _AMD64_
	return 64;
#else
	if (ArkDrv.major >= 6) return 64;
	else return 8;
#endif
}

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
					if (*ptr2 == 0x4c && *(ptr2 + 1) == 0x8d && *(ptr2 + 2) == 0x35) {
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
				if (!MmIsAddressValid((PVOID)psp_routine)) break;
				// c7 8d 35 lea r14
				for (PUCHAR ptr2 = psp_routine; ptr2 <= psp_routine + 0x50; ptr2++) {
					if (*ptr2 == 0xc7 && *(ptr2 + 1) == 0x8d && (*(ptr2 + 2) == 0x25 || *(ptr2 + 2) == 0x35)) {
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
			if (!psp_routine || !MmIsAddressValid((PVOID)psp_routine)) break;
			// 4c 8d 3d lea r15
			for (PUCHAR ptr2 = psp_routine; ptr2 <= psp_routine + 0x50; ptr2++) {
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
			if (*ptr1 == 0xe8 || *ptr1 == 0xe9) {
				psp_routine = *(LONG*)(ptr1 + 1) + ptr1 + 5;
				if (!MmIsAddressValid((PVOID)psp_routine)) break;
			}
			//Win10 1903 4c 8d 2d   lea  r13
			//Win10 1809 4c 8d 2d   lea  r13
			//Win10 1803 4c 8d 2d   lea  r13   48 8d 0d   lea  rcx
			//Win10 1709 4c 8d 2d   lea  r13
			//Win10 1703 4c 8d 25   lea  r12
			//Win10 1607 4c 8d 25   lea  r12
			//Win10 1511 4c 8d 3d   lea  r15
			//Win10 1507 4c 8d 3d   lea  r15
			for (PUCHAR ptr2 = psp_routine; ptr2 <= psp_routine + 0x100; ptr2++) {
				if (*ptr2 == 0x4c && *(ptr2 + 1) == 0x8d &&
					(*(ptr2 + 2) == 0x2d || *(ptr2 + 2) == 0x25 || *(ptr2 + 2) == 0x3d)) {
					callback = (PEX_CALLBACK)(ptr2 + (*(LONG*)(ptr2 + 3)) + 7);
					if (!MmIsAddressValid(callback))  callback = NULL;
					break;
				}
			}
		}
	}
#else
	if (ArkDrv.major == 5) {
		for (PUCHAR ptr1 = routine; ptr1 <= routine + 0x20; ptr1++) {
			if (*ptr1 == 0xbf) {
				callback = (PEX_CALLBACK)*(LONG*)(ptr1 + 1);
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
	PEX_CALLBACK callback = GetProcessNotifyCallback();
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

