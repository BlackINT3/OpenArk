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
#include "win-wrapper.h"
#include "../common/common.h"
#include <Dbghelp.h>
#pragma comment(lib, "Dbghelp.lib")

std::wstring FormatFileTime(FILETIME *file_tm)
{
	SYSTEMTIME systm;
	FileTimeToSystemTime(file_tm, &systm);
	UNONE::TmConvertZoneTime(systm, NULL);
	return UNONE::TmFormatSystemTimeW(systm, L"Y-M-D H:W:S");
}

std::wstring CalcFileTime(FILETIME *file_tm)
{
	SYSTEMTIME systm;
	FileTimeToSystemTime(file_tm, &systm);

	LARGE_INTEGER li;
	li.HighPart = file_tm->dwHighDateTime;
	li.LowPart = file_tm->dwLowDateTime;
	auto val = UNONE::TmFileTimeToMs(*file_tm);
	int hours = val / (3600 * 1000);
	int mins = (val % (3600 * 1000)) / (60 * 1000);
	int secs = (val % (60 * 1000)) / 1000;
	int mss = (val % (1000));

	return UNONE::StrFormatW(L"%d:%02d:%02d.%03d", hours, mins, secs, mss);
}

bool RetrieveThreadTimes(DWORD tid, std::wstring& ct, std::wstring& kt, std::wstring& ut)
{
	HANDLE thd = OpenThread(THREAD_QUERY_INFORMATION, FALSE, tid);
	if (!thd) {
		return false;
	}

	FILETIME create_tm;
	FILETIME exit_tm;
	FILETIME kern_tm;
	FILETIME user_tm;
	if (!GetThreadTimes(thd, &create_tm, &exit_tm, &kern_tm, &user_tm)) {
		CloseHandle(thd);
		return false;
	}
	CloseHandle(thd);

	ct = FormatFileTime(&create_tm);
	kt = CalcFileTime(&kern_tm);
	ut = CalcFileTime(&user_tm);
	return true;
}

std::wstring ProcessCreateTime(__in DWORD pid)
{
	HANDLE Process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
	if (!Process) {
		return L"";
	}

	FILETIME create_tm;
	FILETIME exit_tm;
	FILETIME kern_tm;
	FILETIME user_tm;
	if (!GetProcessTimes(Process, &create_tm, &exit_tm, &kern_tm, &user_tm)) {
		CloseHandle(Process);
		return L"";
	}
	CloseHandle(Process);
	return FormatFileTime(&create_tm);
}

bool CreateDump(DWORD pid, const std::wstring& path, bool mini)
{
	if (UNONE::PsIsX64(pid) && !UNONE::PsIsX64(GetCurrentProcessId())) {
		MsgBoxError("Can't dump 64-bit process.");
		return false;
	}
	MINIDUMP_TYPE dmp_type;
	if (mini) {
		dmp_type = (MINIDUMP_TYPE)(MiniDumpWithThreadInfo | MiniDumpWithFullMemoryInfo |
			MiniDumpWithProcessThreadData | MiniDumpWithHandleData | MiniDumpWithDataSegs);
	}
	else {
		dmp_type = (MINIDUMP_TYPE)(MiniDumpWithThreadInfo | MiniDumpWithFullMemoryInfo | MiniDumpWithTokenInformation |
			MiniDumpWithProcessThreadData | MiniDumpWithDataSegs | MiniDumpWithFullMemory | MiniDumpWithHandleData);
	}
	HANDLE phd = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (!phd) {
		return false;
	}
	HANDLE fd = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fd == INVALID_HANDLE_VALUE) {
		CloseHandle(phd);
		return false;
	}
	BOOL ret = MiniDumpWriteDump(phd, pid, fd, dmp_type, NULL, NULL, NULL);
	CloseHandle(fd);
	CloseHandle(phd);
	if (ret) {
		MsgBoxInfo("Create dump ok.");
	}
	else {
		MsgBoxError("Create dump failed.");
	}
	return ret == TRUE;
}

// Clipboard
void ClipboardCopyData(const std::string &data)
{
	OpenClipboard(NULL);
	EmptyClipboard();
	HGLOBAL hd = GlobalAlloc(GMEM_MOVEABLE, data.size() + 1);
	LPWSTR buf = (LPWSTR)GlobalLock(hd);
	memcpy(buf, data.c_str(), data.size());
	GlobalUnlock(hd);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, buf);
	CloseClipboard();
}

BOOL CALLBACK RetrieveWndCallback(HWND wnd, LPARAM param)
{
	DWORD pid;
	std::vector<HWND> &wnds = *(std::vector<HWND>*)param;
	wnds.push_back(wnd);
	return TRUE;
}

std::vector<HWND> GetSystemWnds()
{
	std::vector<HWND> wnds;
	EnumChildWindows(GetDesktopWindow(), (WNDENUMPROC)RetrieveWndCallback, (LPARAM)&wnds);
	return wnds;
}

int64_t FileTimeToInt64(FILETIME tm)
{
	return tm.dwHighDateTime << 32 | tm.dwLowDateTime;
}

double GetSystemUsageOfCPU()
{
	static int64_t s_idle = 0, s_kernel = 0, s_user = 0;
	FILETIME tm1, tm2, tm3;
	GetSystemTimes(&tm1, &tm2, &tm3);
	auto idle = FileTimeToInt64(tm1) - s_idle;
	auto kernel = FileTimeToInt64(tm2) - s_kernel;
	auto user = FileTimeToInt64(tm3) - s_user;
	s_idle = FileTimeToInt64(tm1);
	s_kernel = FileTimeToInt64(tm2);
	s_user = FileTimeToInt64(tm3);
	return (double)(kernel + user - idle) * 100 / (kernel + user);
}

double GetSystemUsageOfMemory()
{
	PERFORMANCE_INFORMATION perf = { 0 };
	GetPerformanceInfo(&perf, sizeof(perf));
	return (1.0 - (double)perf.PhysicalAvailable / perf.PhysicalTotal) * 100;
}

SIZE_T GetProcessPrivateWorkingSet(DWORD pid)
{
	PROCESS_MEMORY_COUNTERS_EX mm_info;
	if (!UNONE::MmGetProcessMemoryInfo(pid, mm_info))
		return 0;
	HANDLE phd = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	if (!phd) {
		return 0;
	}
	PPSAPI_WORKING_SET_INFORMATION ws;
	DWORD size = sizeof(PSAPI_WORKING_SET_BLOCK) * 1024 * 1024 * 4;
	ws = (PPSAPI_WORKING_SET_INFORMATION)new char[size];
	mm_info.WorkingSetSize;
	if (!QueryWorkingSet(phd, ws, size)) {
		ERR(L"QueryWorkingSet pid:%d, err:%d", pid, GetLastError());
		return 0;
	}
	SIZE_T shared = 0;
	for (size_t i = 0; i < ws->NumberOfEntries; i++) {
		if (ws->WorkingSetInfo[i].Shared) shared += PAGE_SIZE;
	}
	delete[] ws;
	CloseHandle(phd);
	return shared;
}