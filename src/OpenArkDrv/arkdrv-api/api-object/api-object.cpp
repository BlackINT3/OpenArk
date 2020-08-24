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
#include "api-object.h"
#ifdef _ARKDRV_
#else

#undef ALIGN_DOWN_BY
#undef ALIGN_UP_BY
#undef ALIGN_DOWN_POINTER_BY
#undef ALIGN_UP_POINTER_BY
#undef ALIGN_DOWN
#undef ALIGN_UP
#undef ALIGN_DOWN_POINTER
#undef ALIGN_UP_POINTER

#define ALIGN_DOWN_BY(length, alignment) \
    ((ULONG_PTR)(length) & ~(alignment - 1))

#define ALIGN_UP_BY(length, alignment) \
    (ALIGN_DOWN_BY(((ULONG_PTR)(length) + alignment - 1), alignment))

#define ALIGN_DOWN_POINTER_BY(address, alignment) \
    ((PVOID)((ULONG_PTR)(address) & ~((ULONG_PTR)alignment - 1)))

#define ALIGN_UP_POINTER_BY(address, alignment) \
    (ALIGN_DOWN_POINTER_BY(((ULONG_PTR)(address) + alignment - 1), alignment))

#define ALIGN_DOWN(length, type) \
    ALIGN_DOWN_BY(length, sizeof(type))

#define ALIGN_UP(length, type) \
    ALIGN_UP_BY(length, sizeof(type))

#define ALIGN_DOWN_POINTER(address, type) \
    ALIGN_DOWN_POINTER_BY(address, sizeof(type))

#define ALIGN_UP_POINTER(address, type) \
    ALIGN_UP_POINTER_BY(address, sizeof(type))

typedef struct _OBJECT_TYPE_INFORMATION2 {
	UNICODE_STRING TypeName;
	ULONG TotalNumberOfObjects;
	ULONG TotalNumberOfHandles;
	ULONG TotalPagedPoolUsage;
	ULONG TotalNonPagedPoolUsage;
	ULONG TotalNamePoolUsage;
	ULONG TotalHandleTableUsage;
	ULONG HighWaterNumberOfObjects;
	ULONG HighWaterNumberOfHandles;
	ULONG HighWaterPagedPoolUsage;
	ULONG HighWaterNonPagedPoolUsage;
	ULONG HighWaterNamePoolUsage;
	ULONG HighWaterHandleTableUsage;
	ULONG InvalidAttributes;
	GENERIC_MAPPING GenericMapping;
	ULONG ValidAccessMask;
	BOOLEAN SecurityRequired;
	BOOLEAN MaintainHandleCount;
	UCHAR TypeIndex; // since WINBLUE
	CHAR ReservedByte;
	ULONG PoolType;
	ULONG DefaultPagedPoolCharge;
	ULONG DefaultNonPagedPoolCharge;
} OBJECT_TYPE_INFORMATION2, *POBJECT_TYPE_INFORMATION2;

//#include <WtsApi32.h>
//#pragma comment(lib, "WtsApi32.lib")

namespace ArkDrvApi {
namespace Object {
bool ObjectTypeEnumR3(std::vector<ARK_OBJECT_TYPE_ITEM> &items)
{
	NTSTATUS status;
	__NtQueryObject pNtQueryObject = (__NtQueryObject)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryObject");
	if (!pNtQueryObject) return false;
	ULONG bufsize = PAGE_SIZE;
	PVOID buf = VirtualAlloc(NULL, bufsize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	while ((status = pNtQueryObject(
		INVALID_HANDLE_VALUE,
		ObjectTypesInformation,
		buf,
		bufsize,
		NULL
	)) == STATUS_INFO_LENGTH_MISMATCH) {
		VirtualFree(buf, bufsize, MEM_RELEASE);
		bufsize *= 2;
		buf = VirtualAlloc(NULL, bufsize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	}
	if (!NT_SUCCESS(status)) {
		VirtualFree(buf, bufsize, MEM_RELEASE);
		return status;
	}
	POBJECT_TYPES_INFORMATION obj_types = (POBJECT_TYPES_INFORMATION)buf;
	POBJECT_TYPE_INFORMATION obj_info = (POBJECT_TYPE_INFORMATION)(((PUCHAR)obj_types) + ALIGN_UP(sizeof(*obj_types), ULONG_PTR));
	for (ULONG i = 0; i < obj_types->NumberOfTypes; i++) {
		ARK_OBJECT_TYPE_ITEM item;
		item.type_index = i + 2;
		RtlZeroMemory(item.type_name, sizeof(item.type_name));
		RtlCopyMemory(item.type_name, obj_info->TypeName.Buffer, obj_info->TypeName.Length*2);
		std::wstring &&type_name = UNONE::StrTrimRightW(item.type_name);
		RtlCopyMemory(item.type_name, type_name.c_str(), (type_name.size()+1) * 2);
		item.type_object = NULL;
		item.total_objects = obj_info->TotalNumberOfObjects;
		item.total_handles = obj_info->TotalNumberOfHandles;
		items.push_back(item);

		obj_info = (POBJECT_TYPE_INFORMATION)
			((PCHAR)(obj_info + 1) + ALIGN_UP(obj_info->TypeName.MaximumLength, ULONG_PTR));
	}
	return true;
}

bool ObjectTypeEnum(std::vector<ARK_OBJECT_TYPE_ITEM> &items)
{
	return ObjectTypeEnumR3(items);
}

typedef NTSTATUS(NTAPI *__NtOpenDirectoryObject)(
	__out PHANDLE DirectoryHandle,
	__in ACCESS_MASK DesiredAccess,
	__in POBJECT_ATTRIBUTES ObjectAttributes
);
typedef NTSTATUS(NTAPI *__NtQueryDirectoryObject)(
	__in HANDLE DirectoryHandle,
	__out_bcount_opt(Length) PVOID Buffer,
	__in ULONG Length,
	__in BOOLEAN ReturnSingleEntry,
	__in BOOLEAN RestartScan,
	__inout PULONG Context,
	__out_opt PULONG ReturnLength
);
typedef struct _OBJECT_DIRECTORY_INFORMATION {
	UNICODE_STRING Name;
	UNICODE_STRING TypeName;
} OBJECT_DIRECTORY_INFORMATION, *POBJECT_DIRECTORY_INFORMATION;

typedef enum _WTS_CONNECTSTATE_CLASS {
	WTSActive,              // User logged on to WinStation
	WTSConnected,           // WinStation connected to client
	WTSConnectQuery,        // In the process of connecting to client
	WTSShadow,              // Shadowing another WinStation
	WTSDisconnected,        // WinStation logged on without client
	WTSIdle,                // Waiting for client to connect
	WTSListen,              // WinStation is listening for connection
	WTSReset,               // WinStation is being reset
	WTSDown,                // WinStation is down due to error
	WTSInit,                // WinStation in initialization
} WTS_CONNECTSTATE_CLASS;
#define WTS_CURRENT_SERVER_HANDLE  ((HANDLE)NULL)

typedef struct _WTS_SESSION_INFOW {
	DWORD SessionId;             // session id
	LPWSTR pWinStationName;      // name of WinStation this session is
									   // connected to
	WTS_CONNECTSTATE_CLASS State; // connection state (see enum)
} WTS_SESSION_INFOW, *PWTS_SESSION_INFOW;

typedef struct _SESSION_INFOW {
	DWORD SessionId;             // session id
	std::wstring pWinStationName;      // name of WinStation this session is
								 // connected to
	WTS_CONNECTSTATE_CLASS State; // connection state (see enum)
} SESSION_INFOW, *PSESSION_INFOW;

bool GetSessions(std::vector<SESSION_INFOW> &sinfos)
{
	typedef BOOL (WINAPI *__WTSEnumerateSessionsW)(
		IN HANDLE          hServer,
		IN DWORD           Reserved,
		IN DWORD           Version,
		PWTS_SESSION_INFOW *ppSessionInfo,
		DWORD              *pCount
	);
	typedef void (WINAPI *__WTSFreeMemory)(
		IN PVOID pMemory
	);
	__WTSEnumerateSessionsW pWTSEnumerateSessionsW = (__WTSEnumerateSessionsW)GetProcAddress(GetModuleHandleA("Wtsapi32.dll"), "WTSEnumerateSessionsW");
	__WTSFreeMemory pWTSFreeMemory = (__WTSFreeMemory)GetProcAddress(GetModuleHandleA("Wtsapi32.dll"), "WTSFreeMemory");
	if (!pWTSEnumerateSessionsW || !pWTSFreeMemory) return false;

	DWORD scount = 0;
	PWTS_SESSION_INFOW sessions = NULL;
	BOOL ret = pWTSEnumerateSessionsW(WTS_CURRENT_SERVER_HANDLE, 0, 1, &sessions, &scount);
	if (!ret) return false;

	for (int i = 0; i < scount; i++) {
		SESSION_INFOW info;
		info.SessionId = sessions[i].SessionId;
		info.pWinStationName = sessions[i].pWinStationName;
		info.State = sessions[i].State;
		sinfos.push_back(info);
	}

	pWTSFreeMemory(sessions);
	return true;
}

bool ObjectSectionEnumR3(std::vector<ARK_OBJECT_SECTION_ITEM> &items, ULONG session)
{
	std::wstring dirname, prefix;

	if (session == ARK_SESSION_GLOBAL) {
		dirname = L"\\BaseNamedObjects";
		prefix = L"Global";
	} else {
		dirname = UNONE::StrFormatW(L"\\Sessions\\%u\\BaseNamedObjects", session);
		prefix = L"";
	}

	#define DIRECTORY_QUERY                 (0x0001)
	NTSTATUS status;
	__NtOpenDirectoryObject pNtOpenDirectoryObject = (__NtOpenDirectoryObject)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtOpenDirectoryObject");
	__NtQueryDirectoryObject pNtQueryDirectoryObject = (__NtQueryDirectoryObject)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryDirectoryObject");
	if (!pNtOpenDirectoryObject || !pNtQueryDirectoryObject) return false;

	HANDLE dirobj;
	OBJECT_ATTRIBUTES oa;
	UNICODE_STRING udirname;
	udirname.Buffer = (WCHAR*)dirname.c_str();
	udirname.Length = dirname.size() * 2;
	udirname.MaximumLength = udirname.Length;
	InitializeObjectAttributes(&oa, &udirname, 0, NULL, NULL);
	status = pNtOpenDirectoryObject(&dirobj, DIRECTORY_QUERY, &oa);
	if (!NT_SUCCESS(status)) return false;

	ULONG context, written;
	ULONG bufsize = 512;
	POBJECT_DIRECTORY_INFORMATION info = (POBJECT_DIRECTORY_INFORMATION)malloc(bufsize);
	if (!info) return false;
	status = pNtQueryDirectoryObject(dirobj, info, bufsize, TRUE, TRUE, &context, &written);
	if (!NT_SUCCESS(status)) {
		CloseHandle(dirobj);
		free(info);
		return false;
	}
	while (NT_SUCCESS(pNtQueryDirectoryObject(dirobj, info, bufsize, TRUE, FALSE, &context, &written))) {
		if (!wcsncmp(L"Section", info->TypeName.Buffer, 7)) {
			ARK_OBJECT_SECTION_ITEM item;
			RtlZeroMemory(item.section_name, sizeof(item.section_name));
			RtlZeroMemory(item.section_dir, sizeof(item.section_dir));
			wcsncpy(item.section_name, info->Name.Buffer, MIN(info->Name.Length / 2, 127));
			wcsncpy(item.section_dir, dirname.c_str(), MIN(dirname.size(), 127));
			std::wstring map_name;
			if (!prefix.empty()) {
				map_name = UNONE::StrFormatW(L"%s\\%s", prefix.c_str(), item.section_name);
			} else {
				map_name = item.section_name;
			}
			
			HANDLE maphd = OpenFileMappingW(FILE_MAP_READ, FALSE, map_name.c_str());
			if (maphd) {
				PVOID mapaddr = MapViewOfFileEx(maphd, FILE_MAP_READ, 0, 0, 0, NULL);
				MEMORY_BASIC_INFORMATION mbi;
				VirtualQuery(mapaddr, &mbi, sizeof(mbi));
				item.section_size = (ULONG)mbi.RegionSize;
				UnmapViewOfFile(mapaddr);
				CloseHandle(maphd);
			} else {
				ERR(L"%s %d", map_name.c_str(), GetLastError());
			}
			item.session_id = session;
			items.push_back(item);
		}
	}

	CloseHandle(dirobj);
	free(info);
	return true;
}

bool ObjectSectionEnum(std::vector<ARK_OBJECT_SECTION_ITEM> &items, ULONG session)
{
	return ObjectSectionEnumR3(items, session);
}

bool ObjectSectionEnum(std::vector<ARK_OBJECT_SECTION_ITEM> &items)
{
	std::vector<ARK_OBJECT_SECTION_ITEM> temps;
	ObjectSectionEnumR3(temps, ARK_SESSION_GLOBAL);
	items.insert(items.end(), temps.begin(), temps.end());
	for (auto &item : items) {
		wcscpy(item.session_name, L"Global");
	}

	std::vector<SESSION_INFOW> sinfos;
	GetSessions(sinfos);

	for (int i = 0; i < sinfos.size(); i++) {
		temps.clear();
		ObjectSectionEnumR3(temps, sinfos[i].SessionId);
		for (auto &item : temps) {
			wcscpy(item.session_name, sinfos[i].pWinStationName.c_str());
		}
		items.insert(items.end(), temps.begin(), temps.end());
	}


	return true;
}

} // namespace Object
} // namespace ArkDrvApi
#endif