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
#include "ui_kernel.h"

class OpenArk;

namespace Ui {
	class Kernel;
}
class DriversSortFilterProxyModel : public QSortFilterProxyModel {
	Q_OBJECT
public:
	DriversSortFilterProxyModel(QWidget *parent) {};
	~DriversSortFilterProxyModel() {};
protected:
	bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
};

class Kernel : public QWidget {
	Q_OBJECT
public:
	Kernel(QWidget *parent);
	~Kernel();

protected:
	bool eventFilter(QObject *obj, QEvent *e);
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);

signals:
	void signalOpen(QString);

private slots:
	void onTabChanged(int index);
	void onOpenFile(QString path);
	void onSignDriver();
	void onInstallNormallyDriver();
	void onInstallUnsignedDriver();
	void onInstallExpiredDriver();
	void onUninstallDriver();

private:
	void InitDriversView();
	void InitDriverKitView();
	bool InstallDriver(QString driver);
	bool UninstallDriver(QString service);
	void ShowDrivers();
	int DriversCurRow();
	QString DriversCurViewItemData(int column);

private:
	Ui::Kernel ui;
	OpenArk *parent_;
	QMenu *drivers_menu_;
	QStandardItemModel *drivers_model_;
	DriversSortFilterProxyModel *proxy_drivers_;
};