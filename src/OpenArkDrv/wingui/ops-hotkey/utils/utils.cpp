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
#include "utils.h"
#include <ntifs.h>
#include <ntimage.h>
#include <ntintsafe.h>
#include <windef.h>

// The code of PE parser is part of UNONE
// https://github.com/BlackINT3/none/blob/master/src/unone/pe/unone-pe.cpp
#ifndef IN_RANGE
#define IN_RANGE(pos,begin,size) (((ULONGLONG)pos>=(ULONGLONG)begin) && ((ULONGLONG)pos<=((ULONGLONG)begin+size)))
#endif
#define PE_DOS_HEADER(base) ((PIMAGE_DOS_HEADER)base)
#define PE_NT_HEADER(base) ((PIMAGE_NT_HEADERS)((CHAR*)base + PE_DOS_HEADER(base)->e_lfanew))
#define PE_OPT_HEADER(base) ((PIMAGE_OPTIONAL_HEADER)(&PE_NT_HEADER(base)->OptionalHeader))
#define PE_OPT_HEADER32(base) ((PIMAGE_OPTIONAL_HEADER32)(&PE_NT_HEADER(base)->OptionalHeader))
#define PE_OPT_HEADER64(base) ((PIMAGE_OPTIONAL_HEADER64)(&PE_NT_HEADER(base)->OptionalHeader))
#define PE_X64(base) (PE_OPT_HEADER(base)->Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
NTSTATUS GetSectionRegion(PUCHAR base, CHAR* name, PUCHAR& start, ULONG& size)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PIMAGE_SECTION_HEADER section;
	PIMAGE_NT_HEADERS nt_hdr = PE_NT_HEADER(base);
	DWORD section_cnt = nt_hdr->FileHeader.NumberOfSections;
	if (!MmIsAddressValid(nt_hdr))
		return status;
	section = IMAGE_FIRST_SECTION(nt_hdr);
	for (DWORD i = 0; i < section_cnt; i++) {
		CHAR sn[IMAGE_SIZEOF_SHORT_NAME + 1];
		RtlCopyMemory(sn, section->Name, IMAGE_SIZEOF_SHORT_NAME);
		if (!_stricmp(sn, name)) {
			start = base + section->VirtualAddress;
			size = section->Misc.VirtualSize;
			status = STATUS_SUCCESS;
			break;
		}
		section++;
	}
	return status;
}
PIMAGE_DATA_DIRECTORY PeGetDataDirectory(__in DWORD idx, __in CHAR* base)
{
	DWORD rva = 0;
	DWORD size = 0;
	if (PE_X64(base)) {
		return &PE_OPT_HEADER64(base)->DataDirectory[idx];
	}
	else {
		return &PE_OPT_HEADER32(base)->DataDirectory[idx];
	}
}
bool PeValid(__in CHAR* base)
{
	__try {
		PIMAGE_DOS_HEADER dos_hdr = (PIMAGE_DOS_HEADER)base;
		PIMAGE_NT_HEADERS nt_hdr = NULL;
		if (dos_hdr == NULL)
			return false;
		if (dos_hdr->e_magic != IMAGE_DOS_SIGNATURE)
			return false;
		nt_hdr = (PIMAGE_NT_HEADERS)(base + dos_hdr->e_lfanew);
		if (nt_hdr->Signature != IMAGE_NT_SIGNATURE)
			return false;
		return true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
}
DWORD PeGetImageSize(__in CHAR* base)
{
	if (!PeValid(base)) return 0;
	if (PE_X64(base))
		return PE_OPT_HEADER64(base)->SizeOfImage;
	else
		return PE_OPT_HEADER32(base)->SizeOfImage;
}
bool PeRegionValid(CHAR* base, PVOID va, DWORD size = 0)
{
	DWORD image_size = PeGetImageSize(base);
	return (IN_RANGE((CHAR*)va, base, image_size)) && (IN_RANGE((CHAR*)va + size, base, image_size));
}
DWORD PeRvaToRaw(__in CHAR* base, __in DWORD rva)
{
	PIMAGE_NT_HEADERS nt_hdr = PE_NT_HEADER(base);
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(nt_hdr);
	for (DWORD i = 0; i < nt_hdr->FileHeader.NumberOfSections; i++) {
		DWORD va_base = section->VirtualAddress;
		DWORD raw_base = section->PointerToRawData;
		DWORD raw_size = section->SizeOfRawData;
		if (IN_RANGE(rva, va_base, raw_size))
			return raw_base + (rva - va_base);
		section++;
	}
	return 0;
}

typedef struct _RTL_PROCESS_MODULE_INFORMATION {
	HANDLE Section;         // Not filled in
	PVOID MappedBase;
	PVOID ImageBase;
	ULONG ImageSize;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT OffsetToFileName;
	UCHAR  FullPathName[MAXIMUM_FILENAME_LENGTH];
} RTL_PROCESS_MODULE_INFORMATION, * PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES {
	ULONG NumberOfModules;
	RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, * PRTL_PROCESS_MODULES;

typedef enum _SYSTEM_INFORMATION_CLASS {
	SystemModuleInformation = 0xb,
} SYSTEM_INFORMATION_CLASS;

extern "C"
NTSTATUS
NTAPI
ZwQuerySystemInformation(
	IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
	OUT PVOID SystemInformation,
	IN ULONG SystemInformationLength,
	OUT PULONG ReturnLength OPTIONAL
);

PVOID GetSystemModuleBase(IN char* modname, OUT PULONG imagesize)
{
	NTSTATUS status = STATUS_SUCCESS;
	PRTL_PROCESS_MODULES mods = NULL;
	PVOID imagebase = NULL;
	ULONG retlen = 0;
	status = ZwQuerySystemInformation(SystemModuleInformation, 0, 0, &retlen);
	if (retlen == 0) {
		return NULL;
	}

	mods = (PRTL_PROCESS_MODULES)ExAllocatePool(NonPagedPool, retlen);
	RtlZeroMemory(mods, retlen);

	status = ZwQuerySystemInformation(SystemModuleInformation, mods, retlen, &retlen);
	if (NT_SUCCESS(status)) {
		PRTL_PROCESS_MODULE_INFORMATION modinfo = mods->Modules;
		if (modname) {
			modname = _strlwr(modname);
			for (ULONG i = 0; i < mods->NumberOfModules; i++) {
				if (strstr(_strlwr((char*)modinfo[i].FullPathName), modname)) {
					imagebase = modinfo[i].ImageBase;
					*imagesize = modinfo[i].ImageSize;
					break;
				}
			}
		} else {
			imagebase = modinfo[0].ImageBase;
			*imagesize = modinfo[0].ImageSize;
		}

	}
	if (mods) ExFreePool(mods);

	return imagebase;
}

BOOLEAN OsGetVersionInfo(IN OUT RTL_OSVERSIONINFOEXW& info)
{
	NTSTATUS status;
	RtlZeroMemory(&info, sizeof(info));
	info.dwOSVersionInfoSize = sizeof(info);
	status = RtlGetVersion((RTL_OSVERSIONINFOW*)&info);
	return NT_SUCCESS(status);
}
