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
#include <QString>
#include "../openark/openark.h"

extern OpenArk *app;

void MsgBoxInfo(QString msg);
void MsgBoxWarn(QString msg);
void MsgBoxError(QString msg);

enum LogOuputLevel {
	LevelInfo,
	LevelWarn,
	LevelErr,
	LevelDbg
};
bool LogOutput(LogOuputLevel lev, const char* func, const char* format, ...);
bool LogOutput(LogOuputLevel lev, const char* func, const wchar_t* format, ...);
#define INFO(format, ...)  \
	LogOutput(LevelInfo, __FUNCTION__, (format), __VA_ARGS__)
#define WARN(format, ...)  \
	LogOutput(LevelWarn, __FUNCTION__, (format), __VA_ARGS__)
#define ERR(format, ...)  \
	LogOutput(LevelErr, __FUNCTION__, (format), __VA_ARGS__)
#define DBG(format, ...)  \
	LogOutput(LevelDbg, __FUNCTION__, (format), __VA_ARGS__)

#include "win-wrapper/win-wrapper.h"
#include "qt-wrapper/qt-wrapper.h"
#include "cpp-wrapper/cpp-wrapper.h"


__inline QString AppVersion()
{
	std::wstring ver;
	UNONE::FsGetFileInfoW(UNONE::PsGetProcessPathW(), L"ProductVersion", ver);
	if (!ver.empty()) {
		ver = ver.substr(0, ver.find_last_of(L"."));
	}
	return WStrToQ(ver);
}

__inline QString AppBdTime()
{
	QString stamp = StrToQ(UNONE::TmFormatUnixTimeA(UNONE::PeGetTimeStamp((CHAR*)GetModuleHandleW(NULL)), "YMDHW"));
	return stamp;
}

// disable logger, exit recover
#define DISABLE_RECOVER() \
	UNONE::LogCallback routine;\
	bool regok = UNONE::InterCurrentLogger(routine);\
	if (regok) UNONE::InterRegisterLogger([&](const std::wstring &) {});\
	ON_SCOPE_EXIT([&] {if (regok) UNONE::InterUnregisterLogger(); });