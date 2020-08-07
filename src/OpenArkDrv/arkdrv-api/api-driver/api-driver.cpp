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
#include "api-driver.h"
#ifdef _ARKDRV_
#else
namespace ArkDrvApi {
namespace Driver {

bool DriverEnumInfo(std::vector<DRIVER_ITEM> &infos)
{
	infos.clear();
	DWORD op = DRIVER_ENUM_INFO;
	PDRIVER_INFO drivers;
	DWORD outlen;
	bool ret = IoControlDriver(IOCTL_ARK_DRIVER, op, NULL, 0, (PVOID*)&drivers, &outlen);
	if (!ret) return false;
	for (int i = 0; i < drivers->count; i++) {
		infos.push_back(drivers->items[i]);
	}
	free(drivers);
	return true;
}
} // namespace Memory
} // namespace ArkDrvApi
#endif