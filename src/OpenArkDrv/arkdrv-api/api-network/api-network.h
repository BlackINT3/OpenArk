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

enum ARK_NETWORK_OPS {
	
};

#define ARK_NETWORK_IPV4 4
#define ARK_NETWORK_IPV6 6
#define ARK_NETWORK_TCP 0
#define ARK_NETWORK_UDP 1

#pragma pack(push, 1)
typedef struct _ARK_NETWORK_ENDPOINT_ITEM {
	ULONG ip_ver;
	ULONG tran_ver;
	CHAR protocol[8];
	union {
		ULONG local_addr;
		UCHAR local_addr6[16];
	} u0;
	union {
		ULONG remote_addr;
		UCHAR remote_addr6[16];
	} u1;
	ULONG local_port;
	ULONG remote_port;
	CHAR local[64];
	CHAR remote[64];
	ULONG state;
	CHAR readable_state[32];
	ULONG pid;
} ARK_NETWORK_ENDPOINT_ITEM, *PARK_NETWORK_ENDPOINT_ITEM;
#pragma pack(pop)

//#undef _ARKDRV_
#ifdef _ARKDRV_
#include <ntifs.h>
#else
#include <unone.h>
#include <string>
#include <vector>
namespace ArkDrvApi {
namespace Network {
	bool EnumTcp4Endpoints(std::vector<ARK_NETWORK_ENDPOINT_ITEM> &items);
	bool EnumTcp6Endpoints(std::vector<ARK_NETWORK_ENDPOINT_ITEM> &items);
	bool EnumUdp4Endpoints(std::vector<ARK_NETWORK_ENDPOINT_ITEM> &items);
	bool EnumUdp6Endpoints(std::vector<ARK_NETWORK_ENDPOINT_ITEM> &items);
} // namespace Network
} // namespace ArkDrvApi
#endif //_NTDDK_