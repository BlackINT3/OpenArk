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
#include <arkdrv-api/arkdrv-api.h>

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
	HANDLE thd = OpenThreadWrapper(THREAD_QUERY_INFORMATION, FALSE, tid);
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
	HANDLE Process = OpenProcessWrapper(PROCESS_QUERY_INFORMATION, FALSE, pid);
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

LONGLONG ProcessCreateTimeValue(__in DWORD pid)
{
	HANDLE phd = OpenProcessWrapper(PROCESS_QUERY_INFORMATION, FALSE, pid);
	if (!phd) {
		return 0;
	}
	FILETIME create_tm;
	FILETIME exit_tm;
	FILETIME kern_tm;
	FILETIME user_tm;
	if (!GetProcessTimes(phd, &create_tm, &exit_tm, &kern_tm, &user_tm)) {
		CloseHandle(phd);
		return 0;
	}
	CloseHandle(phd);
	return UNONE::TmFileTimeToMs(create_tm);
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
	} else {
		dmp_type = (MINIDUMP_TYPE)(MiniDumpWithThreadInfo | MiniDumpWithFullMemoryInfo | MiniDumpWithTokenInformation |
			MiniDumpWithProcessThreadData | MiniDumpWithDataSegs | MiniDumpWithFullMemory | MiniDumpWithHandleData);
	}
	HANDLE phd = OpenProcessWrapper(PROCESS_ALL_ACCESS, FALSE, pid);
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
	HANDLE phd = OpenProcessWrapper(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
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

bool ExtractResource(const QString &res, const QString &path)
{
	QFile f(res);
	if (!f.open(QIODevice::ReadOnly)) {
		return false;
	}
	auto &&data = f.readAll().toStdString();
	auto &&dst = path.toStdWString();
	UNONE::FsCreateDirW(UNONE::FsPathToDirW(dst));
	if (!UNONE::FsWriteFileDataW(dst, data)) {
		return false;
	}
	return true;
}

bool WriteFileDataW(__in const std::wstring& fpath, __in int64_t offset, __in const std::string& fdata)
{
	bool result = false;
	bool read_only = false;
	DWORD saved_attr = GetFileAttributesW(fpath.c_str());
	if (saved_attr != INVALID_FILE_ATTRIBUTES) {
		if (saved_attr & FILE_ATTRIBUTE_READONLY) {
			read_only = true;
			SetFileAttributesW(fpath.c_str(), saved_attr&(~FILE_ATTRIBUTE_READONLY));
		}
	}
	HANDLE fd = CreateFileW(fpath.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fd != INVALID_HANDLE_VALUE) {
		DWORD writelen;
		LARGE_INTEGER li;
		li.QuadPart = offset;
		SetFilePointer(fd, li.LowPart, &li.HighPart, FILE_BEGIN);
		if (WriteFile(fd, fdata.data(), (DWORD)fdata.size(), &writelen, NULL)) {
			if (fdata.size() == writelen) {
				result = true;
			} else {
				ERR(L"WriteFile %s err, expected-size:%d actual-size:%d", fpath.c_str(), fdata.size(), writelen);
			}
		} else {
			ERR(L"WriteFile %s err:%d", fpath.c_str(), GetLastError());
		}
		CloseHandle(fd);
	} else {
		ERR(L"CreateFileW %s err:%d", fpath.c_str(), GetLastError());
	}
	if (read_only)
		SetFileAttributesW(fpath.c_str(), saved_attr);
	return result;
}

bool ReadFileDataW(__in const std::wstring &fpath, __in int64_t offset, __in int64_t readsize, __out std::string &fdata)
{
	bool result = false;
	DWORD fsize = 0;
	HANDLE fd = CreateFileW(fpath.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (fd == INVALID_HANDLE_VALUE) {
		ERR(L"CreateFileW %s err:%d", fpath.c_str(), GetLastError());
		return false;
	}
	fsize = GetFileSize(fd, NULL);
	if (fsize == INVALID_FILE_SIZE) {
		ERR(L"GetFileSize %s err:%d", fpath.c_str(), GetLastError());
		CloseHandle(fd);
		return false;
	}
	if ((offset+readsize) > fsize) {
		WARN(L"read offset out of bound");
		readsize = fsize - offset;
	}
	LARGE_INTEGER li;
	li.QuadPart = offset;
	SetFilePointer(fd, li.LowPart, &li.HighPart, FILE_BEGIN);
	char* buff = new(std::nothrow) char[readsize];
	if (buff == NULL) {
		ERR(L"alloc memory err");
		CloseHandle(fd);
		return false;
	}
	DWORD readlen;
	if (ReadFile(fd, buff, readsize, &readlen, NULL)) {
		if (readlen == readsize) {
			try {
				fdata.assign(buff, readsize);
				result = true;
			} catch (std::exception& e) {
				fdata.clear();
				ERR("c++ exception: %s", e.what());
			} catch (...) {
				fdata.clear();
				ERR("c++ exception: unknown");
			}
		} else {
			ERR(L"ReadFile %s err, expected-size:%d actual-size:%d", fpath.c_str(), readsize, readlen);
		}
	} else {
		ERR(L"ReadFile %s err:%d", fpath.c_str(), GetLastError());
	}
	delete[] buff;
	CloseHandle(fd);
	return result;
} 

bool ReadStdout(const std::wstring& cmdline, std::wstring& output, DWORD& exitcode, DWORD timeout /*= INFINITE*/)
{
	const int blksize = 512;
	SECURITY_ATTRIBUTES sa;
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;
	sa.nLength = sizeof(sa);
	HANDLE rstdin = NULL;
	HANDLE wstdout = NULL;
	BOOL ret = CreatePipe(&rstdin, &wstdout, &sa, 64 * 0x1000);
	if (!ret) {
		ERR("CreatePipe failed, err:%d", GetLastError());
		return false;
	}

	STARTUPINFOW si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	GetStartupInfoW(&si);
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow = SW_HIDE;
	si.hStdOutput = wstdout;
	si.hStdError = wstdout;
	ret = CreateProcessW(NULL, (LPWSTR)cmdline.c_str(), NULL, NULL,
		TRUE, 0, NULL, NULL, &si, &pi);
	CloseHandle(wstdout);
	if (!ret) {
		CloseHandle(rstdin);
		ERR(L"CreateProcessW %s failed, err:%d", cmdline.c_str(), GetLastError());
		return false;
	}
	if (WaitForSingleObject(pi.hProcess, timeout) == WAIT_TIMEOUT) {
		TerminateProcess(pi.hProcess, 1);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		CloseHandle(rstdin);
		return false;
	}
	GetExitCodeProcess(pi.hProcess, &exitcode);

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	bool result = false;
	char *buf = new(std::nothrow) char[blksize];
	if (buf == nullptr) {
		CloseHandle(rstdin);
		return false;
	}
	while (1) {
		DWORD readlen = 0;
		if (ReadFile(rstdin, buf, blksize, &readlen, NULL)) {
			output.append(UNONE::StrToW(std::string(buf, readlen)));
			if (blksize > readlen) {
				result = true;
				break;
			}
		}	else {
			if (GetLastError() == ERROR_BROKEN_PIPE) {
				result = true;
				break;
			}
		}
	}
	CloseHandle(rstdin);
	delete[] buf;
	return result;
}

DWORD PsGetPidByWindowW(wchar_t *cls, wchar_t *title)
{
	DWORD pid = INVALID_PID;
	HWND wnd = FindWindowW(cls, title);
	if (wnd != NULL) {
		GetWindowThreadProcessId(wnd, &pid);
	}
	return pid;
}
DWORD OsGetExplorerPid()
{
	return PsGetPidByWindowW(L"Progman", L"Program Manager");
}


HANDLE OpenProcessWrapper(DWORD access, BOOL inherit, DWORD pid)
{
	HANDLE phd = OpenProcess(access, inherit, pid);
	if (!phd && GetLastError()==ERROR_ACCESS_DENIED) {
		phd = ArkDrvApi::Process::OpenProcess(access, inherit, pid);
		if (!phd) return 0;
	}
	return phd;
}

HANDLE OpenThreadWrapper(DWORD access, BOOL inherit, DWORD tid)
{
	HANDLE phd = OpenThread(access, inherit, tid);
	if (!phd && GetLastError() == ERROR_ACCESS_DENIED) {
		phd = ArkDrvApi::Process::OpenThread(access, inherit, tid);
		if (!phd) return 0;
	}
	return phd;
}