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
#include "icons.h"
#include <unone.h>
using namespace std;

//#define FIELD_OFFSET(type, field) ((ULONG)&(((type *)0)->field))

typedef struct
{
	BYTE        bWidth;          // Width, in pixels, of the image
	BYTE        bHeight;         // Height, in pixels, of the image
	BYTE        bColorCount;     // Number of colors in image (0 if >=8bpp)
	BYTE        bReserved;       // Reserved ( must be 0)
	WORD        wPlanes;         // Color Planes
	WORD        wBitCount;       // Bits per pixel
	DWORD       dwBytesInRes;    // How many bytes in this resource?
	DWORD       dwImageOffset;   // Where in the file is this image?
} ICONDIRENTRY, *LPICONDIRENTRY;

typedef struct
{
	WORD           idReserved;   // Reserved (must be 0)
	WORD           idType;       // Resource Type (1 for icons)
	WORD           idCount;      // How many images?
															 //ICONDIRENTRY   idEntries[1]; // An entry for each image (idCount of 'em)
} ICONDIR, *LPICONDIR;

#pragma pack( push )
#pragma pack( 2 )
typedef struct
{
	BYTE   bWidth;               // Width, in pixels, of the image
	BYTE   bHeight;              // Height, in pixels, of the image
	BYTE   bColorCount;          // Number of colors in image (0 if >=8bpp)
	BYTE   bReserved;            // Reserved
	WORD   wPlanes;              // Color Planes
	WORD   wBitCount;            // Bits per pixel
	DWORD  dwBytesInRes;         // how many bytes in this resource?
	WORD   nID;                  // the ID
} GRPICONDIRENTRY, *LPGRPICONDIRENTRY;
#pragma pack( pop )

// #pragmas are used here to insure that the structure's
// packing in memory matches the packing of the EXE or DLL.
#pragma pack( push )
#pragma pack( 2 )
typedef struct
{
	WORD            idReserved;   // Reserved (must be 0)
	WORD            idType;       // Resource type (1 for icons)
	WORD            idCount;      // How many images?
	GRPICONDIRENTRY idEntries[1]; // The entries for each image
} GRPICONDIR, *LPGRPICONDIR;
#pragma pack( pop )

typedef struct
{
	LPCSTR			ResType;
	LPCSTR			ResName;
	DWORD			ResSize;
	WORD			ResLanguage;
} RESINFO, *PRESINFO;

BOOL CALLBACK EnumFirstResProc(HMODULE hModule, LPCSTR lpszType, LPSTR lpszName, LONG_PTR lParam)
{
	if (IS_INTRESOURCE(lpszName))
		sprintf_s((CHAR*)lParam, MAX_PATH, "#%d", (USHORT)lpszName);
	else
		strncpy_s((CHAR*)lParam, MAX_PATH, lpszName, MAX_PATH - 1);
	return FALSE;
}
BOOL CALLBACK EnumLastResIdProc(HMODULE hModule, LPCSTR lpszType, LPSTR lpszName, LONG_PTR lParam)
{
	if (IS_INTRESOURCE(lpszName))
		sprintf_s((CHAR*)lParam, MAX_PATH, "#%d", (USHORT)lpszName);
	return TRUE;
}
BOOL CALLBACK EnumFirstResLangProc(HMODULE hModule, LPCSTR lpszType, LPCSTR lpszName, WORD wIDLanguage, LONG_PTR lParam)
{
	*(WORD*)lParam = wIDLanguage;
	return FALSE;
}

bool GetResLanguage(HMODULE Module, LPCSTR ResType, LPCSTR ResName, WORD& Language)
{
	return EnumResourceLanguagesA(Module, (LPCSTR)RT_GROUP_ICON, ResName, EnumFirstResLangProc, (LONG_PTR)&Language) ? true : false;
}

bool GetResData(HMODULE Module, LPCSTR ResType, LPCSTR ResName, LPVOID& Data, DWORD& Size)
{
	HRSRC Res = FindResourceA(Module, ResName, ResType);
	if (!Res)
		return false;
	HGLOBAL Global = LoadResource(Module, Res);
	if (!Global)
		return false;
	Size = SizeofResource(Module, Res);
	Data = LockResource(Global);
	if (!Data)
		return false;
	return true;
}
bool SetResData(HANDLE Update, LPCSTR ResType, LPCSTR ResName, LPVOID& Data, DWORD& Size, WORD wLanguage = MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT))
{
	return UpdateResourceA(Update, ResType, ResName, wLanguage, Data, Size) ? true : false;
}
bool DeleteResData(HANDLE Update, LPCSTR ResType, LPCSTR ResName, WORD ResLanguage)
{
	return UpdateResourceA(Update, ResType, ResName, ResLanguage, NULL, 0) ? true : false;
	return true;
}
bool GetIconData(const std::wstring& ExePath, std::string& IconData)
{
	bool Result = false;
	HMODULE Module = NULL;
	LPICONDIRENTRY IconEntry = NULL;
	if (ExePath.empty())
		return false;
	int pos = ExePath.find_last_of(L".");
	if (pos != std::wstring::npos && ExePath.substr(pos) == L".ico") {
		return UNONE::FsReadFileDataW(ExePath, IconData);
	}
	do {
		Module = LoadLibraryExW(ExePath.c_str(), NULL, LOAD_LIBRARY_AS_DATAFILE);
		if (!Module)
			break;

		CHAR ResGroupName[MAX_PATH] = { 0 };
		if (!EnumResourceNamesA(Module, (LPCSTR)RT_GROUP_ICON, EnumFirstResProc, (LONG_PTR)ResGroupName) &&
			GetLastError() == ERROR_RESOURCE_TYPE_NOT_FOUND)
			break;

		LPCSTR ResName = NULL;
		if (ResGroupName[0] == '#')
			ResName = MAKEINTRESOURCEA(atoi(&ResGroupName[1]));
		else
			ResName = ResGroupName;
		LPGRPICONDIR ResGroupData = NULL;
		DWORD ResGroupSize = 0;
		if (!GetResData(Module, (LPCSTR)RT_GROUP_ICON, ResName, (LPVOID&)ResGroupData, ResGroupSize))
			break;
		DWORD Count = ResGroupData->idCount;
		ICONDIR IconDir;
		memcpy(&IconDir, ResGroupData, sizeof(ICONDIR));

		DWORD Offset = 0, Size = 0;
		IconEntry = new(std::nothrow) ICONDIRENTRY[Count];
		if (!IconEntry)
			break;
		Offset += sizeof(ICONDIR) + sizeof(ICONDIRENTRY)*Count;
		for (DWORD i = 0; i < Count; i++)
		{
			memcpy(&IconEntry[i], &ResGroupData->idEntries[i], sizeof(GRPICONDIRENTRY));
			LPVOID IconItem = NULL;
			DWORD IconItemSize = 0;
			GetResData(Module, (LPCSTR)RT_ICON, MAKEINTRESOURCEA(ResGroupData->idEntries[i].nID), IconItem, IconItemSize);
			Size = Offset + IconItemSize;
			IconData.resize(Size);
			memcpy(&IconData[Offset], IconItem, IconItemSize);
			IconEntry[i].dwImageOffset = Offset;
			IconEntry[i].dwBytesInRes = IconItemSize;
			Offset += IconItemSize;
			UnlockResource(IconItem);
		}
		UnlockResource(ResGroupData);
		memcpy(&IconData[0], &IconDir, sizeof(ICONDIR));
		memcpy(&IconData[sizeof(ICONDIR)], IconEntry, sizeof(ICONDIRENTRY)*Count);
		Result = true;
	} while (0);
	if (Module)
		FreeLibrary(Module);
	if (IconEntry)
		delete[] IconEntry;
	return Result;
}
bool SetIconData(const std::wstring& ExePath, std::string& IconData)
{
	bool Result = false;
	HMODULE Module = NULL;
	LPICONDIRENTRY IconEntry = NULL;
	if (ExePath.empty())
		return false;
	LPGRPICONDIR ResGroupData = NULL;
	BOOL IsGroupExsited = FALSE;
	LPCSTR GroupName = NULL;
	WORD OriResLang = 0;
	Module = LoadLibraryExW(ExePath.c_str(), NULL, LOAD_LIBRARY_AS_DATAFILE);
	if (!Module)
		return false;
	do
	{
		CHAR Name[MAX_PATH] = { 0 };
		if (!EnumResourceNamesA(Module, (LPCSTR)RT_GROUP_ICON, EnumFirstResProc, (LONG_PTR)Name))
			break;
		if (Name[0] == '#')
			GroupName = MAKEINTRESOURCEA(atoi(&Name[1]));
		else
			GroupName = Name;
		DWORD ResGroupSize = 0;
		GetResData(Module, (LPCSTR)RT_GROUP_ICON, GroupName, (LPVOID&)ResGroupData, ResGroupSize);
		GetResLanguage(Module, (LPCSTR)RT_GROUP_ICON, GroupName, OriResLang);
		IsGroupExsited = TRUE;
	} while (0);

	GRPICONDIR TempGroupData;
	if (!IsGroupExsited)
	{
		ResGroupData = &TempGroupData;
		ResGroupData->idCount = 0;
		GroupName = MAKEINTRESOURCEA(1);
	}

	vector<DWORD> AddResId;
	vector<RESINFO> DelResInfo;
	LPICONDIR IconDir = (LPICONDIR)&IconData[0];
	LPICONDIRENTRY IconDirEntry = (LPICONDIRENTRY)&IconData[sizeof(ICONDIR)];
	for (DWORD i = 0; i < ResGroupData->idCount; i++)
	{
		RESINFO Info =
		{
			(LPCSTR)RT_ICON,
			MAKEINTRESOURCEA(ResGroupData->idEntries[i].nID),
			ResGroupData->idEntries[i].dwBytesInRes,
			OriResLang
		};
		DelResInfo.push_back(Info);
	}
	if (IconDir->idCount == ResGroupData->idCount)
	{
		for (DWORD i = 0; i < ResGroupData->idCount; i++)
			AddResId.push_back((DWORD)ResGroupData->idEntries[i].nID);
	}
	else if (IconDir->idCount < ResGroupData->idCount)
	{
		for (DWORD i = 0; i < ResGroupData->idCount; i++)
		{
			if (i < IconDir->idCount)
				AddResId.push_back((DWORD)ResGroupData->idEntries[i].nID);
		}
	}
	else
	{
		DWORD i, LastId;
		CHAR Str[MAX_PATH] = { 0 };
		for (i = 0; i < ResGroupData->idCount; i++)
			AddResId.push_back((DWORD)ResGroupData->idEntries[i].nID);
		EnumResourceNamesA(Module, (LPCSTR)RT_ICON, EnumLastResIdProc, (LONG_PTR)Str);
		if (Str[0] == '#')
			LastId = atoi(&Str[1]) + 1;
		else
			LastId = 1;
		for (; i < IconDir->idCount; i++, LastId++)
			AddResId.push_back(LastId);
	}
	if (IsGroupExsited)
		UnlockResource(ResGroupData);
	FreeLibrary(Module);
	HANDLE Update = BeginUpdateResourceW(ExePath.c_str(), FALSE);
	if (!Update)
		return false;
	DWORD GroupResSize = 6 + sizeof(GRPICONDIRENTRY)*IconDir->idCount;
	PBYTE GroupResData = new(std::nothrow) BYTE[GroupResSize];
	LPGRPICONDIR NewGroupDir = (LPGRPICONDIR)GroupResData;
	LPGRPICONDIRENTRY NewGroupDirEntry = (LPGRPICONDIRENTRY)(GroupResData + 6);
	NewGroupDir->idReserved = IconDir->idReserved;
	NewGroupDir->idCount = IconDir->idCount;
	NewGroupDir->idType = IconDir->idType;

	if (IsGroupExsited)
	{
		for_each(begin(DelResInfo), end(DelResInfo), [&Update](RESINFO& Info)
		{
			DeleteResData(Update, Info.ResType, Info.ResName, Info.ResLanguage);
		});
		DeleteResData(Update, (LPCSTR)RT_GROUP_ICON, GroupName, OriResLang);
	}
	for (DWORD i = 0; i < IconDir->idCount; i++)
	{
		memcpy(&NewGroupDirEntry[i], &IconDirEntry[i], sizeof(GRPICONDIRENTRY));
		NewGroupDirEntry[i].nID = (WORD)AddResId[i];
		LPVOID ItemData = &IconData[0] + IconDirEntry[i].dwImageOffset;
		DWORD ItemSize = IconDirEntry[i].dwBytesInRes;
		SetResData(Update, (LPCSTR)RT_ICON, MAKEINTRESOURCEA(AddResId[i]), ItemData, ItemSize);
	}
	SetResData(Update, (LPCSTR)RT_GROUP_ICON, GroupName, (LPVOID&)GroupResData, GroupResSize, 1033);
	EndUpdateResource(Update, FALSE);
	if (GroupResData)
		delete[] GroupResData;
	return true;
}