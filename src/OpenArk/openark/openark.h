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
#include <QtWidgets/QMainWindow>
#include "ui_openark.h"
#include "common/config/config.h"

class Cmds;

class OpenArk : public QMainWindow {
	Q_OBJECT
public:
	OpenArk(QWidget *parent = Q_NULLPTR);

protected:
	bool eventFilter(QObject *obj, QEvent *e);
	void changeEvent(QEvent *e);

signals:
	void signalRefresh();
	void signalShowPtool(int);

public slots:
	void onLogOutput(QString log);
	void onExecCmd(const std::wstring &cmdline);
	void onOpen(QString path);

private slots:
	void onActionOpen(bool checked);
	void onActionRefresh(bool checked);
	void onActionReset(bool checked);
	void onActionOnTop(bool checked);
	void onActionAbout(bool checked);
	void onActionSettings(bool checked);
	void onActionConsole(bool checked);
	void onActionPtool(bool checked);
	void onActionManuals(bool checked);
	void onActionGithub(bool checked);
	void onActionCoderKit(bool checked);
	void onActionScanner(bool checked);
	void onActionBundler(bool checked);
	void onActionCheckUpdate(bool checked);
	void onActionLanguage(QAction *act);
	void onCmdHelp();
	void onShowConsoleMenu(const QPoint &pt);
	void onConsoleClear();
	void onConsoleHelps();
	void onConsoleHistory();
	void onCmdInput();
	void onTabChanged(int);

public:
	void StatusBarClear();
	void StatusBarAdd(QWidget *label);
	void SetActiveTab(int idx);

private:
	Cmds *cmds_;
	QSize old_window_size_;
	QPoint old_window_pos_;
	QTimer *chkupt_timer_;
	QToolBar *stool_;
	Ui::OpenArkWindow ui;
};
