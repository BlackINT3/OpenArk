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

enum OBJECT_OPS {
	OBJECT_TYPE_ENUM,
};

#pragma pack(push, 1)
typedef struct _OBJECT_TYPE_ITEM {
	ULONG  type_index; // object type index
	ULONG  ref_count; // ref count
	HANDLE pid; // process id
	LPVOID object; // 
	HANDLE handle; //
	WCHAR   type_name[64]; // the object type name
	WCHAR   name[260]; // object name
} OBJECT_TYPE_ITEM, *POBJECT_TYPE_ITEM;

typedef struct _OBJECT_TYPE_INFO {
	ULONG	count;
	OBJECT_TYPE_ITEM items[1];
} OBJECT_TYPE_INFO, *POBJECT_TYPE_INFO;
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
	bool ObjectTypeEnum(std::vector<OBJECT_TYPE_ITEM> &items);
} // namespace Object
} // namespace ArkDrvApi
#endif //_NTDDK_