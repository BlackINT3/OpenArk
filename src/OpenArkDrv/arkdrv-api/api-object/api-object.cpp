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
namespace ArkDrvApi {
namespace Object {
bool ObjectTypeEnum(std::vector<OBJECT_TYPE_ITEM> &items)
{
	if (!ConnectDriver()) return false;
	DWORD op = OBJECT_TYPE_ENUM;
	PHANDLE_INFO info;
	DWORD outlen;
	bool ret = IoControlDriver(IOCTL_ARK_OBJECT, op, (PVOID)path.c_str(), (path.size()+1)*2, (PVOID*)&info, &outlen);
	if (!ret) return false;
	for (int i = 0; i < info->count; i++) {
		items.push_back(info->items[i]);
	}
	free(info);
	return true;
}
} // namespace Object
} // namespace ArkDrvApi
#endif