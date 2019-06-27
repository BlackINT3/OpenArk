#pragma once
#include "../openark/openark.h"
#include <QString>

extern QApplication *app;
extern QTranslator *app_tr;
extern OpenArk *openark;

enum LogOuputLevel { LevelInfo, LevelWarn, LevelErr, LevelDbg };
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
#define QERR_A(format, ...)  \
	LogOutput(LevelErr, __FUNCTION__, (TRA(format)), __VA_ARGS__)
#define QERR_W(format, ...)  \
	LogOutput(LevelErr, __FUNCTION__, (TRW(format)), __VA_ARGS__)

inline QString AppVersion()
{
	std::wstring ver;
	UNONE::FsGetFileInfoW(UNONE::PsGetProcessPathW(), L"ProductVersion", ver);
	if (!ver.empty()) {
		ver = ver.substr(0, ver.find_last_of(L"."));
	}
	return WStrToQ(ver);
}

inline QString AppBuildTime()
{
	QString &&stamp = StrToQ(UNONE::TmFormatUnixTimeA(UNONE::PeGetTimeStamp((CHAR*)GetModuleHandleW(NULL)), "YMDHW"));
	return stamp;
}

// disable logger, exit recover
#define DISABLE_RECOVER() \
	UNONE::LogCallback routine;\
	bool regok = UNONE::InterCurrentLogger(routine);\
	if (regok) UNONE::InterRegisterLogger([&](const std::wstring &) {});\
	ON_SCOPE_EXIT([&] {if (regok) UNONE::InterUnregisterLogger(); });
