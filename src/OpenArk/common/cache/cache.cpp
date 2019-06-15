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
#include "cache.h"
#include "../common.h"

static struct {
	QMutex lck;
	QMap<unsigned int, ProcInfo> d;
} proc_info;


ProcInfo CacheGetProcInfo(unsigned int pid, ProcInfo& info)
{
	QMutexLocker locker(&proc_info.lck);
	if (proc_info.d.contains(pid)) {
		auto it = proc_info.d.find(pid);
		info = it.value();
		return info;
	}
	static bool is_os64 = UNONE::OsIs64();
	info.pid = pid;
	if (info.ppid == -1) info.ppid = UNONE::PsGetParentPid(pid);
	auto &&path = UNONE::PsGetProcessPathW(pid);
	info.path = WStrToQ(path);
	std::wstring corp, desc;
	UNONE::FsGetFileInfoW(path, L"CompanyName", corp);
	UNONE::FsGetFileInfoW(path, L"FileDescription", desc);
	info.corp = WStrToQ(corp);
	info.desc = WStrToQ(desc);
	if (info.name.isEmpty()) info.name = WStrToQ(UNONE::FsPathToNameW(path));
	info.ctime = WStrToQ(ProcessCreateTime(pid));
	if (is_os64 && !UNONE::PsIsX64(pid))	info.name.append(" *32");
	proc_info.d.insert(pid, info);
	return info;
}

void CacheGetProcChilds(unsigned int pid, QVector<ProcInfo>& infos)
{
	if (pid == 0) {
		return;
	}
	QMutexLocker locker(&proc_info.lck);
	for (auto &info : proc_info.d) {
		if (info.ppid == pid) {
			infos.push_back(info);
		}
	}
}

void CacheRefreshProcInfo()
{
	QMutexLocker locker(&proc_info.lck);
	auto &d = proc_info.d;
	for (auto it = d.begin(); it != d.end(); ) {
		auto pid = it.key();
		if ((pid == INVALID_PID ) || (pid != 0 && UNONE::PsIsDeleted(pid))) {
			d.erase(it++);
		}	else {
			it++;
		}
	}
}

static struct {
	QMutex lck;
	QMap<unsigned int, UNONE::PROCESS_BASE_INFOW> d;
} proc_baseinfo;

UNONE::PROCESS_BASE_INFOW CacheGetProcessBaseInfo(DWORD pid)
{
	UNONE::PROCESS_BASE_INFOW info;
	QMutexLocker locker(&proc_baseinfo.lck);
	if (proc_baseinfo.d.contains(pid)) {
		auto it = proc_baseinfo.d.find(pid);
		return it.value();
	}
	UNONE::PsGetProcessInfoW(pid, info);
	proc_baseinfo.d.insert(pid, info);
	return info;
}

static struct {
	QMutex lck;
	QMap<QString, FileBaseInfo> d;
} filebase_info;

FileBaseInfo CacheGetFileBaseInfo(QString path)
{
	QMutexLocker locker(&filebase_info.lck);
	if (filebase_info.d.contains(path)) {
		auto it = filebase_info.d.find(path);
		return it.value();
	}
	std::wstring corp, desc, ver;
	auto w_path = path.toStdWString();
	UNONE::FsGetFileInfoW(w_path, L"CompanyName", corp);
	UNONE::FsGetFileInfoW(w_path, L"FileDescription", desc);
	UNONE::FsGetFileVersionW(w_path, ver);
	auto info = FileBaseInfo{ path, WStrToQ(desc), WStrToQ(ver),  WStrToQ(corp) };
	filebase_info.d.insert(path, info);
	return info;
}