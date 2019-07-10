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
	Q_INVOKABLE void CmdHelp(QString cmd, QStringList argv);
	Q_INVOKABLE void CmdCls(QString cmd, QStringList argv);
	Q_INVOKABLE void CmdHistory(QString cmd, QStringList argv);
	Q_INVOKABLE void CmdTimeStamp(QString cmd, QStringList argv);
	Q_INVOKABLE void CmdErrorShow(QString cmd, QStringList argv);
	Q_INVOKABLE void CmdFormats(QString cmd, QStringList argv);
	Q_INVOKABLE void CmdExit(QString cmd, QStringList argv);
	Q_INVOKABLE void CmdRestart(QString cmd, QStringList argv);
	Q_INVOKABLE void CmdCmd(QString cmd, QStringList argv);
	Q_INVOKABLE void CmdStart(QString cmd, QStringList argv);
	Q_INVOKABLE void CmdMsg(QString cmd, QStringList argv);
	Q_INVOKABLE void CmdWndInfo(QString cmd, QStringList argv);
	Q_INVOKABLE void CmdProcessInfo(QString cmd, QStringList argv);
	Q_INVOKABLE void CmdProcessTree(QString cmd, QStringList argv);
	Q_INVOKABLE void CmdMemoryEditor(QString cmd, QStringList argv);
	Q_INVOKABLE void CmdFileEditor(QString cmd, QStringList argv);

	QString CmdGetLast();
	QString CmdGetNext();
	void CmdException(QString cmd, int type);
	void CmdErrorOutput(const std::wstring &err);
	void CmdOutput(const char* format, ...);
	void CmdOutput(const wchar_t* format, ...);
	void CmdDispatcher(const std::wstring &cmdline);

	size_t cmd_cursor_;
	QStringList cmd_history_;
	QTextBrowser *cmd_window_;

};