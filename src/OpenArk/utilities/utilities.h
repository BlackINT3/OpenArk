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
#include "ui_utilities.h"

class OpenArk;
class Ui::Utilities;

class JunksSortFilterProxyModel : public QSortFilterProxyModel {
	Q_OBJECT
public:
	JunksSortFilterProxyModel(QWidget *parent) {};
	~JunksSortFilterProxyModel() {};
protected:
	bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
};

struct JunkItem {
	QString name;
	QString path;
	DWORD64 size;
};

class ScanJunksThread : public QThread {
	Q_OBJECT
signals:
	void appendJunks(QList<JunkItem>);
protected:
	void run();
};

class CleanJunksThread : public QThread {
	Q_OBJECT
signals:
	void cleanJunks(QList<JunkItem>);
public:
	void setJunks(QStringList junks) { junks_ = junks; };
protected:
	void run();
private:
	QStringList junks_;
};


class Utilities : public QWidget {
	Q_OBJECT
public:
	Utilities(QWidget *parent);
	~Utilities();

private slots:
	void onAppendJunkfiles(QList<JunkItem>);

private:
	void InitCleanerView();
	void InitSystemToolsView();

private:
	QStandardItemModel *junks_model_;
	JunksSortFilterProxyModel *proxy_junks_;
	ScanJunksThread *scanjunks_thread_;
	CleanJunksThread *cleanjunks_thread_;
	Ui::Utilities ui;
	OpenArk *parent_;
};