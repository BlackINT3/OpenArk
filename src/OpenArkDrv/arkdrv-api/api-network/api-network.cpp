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
#include <Winsock2.h>
#include <ws2ipdef.h>
#include <IPHlpApi.h>
#include <vector>
#include <mstcpip.h>
#include <ws2tcpip.h>
#include "api-network.h"
#pragma comment(lib, "IPHlpApi.lib")

#ifdef _ARKDRV_
#else
namespace ArkDrvApi {
namespace Network {
char* GetReadableState(int state)
{
	switch (state) {
	case MIB_TCP_STATE_CLOSED:
		return "CLOSED";
	case MIB_TCP_STATE_LISTEN:
		return "LISTENING";
	case MIB_TCP_STATE_ESTAB:
		return "ESTABLISHED";
	case MIB_TCP_STATE_SYN_SENT:
		return "SYN_SENT";
	case MIB_TCP_STATE_SYN_RCVD:
		return "SYN_RECV";
	case MIB_TCP_STATE_FIN_WAIT1:
		return "FIN_WAIT1";
	case MIB_TCP_STATE_FIN_WAIT2:
		return "FIN_WAIT2";
	case MIB_TCP_STATE_CLOSE_WAIT:
		return "CLOSE_WAIT";
	case MIB_TCP_STATE_CLOSING:
		return "CLOSING";
	case MIB_TCP_STATE_LAST_ACK:
		return "LAST_ACK";
	case MIB_TCP_STATE_TIME_WAIT:
		return "TIME_WAIT";
	case MIB_TCP_STATE_DELETE_TCB:
		return "DELETE_TCB";
	default:
		return "UNKNOWN";
	}
}

bool EnumTcp4Endpoints(std::vector<ARK_NETWORK_ENDPOINT_ITEM> &items)
{
	DWORD size = 0;
	PMIB_TCPTABLE_OWNER_PID tcp = NULL;
	if (GetExtendedTcpTable(tcp, &size, TRUE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0) != ERROR_INSUFFICIENT_BUFFER)
		return false;

	tcp = (MIB_TCPTABLE_OWNER_PID *)new char[size];
	if (GetExtendedTcpTable(tcp, &size, TRUE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0) != NO_ERROR) {
		delete tcp;
		return false;
	}
	int nums = (int)tcp->dwNumEntries;
	for (int i = 0; i < nums; i++) {
		ARK_NETWORK_ENDPOINT_ITEM endpoint;
		endpoint.ip_ver = ARK_NETWORK_IPV4;
		endpoint.tran_ver = ARK_NETWORK_TCP;
		strcpy_s(endpoint.protocol, "TCP");
		endpoint.u0.local_addr = tcp->table[i].dwLocalAddr;
		endpoint.u1.remote_addr = tcp->table[i].dwRemoteAddr;
		endpoint.local_port = tcp->table[i].dwLocalPort;
		endpoint.remote_port = tcp->table[i].dwRemotePort;
		endpoint.state = tcp->table[i].dwState;
		endpoint.pid = tcp->table[i].dwOwningPid;
		strcpy_s(endpoint.readable_state, GetReadableState(endpoint.state));
		sprintf_s(endpoint.local, "%s:%d", inet_ntoa(*(in_addr*)& tcp->table[i].dwLocalAddr), htons(tcp->table[i].dwLocalPort));
		if (endpoint.state == MIB_TCP_STATE_LISTEN) {
			strcpy_s(endpoint.remote, "0.0.0.0:0");
		}
		else {
			sprintf_s(endpoint.remote, "%s:%d", inet_ntoa(*(in_addr*)& tcp->table[i].dwRemoteAddr), htons(tcp->table[i].dwRemotePort));
		}
		items.push_back(endpoint);
	}
	delete tcp;
	//for (auto i : items) printf("%s	%s	%s	%s	%d\n", i.protocol, i.local, i.remote, i.readable_state, i.pid);
	return 0;
}

bool EnumTcp6Endpoints(std::vector<ARK_NETWORK_ENDPOINT_ITEM> &items)
{
	typedef PSTR(NTAPI *__RtlIpv6AddressToStringA)(
		_In_ const struct in6_addr *Addr,
		_Out_writes_(46) PSTR S
		);
	auto pRtlIpv6AddressToStringA = (__RtlIpv6AddressToStringA)
		GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlIpv6AddressToStringA");
	if (!pRtlIpv6AddressToStringA) return false;

	DWORD size = 0;
	PMIB_TCP6TABLE_OWNER_PID tcp = NULL;
	if (GetExtendedTcpTable(tcp, &size, TRUE, AF_INET6, TCP_TABLE_OWNER_PID_ALL, 0) != ERROR_INSUFFICIENT_BUFFER)
		return false;

	tcp = (MIB_TCP6TABLE_OWNER_PID *)new char[size];
	if (GetExtendedTcpTable(tcp, &size, TRUE, AF_INET6, TCP_TABLE_OWNER_PID_ALL, 0) != NO_ERROR) {
		delete[] tcp;
		return false;
	}
	int nums = (int)tcp->dwNumEntries;
	for (int i = 0; i < nums; i++) {
		ARK_NETWORK_ENDPOINT_ITEM endpoint;
		endpoint.ip_ver = ARK_NETWORK_IPV6;
		endpoint.tran_ver = ARK_NETWORK_TCP;
		strcpy_s(endpoint.protocol, "TCP6");
		RtlCopyMemory(&endpoint.u0.local_addr6, &tcp->table[i].ucLocalAddr, 16);
		RtlCopyMemory(&endpoint.u1.remote_addr6, &tcp->table[i].ucRemoteAddr, 16);
		endpoint.local_port = tcp->table[i].dwLocalPort;
		endpoint.remote_port = tcp->table[i].dwRemotePort;
		endpoint.state = tcp->table[i].dwState;
		endpoint.pid = tcp->table[i].dwOwningPid;
		strcpy_s(endpoint.readable_state, GetReadableState(endpoint.state));

		CHAR str[64] = { 0 };
		pRtlIpv6AddressToStringA((in6_addr*)&endpoint.u0.local_addr6, (char*)&str);
		sprintf_s(endpoint.local, "[%s]:%d", &str, htons(endpoint.local_port));
		if (endpoint.state == MIB_TCP_STATE_LISTEN) {
			strcpy_s(endpoint.remote, "[::]:0");
		}
		else {
			RtlZeroMemory(&str, sizeof(str));
			pRtlIpv6AddressToStringA((in6_addr*)&endpoint.u1.remote_addr6, (char*)&str);
			sprintf_s(endpoint.remote, "[%s]:%d", &str, htons(endpoint.remote_port));
		}
		items.push_back(endpoint);
	}
	delete[] tcp;
	//for (auto i : items) printf("%s	%s	%s	%s	%d\n", i.protocol, i.local, i.remote, i.readable_state, i.pid);
	return 0;
}

bool EnumUdp4Endpoints(std::vector<ARK_NETWORK_ENDPOINT_ITEM> &items)
{
	DWORD size = 0;
	PMIB_UDPTABLE_OWNER_PID udp = NULL;
	if (GetExtendedUdpTable(udp, &size, TRUE, AF_INET, UDP_TABLE_OWNER_PID, 0) != ERROR_INSUFFICIENT_BUFFER)
		return false;

	udp = (MIB_UDPTABLE_OWNER_PID *)new char[size];
	if (GetExtendedUdpTable(udp, &size, TRUE, AF_INET, UDP_TABLE_OWNER_PID, 0) != NO_ERROR) {
		delete udp;
		return false;
	}
	int nums = (int)udp->dwNumEntries;
	for (int i = 0; i < nums; i++) {
		ARK_NETWORK_ENDPOINT_ITEM endpoint;
		endpoint.ip_ver = ARK_NETWORK_IPV4;
		endpoint.tran_ver = ARK_NETWORK_UDP;
		strcpy_s(endpoint.protocol, "UDP");
		endpoint.u0.local_addr = udp->table[i].dwLocalAddr;
		endpoint.local_port = udp->table[i].dwLocalPort;
		endpoint.pid = udp->table[i].dwOwningPid;
		sprintf_s(endpoint.local, "%s:%d", inet_ntoa(*(in_addr*)&udp->table[i].dwLocalAddr), htons(udp->table[i].dwLocalPort));
		strcpy_s(endpoint.remote, "*:*");
		items.push_back(endpoint);
	}
	delete udp;
	//for (auto i : items) printf("%s	%s	%s	%s	%d\n", i.protocol, i.local, i.remote, i.readable_state, i.pid);
	return 0;
}

bool EnumUdp6Endpoints(std::vector<ARK_NETWORK_ENDPOINT_ITEM> &items)
{
	typedef PSTR(NTAPI *__RtlIpv6AddressToStringA)(
		_In_ const struct in6_addr *Addr,
		_Out_writes_(46) PSTR S
		);
	auto pRtlIpv6AddressToStringA = (__RtlIpv6AddressToStringA)
		GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlIpv6AddressToStringA");
	if (!pRtlIpv6AddressToStringA) return false;

	DWORD size = 0;
	PMIB_UDP6TABLE_OWNER_PID udp = NULL;
	if (GetExtendedUdpTable(udp, &size, TRUE, AF_INET6, UDP_TABLE_OWNER_PID, 0) != ERROR_INSUFFICIENT_BUFFER)
		return false;

	udp = (MIB_UDP6TABLE_OWNER_PID *)new char[size];
	if (GetExtendedUdpTable(udp, &size, TRUE, AF_INET6, UDP_TABLE_OWNER_PID, 0) != NO_ERROR) {
		delete udp;
		return false;
	}
	int nums = (int)udp->dwNumEntries;
	for (int i = 0; i < nums; i++) {
		ARK_NETWORK_ENDPOINT_ITEM endpoint;
		endpoint.ip_ver = ARK_NETWORK_IPV6;
		endpoint.tran_ver = ARK_NETWORK_UDP;
		strcpy_s(endpoint.protocol, "UDP6");
		RtlCopyMemory(&endpoint.u0.local_addr6, &udp->table[i].ucLocalAddr, 16);
		endpoint.local_port = udp->table[i].dwLocalPort;
		endpoint.pid = udp->table[i].dwOwningPid;
		CHAR str[64] = { 0 };
		pRtlIpv6AddressToStringA((in6_addr*)&endpoint.u0.local_addr6, (char*)&str);
		sprintf_s(endpoint.local, "[%s]:%d", &str, htons(endpoint.local_port));
		strcpy_s(endpoint.remote, "*:*");
		items.push_back(endpoint);
	}
	delete udp;
	//for (auto i : items) printf("%s	%s	%s	%s	%d\n", i.protocol, i.local, i.remote, i.readable_state, i.pid);
	return 0;
}
} // namespace Network
} // namespace ArkDrvApi
#endif