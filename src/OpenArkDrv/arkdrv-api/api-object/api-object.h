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
#include "../arkdrv-api.h"
#ifdef _ARKDRV_
#include <ntifs.h>
#include <windef.h>
#else
#include <Windows.h>
#endif //_NTDDK_

enum ARK_OBJECT_OPS {
	ARK_OBJECT_TYPE_ENUM,
};

#define ARK_SESSION_GLOBAL -1

#pragma pack(push, 1)
typedef struct _ARK_OBJECT_TYPE_ITEM {
	ULONG type_index;
	WCHAR type_name[128];
	PVOID type_object;
	ULONG total_objects;
	ULONG total_handles;
} ARK_OBJECT_TYPE_ITEM, *PARK_OBJECT_TYPE_ITEM;
typedef struct _OBJECT_TYPE_INFO {
	ULONG	count;
	ARK_OBJECT_TYPE_ITEM items[1];
} ARK_OBJECT_TYPE_INFO, *PARK_OBJECT_TYPE_INFO;

typedef struct _ARK_OBJECT_SECTION_ITEM {
	ULONG session_id;
	WCHAR session_name[128];
	ULONG section_size;
	WCHAR section_name[128];
	WCHAR section_dir[128];
} ARK_OBJECT_SECTION_ITEM, *PARK_OBJECT_SECTION_ITEM;
typedef struct _ARK_OBJECT_SECTION_INFO {
	ULONG	count;
	ARK_OBJECT_SECTION_ITEM items[1];
} ARK_OBJECT_SECTION_INFO, *PARK_OBJECT_SECTION_INFO;

#pragma pack(pop)

//#undef _ARKDRV_
#ifdef _ARKDRV_
#include <ntifs.h>
#else
#include <unone.h>
#include <string>
#include <vector>
namespace ArkDrvApi {
namespace Object {
	bool ObjectTypeEnum(std::vector<ARK_OBJECT_TYPE_ITEM> &items);
	bool ObjectSectionEnum(std::vector<ARK_OBJECT_SECTION_ITEM> &items);
	bool ObjectSectionEnum(std::vector<ARK_OBJECT_SECTION_ITEM> &items, ULONG session);
} // namespace Object
} // namespace ArkDrvApi
#endif //_NTDDK_