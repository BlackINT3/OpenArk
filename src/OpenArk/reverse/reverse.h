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
#include "ui_reverse.h"

#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QFile>
#include <QDebug>
#include <QProgressBar>
#include "../common/ui-wrapper/ui-wrapper.h"

class OpenArk;
class Ui::Reverse;

class Reverse : public CommonMainTabObject {
	Q_OBJECT
public:
	Reverse(QWidget *parent, int tabid);
	~Reverse();

private slots:
	void onTabChanged(int index);
	void onExecute();
	void readContent();
	void onProgress(qint64 bytesSent, qint64 bytesTotal);

private:
	void DownloadAndExecuteFile(int type, QString path, QString exe, QString url);
	void InitWinReverseToolsView();

private:
	Ui::Reverse ui;
	OpenArk *parent_;
	QNetworkReply *reply;
	QFile *file;
};