#include "../common.h"
#include "../openark/openark.h"
#include "app.h"

QApplication *app = nullptr;
QTranslator *app_tr = nullptr;
OpenArk *openark = nullptr;

bool LogOutput(LogOuputLevel lev, const char* func, const wchar_t* format, ...)
{
	QString levelstr;
	struct { int lev; QString levstr; } levels[] = {
		{ LevelInfo , TR("[INFO]") },
		{ LevelWarn , TR("<font color=red>[WARN]</font>") },
		{ LevelErr , TR("<font color=red>[ERR]</font>") },
		{ LevelDbg , TR("[DBG]") },
	};
	for (int i = 0; i < _countof(levels); i++) {
		if (levels[i].lev == lev) {
			levelstr = levels[i].levstr;
			break;
		}
	}
	std::wstring prefix = UNONE::StrFormatW(
		TR("<font color=black>[%s] %s %s </font>").toStdWString().c_str(),
		UNONE::StrToW(func).c_str(),
		levelstr.toStdWString().c_str(),
		format);
	std::wstring str;
	va_list lst;
	va_start(lst, format);
	str = UNONE::StrFormatVaListW(prefix.c_str(), lst);
	va_end(lst);

	openark->onLogOutput(QString::fromStdWString(str));

	return true;
}

bool LogOutput(LogOuputLevel lev, const char* func, const char* format, ...)
{
	va_list lst;
	va_start(lst, format);
	std::wstring&& wstr = UNONE::StrToW(UNONE::StrFormatVaListA(format, lst));
	LogOutput(lev, func, L"%s", wstr.c_str());
	va_end(lst);
	return true;
}
