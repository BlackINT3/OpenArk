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
#include "../common/common.h"

bool ReadConsoleOutput(const std::string& Cmdline, std::string& Output, DWORD& ExitCode, DWORD Timeout = INFINITE);

enum {
	ECMD_PARAM_INVALID,
	ECMD_NOSUCH_CMD,
};

class Cmds : public QTextBrowser {
	Q_OBJECT

public:
	Cmds (QTextBrowser *parent);
	~Cmds ();

public:
	Q_INVOKABLE void CmdHelp(QStringList argv);
	Q_INVOKABLE void CmdCls(QStringList argv);
	Q_INVOKABLE void CmdHistory(QStringList argv);
	Q_INVOKABLE void CmdTimeStamp(QStringList argv);
	Q_INVOKABLE void CmdErrorShow(QStringList argv);
	Q_INVOKABLE void CmdFormats(QStringList argv);
	Q_INVOKABLE void CmdExit(QStringList argv);
	Q_INVOKABLE void CmdRestart(QStringList argv);
	Q_INVOKABLE void CmdCmd(QStringList argv);
	Q_INVOKABLE void CmdStart(QStringList argv);
	Q_INVOKABLE void CmdMsg(QStringList argv);
	Q_INVOKABLE void CmdWndInfo(QStringList argv);
	Q_INVOKABLE void CmdProcessInfo(QStringList argv);
	Q_INVOKABLE void CmdProcessTree(QStringList argv);
	Q_INVOKABLE void CmdMemoryInfo(QStringList argv);

	QString CmdGetLast();
	QString CmdGetNext();
	void CmdException(int type);
	void CmdErrorOutput(const std::wstring &err);
	void CmdOutput(const char* format, ...);
	void CmdOutput(const wchar_t* format, ...);
	void CmdDispatcher(const std::wstring &cmdline);

	size_t cmd_cursor_;
	QStringList cmd_history_;
	QTextBrowser *cmd_window_;

};