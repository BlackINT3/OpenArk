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
#include <vector>
#include <map>
#include <string>
#include <QString>
#include "../common.h"
#include "reg-wrapper.h"

#define KB		(1024)
#define MB		(1024*KB)
#define GB		(1024*MB)
#define TB		(1024*GB)

bool RetrieveThreadTimes(DWORD tid, std::wstring& ct, std::wstring& kt, std::wstring& ut);
std::wstring FormatFileTime(FILETIME *file_tm);
std::wstring ProcessCreateTime(__in DWORD pid);
LONGLONG ProcessCreateTimeValue(__in DWORD pid);
bool CreateDump(DWORD pid, const std::wstring& path, bool mini);
void ClipboardCopyData(const std::string &data);
std::vector<HWND> GetSystemWnds();
int64_t FileTimeToInt64(FILETIME tm);
double GetSystemUsageOfCPU();
double GetSystemUsageOfMemory();
SIZE_T GetProcessPrivateWorkingSet(DWORD pid);
void SetWindowOnTop(HWND wnd, bool ontop);
void WinShowProperties(const std::wstring &path);
bool GetCertOwner(const QString &path, QString &owner);
bool ObGetObjectName(HANDLE hd, std::string& obj_name);
bool ExtractResource(const QString &res, const QString &path);
bool WriteFileDataW(__in const std::wstring& fpath, __in int64_t offset, __in const std::string& fdata);
bool ReadFileDataW(__in const std::wstring &fpath, __in int64_t offset, __in int64_t readsize, __out std::string &fdata);
bool ReadStdout(const std::wstring& cmdline, std::wstring& output, DWORD& exitcode, DWORD timeout = INFINITE);
DWORD PsGetPidByWindowW(wchar_t *cls, wchar_t *title);
DWORD OsGetExplorerPid();
HANDLE OpenProcessWrapper(DWORD access, BOOL inherit, DWORD pid);
HANDLE OpenThreadWrapper(DWORD access, BOOL inherit, DWORD tid);