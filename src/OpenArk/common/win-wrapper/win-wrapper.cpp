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
#include <QString>
#include <QtCore>

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

#include <Dbghelp.h>
#pragma comment(lib, "Dbghelp.lib")
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
		MsgBoxInfo(QObject::tr("Create dump ok."));
	}	else {
		MsgBoxError(QObject::tr("Create dump failed."));
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
	int64_t high = (int64_t)tm.dwHighDateTime;
	return high << 32 | tm.dwLowDateTime;
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

void SetWindowOnTop(HWND wnd, bool ontop)
{
	DWORD style = ::GetWindowLong(wnd, GWL_EXSTYLE);
	if (ontop) {
		style |= WS_EX_TOPMOST;
		::SetWindowLong(wnd, GWL_EXSTYLE, style);
		::SetWindowPos(wnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOREPOSITION | SWP_NOSIZE | SWP_SHOWWINDOW);
	} else {
		style &= ~WS_EX_TOPMOST;
		::SetWindowLong(wnd, GWL_EXSTYLE, style);
		::SetWindowPos(wnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOREPOSITION | SWP_NOSIZE | SWP_SHOWWINDOW);
	}
}

void WinShowProperties(const std::wstring &path)
{
	SHELLEXECUTEINFOW shell;
	shell.hwnd = NULL;
	shell.lpVerb = L"properties";
	shell.lpFile = path.c_str();
	shell.lpDirectory = NULL;
	shell.lpParameters = NULL;
	shell.nShow = SW_SHOWNORMAL;
	shell.fMask = SEE_MASK_INVOKEIDLIST;
	shell.lpIDList = NULL;
	shell.cbSize = sizeof(SHELLEXECUTEINFOW);
	ShellExecuteExW(&shell);
}

bool GetCertOwner(const QString &path, QString &owner)
{
	std::vector<UNONE::CertInfoW> infos;
	bool ret = UNONE::SeGetCertInfoW(path.toStdWString(), infos);
	if (ret) {
		owner = WStrToQ(infos[0].owner);
	}
	return ret;
}

struct OBJECTNAME_THREAD_DATA
{
	HANDLE hd;
	HANDLE query_evt;
	HANDLE alloc_evt;
	HANDLE start_evt;
	WCHAR* ObjectName;
};

#include <memory>
static __NtQueryObject pNtQueryObjectPtr = NULL;

DWORD WINAPI ObGetObjectNameThread(LPVOID params)
{
	auto caller = (OBJECTNAME_THREAD_DATA*)params;
	HANDLE hd = caller->hd;
	HANDLE query_evt = caller->query_evt;
	HANDLE alloc_evt = caller->alloc_evt;
	HANDLE start_evt = caller->start_evt;
	WCHAR* &obj_name = caller->ObjectName;
	NTSTATUS status;
	ULONG bufsize;
	POBJECT_NAME_INFORMATION buf;

	SetEvent(start_evt);

	if (pNtQueryObjectPtr == NULL) {
		return false;
	}
	status = pNtQueryObjectPtr(hd,
		ObjectNameInformation,
		NULL,
		0,
		&bufsize);
	if (status != STATUS_INFO_LENGTH_MISMATCH) {
		return false;
	}
	std::unique_ptr<CHAR> ptr(new(std::nothrow) CHAR[bufsize]);
	buf = (POBJECT_NAME_INFORMATION)ptr.get();
	if (buf == nullptr) {
	}
	status = pNtQueryObjectPtr(hd,
		ObjectNameInformation,
		buf,
		bufsize,
		&bufsize);
	if (!NT_SUCCESS(status)) {
		return false;
	}
	auto& name_buf = buf->Name.Buffer;
	auto& name_size = buf->Name.Length;
	if (name_buf == NULL) {
		return false;
	}
	SetEvent(query_evt);
	WaitForSingleObject(alloc_evt, INFINITE);
	obj_name = (WCHAR*)malloc(name_size + 2);
	memset(obj_name, 0, name_size + 2);
	if (obj_name != NULL) {
		memcpy(obj_name, name_buf, name_size);
	}
	SetEvent(query_evt);
	return true;
}

bool ObGetObjectName(HANDLE hd, std::string& obj_name)
{
	bool ret = false;
	OBJECTNAME_THREAD_DATA caller;
	caller.hd = hd;
	auto& query_evt = caller.query_evt = CreateEventA(NULL, FALSE, FALSE, NULL);
	if (query_evt == NULL) {
		return false;
	}
	auto& alloc_evt = caller.alloc_evt = CreateEventA(NULL, FALSE, FALSE, NULL);
	if (alloc_evt == NULL) {
		CloseHandle(query_evt);
		return false;
	}
	auto& start_evt = caller.start_evt = CreateEventA(NULL, FALSE, FALSE, NULL);
	if (start_evt == NULL) {
		CloseHandle(query_evt);
		CloseHandle(alloc_evt);
		return false;
	}
	auto& temp_name = caller.ObjectName = NULL;
	DWORD Tid;
	if (pNtQueryObjectPtr == NULL) {
		pNtQueryObjectPtr = (__NtQueryObject)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryObject");
	}
	HANDLE thd = CreateThread(NULL, 0, ObGetObjectNameThread, &caller, 0, &Tid);
	if (thd != NULL) {
		DWORD stat = WaitForSingleObject(start_evt, 5000);
		if (stat == WAIT_OBJECT_0) {
			stat = WaitForSingleObject(query_evt, 10);
			if (stat == WAIT_TIMEOUT) {
				TerminateThread(thd, ERROR_TIMEOUT);
			} else {
				SetEvent(alloc_evt);
				WaitForSingleObject(query_evt, INFINITE);
				if (temp_name != NULL) {
					obj_name = std::move(UNONE::StrToA(temp_name));
					free(temp_name);
					ret = true;
				}
			}
		}
		CloseHandle(thd);
	}
	CloseHandle(start_evt);
	CloseHandle(query_evt);
	CloseHandle(alloc_evt);
	return ret;
}