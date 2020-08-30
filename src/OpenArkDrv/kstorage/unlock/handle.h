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
#pragma once
#include <ntifs.h>

typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO {
	USHORT	UniqueProcessId;
	USHORT	CreatorBackTraceIndex;
	UCHAR	ObjectTypeIndex;
	UCHAR	HandleAttributes;
	USHORT	HandleValue;
	PVOID	Object;
	ULONG	GrantedAccess;
} SYSTEM_HANDLE_TABLE_ENTRY_INFO, *PSYSTEM_HANDLE_TABLE_ENTRY_INFO;

typedef struct _SYSTEM_HANDLE_INFORMATION {
	HANDLE NumberOfHandles;//32ÊÇulong 64ÊÇLONGLONG
	SYSTEM_HANDLE_TABLE_ENTRY_INFO Handles[1];
} SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION;

/*
typedef enum _OBJECT_INFORMATION_CLASS {
	ObjectBasicInformation,
	ObjectNameInformation,
	ObjectTypeInformation,
	ObjectAllInformation,
	ObjectDataInformation
}OBJECT_INFORMATION_CLASS, *POBJECT_INFORMATION_CLASS;
*/

typedef struct _OBJECT_BASIC_INFORMATION {
	ULONG                   Attributes;
	ACCESS_MASK             DesiredAccess;
	ULONG                   HandleCount;
	ULONG                   ReferenceCount;
	ULONG                   PagedPoolUsage;
	ULONG                   NonPagedPoolUsage;
	ULONG                   Reserved[3];
	ULONG                   NameInformationLength;
	ULONG                   TypeInformationLength;
	ULONG                   SecurityDescriptorLength;
	LARGE_INTEGER           CreationTime;
} OBJECT_BASIC_INFORMATION, *POBJECT_BASIC_INFORMATION;

/*
typedef struct _OBJECT_NAME_INFORMATION {
	UNICODE_STRING          Name;
	WCHAR                   NameBuffer[0];
} OBJECT_NAME_INFORMATION, *POBJECT_NAME_INFORMATION;
*/
/*
typedef struct _OBJECT_TYPE_INFORMATION {
	UNICODE_STRING          TypeName;
	ULONG                   TotalNumberOfHandles;
	ULONG                   TotalNumberOfObjects;
	WCHAR                   Unused1[8];
	ULONG                   HighWaterNumberOfHandles;
	ULONG                   HighWaterNumberOfObjects;
	WCHAR                   Unused2[8];
	ACCESS_MASK             InvalidAttributes;
	GENERIC_MAPPING         GenericMapping;
	ACCESS_MASK             ValidAttributes;
	BOOLEAN                 SecurityRequired;
	BOOLEAN                 MaintainHandleCount;
	USHORT                  MaintainTypeList;
	POOL_TYPE               PoolType;
	ULONG                   DefaultPagedPoolCharge;
	ULONG                   DefaultNonPagedPoolCharge;
} OBJECT_TYPE_INFORMATION, *POBJECT_TYPE_INFORMATION;
*/


typedef struct _OBJECT_HANDLE_FLAG_INFORMATION {
	BOOLEAN Inherit;
	BOOLEAN ProtectFromClose;
}OBJECT_HANDLE_FLAG_INFORMATION, *POBJECT_HANDLE_FLAG_INFORMATION;

EXTERN_C NTSTATUS NTAPI  ObSetHandleAttributes(HANDLE Handle, POBJECT_HANDLE_FLAG_INFORMATION HandleFlags, KPROCESSOR_MODE PreviousMode);


BOOLEAN ForceCloseHandle(HANDLE pid, HANDLE handle);
NTSTATUS EnumHandleInfoByPid(LPVOID buf, ULONG len, HANDLE pid);
NTSTATUS StorageUnlockEnum(PVOID inbuf, ULONG inlen, PVOID outbuf, ULONG outlen, PIRP irp);
NTSTATUS StorageUnlockClose(PVOID inbuf, ULONG inlen, PVOID outbuf, ULONG outlen, PIRP irp);