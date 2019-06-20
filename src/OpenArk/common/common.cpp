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
#include "common.h"
#include "../openark/openark.h"
#include <QMessageBox>

QApplication *app = nullptr;
QTranslator *app_tr = nullptr;
OpenArk *openark = nullptr;

void MsgBoxInfo(QString msg)
{
	QMessageBox::information(nullptr, "OpenArk Info", msg);
}
void MsgBoxWarn(QString msg)
{
	QMessageBox::warning(nullptr, "OpenArk Warn", msg);
}
void MsgBoxError(QString msg)
{
	QMessageBox::critical(nullptr, "OpenArk Error", msg);
}

bool LogOutput(LogOuputLevel lev, const char* func, const wchar_t* format, ...)
{
	std::wstring levelstr;
	struct { int lev; wchar_t* levstr; } levels[] = {
		{ LevelInfo , L"[INFO]" },
		{ LevelWarn , L"<font color=red>[WARN]</font>" },
		{ LevelErr , L"<font color=red>[ERR]</font>" },
		{ LevelDbg , L"[DBG]" },
	};
	for (int i = 0; i < _countof(levels); i++) {
		if (levels[i].lev == lev) {
			levelstr = levels[i].levstr;
			break;
		}
	}
	std::wstring prefix = UNONE::StrFormatW(L"<font color=black>[%s] %s %s </font>", UNONE::StrToW(func).c_str(), levelstr.c_str(), format);
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
