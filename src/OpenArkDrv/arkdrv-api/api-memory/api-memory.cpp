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
#include "api-memory.h"
#ifdef _ARKDRV_
#else
namespace ArkDrvApi {
namespace Memory {

bool MemoryRead(ULONG64 addr, ULONG size, std::string &readbuf)
{
	if (!size) return false;
	MEMORY_IN memin;
	memin.addr = addr;
	memin.size = size;
	DWORD outlen;
	PMEMORY_OUT memout;
	bool ret = IoControlDriver(IOCTL_ARK_MEMORY, MEMORY_READ, &memin, sizeof(memin), (PVOID*)&memout, &outlen);
	if (!ret)	 return false;
	readbuf.resize(memout->size);
	memcpy(&readbuf[0], memout->readbuf, memout->size);
	free(memout);
	return true;
}
} // namespace Memory
} // namespace ArkDrvApi
#endif