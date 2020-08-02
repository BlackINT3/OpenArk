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
#include "../common/ui-wrapper/ui-wrapper.h"

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
struct JunkCluster {
	QString dir;
	DWORD64 sumsize;
	QList<JunkItem> items;
};

class ScanJunksThread : public QThread {
	Q_OBJECT
signals:
	void appendJunks(JunkCluster);
protected:
	void run();
public:
	QList<JunkCluster> junks_cluster_;
	bool is_custom_scan_ = false;
	bool is_builtin_scan_ = false;
	QStringList custom_path_;
	QString custom_suffex_;
};

class CleanJunksThread : public QThread {
	Q_OBJECT
signals:
	void cleanJunks(JunkCluster);
public:
	void setJunkCluster(QList<JunkCluster> clusters) { junks_cluster_ = clusters; };
protected:
	void run();
private:
	QList<JunkCluster> junks_cluster_;
};


class Utilities : public CommonMainTabObject {
	Q_OBJECT
public:
	Utilities(QWidget *parent, int tabid);
	~Utilities();

public:
	void RecordAppServer(const QString &svr);

private slots:
	void onTabChanged(int index);
	void onOpJunkfiles(int, JunkCluster);
	void onAppendJunkfiles(JunkCluster);
	void onCleanJunkfiles(JunkCluster);

private:
	void InitCleanerView();
	void InitSystemToolsView();
	void RemoveCleanerItems();
	QVector<int> removed_rows_;

private:
	QString app_server_;
	QStandardItemModel *junks_model_;
	JunksSortFilterProxyModel *proxy_junks_;
	ScanJunksThread *scanjunks_thread_;
	CleanJunksThread *cleanjunks_thread_;
	Ui::Utilities ui;
	OpenArk *parent_;
};