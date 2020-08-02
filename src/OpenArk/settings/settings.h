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
#include <QtCore>
#include <QtWidgets>
#include <Windows.h>
#include "ui_settings.h"

namespace Ui {
	class Settings;
}

class Settings : public QWidget {
	Q_OBJECT
public:
	Settings(QWidget *parent);
	~Settings();
protected:
	void closeEvent(QCloseEvent *e);
	void InitConsoleView();
	void InitCleanView();
	void InitGeneralView();

private:
	Ui::Settings ui;
	QStandardItemModel *console_model_;
};