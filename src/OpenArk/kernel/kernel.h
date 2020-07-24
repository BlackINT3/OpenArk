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
#include "../common/qt-wrapper/qt-wrapper.h"
#include "network/network.h"
#include "storage/storage.h"

class KernelNetwork;
class KernelStorage;

class OpenArk;
class Ui::Kernel;
PROXY_FILTER(DriversSortFilterProxyModel);
PROXY_FILTER(NotifySortFilterProxyModel);
PROXY_FILTER(HotkeySortFilterProxyModel);

class Kernel : public QWidget {
	Q_OBJECT
public:
	Kernel(QWidget *parent);
	~Kernel();

public:
	Q_INVOKABLE QTabWidget *GetActiveTabWidget() { return ui.tabWidget; };
	Q_INVOKABLE int GetActiveTab() { return ui.tabWidget->currentIndex(); };
	Q_INVOKABLE void SetActiveTab(QVector<int> idx) { 
		ui.tabWidget->setCurrentIndex(idx[0]); 
		qobject_cast<QTabWidget *>(ui.tabWidget->currentWidget())->setCurrentIndex(idx[1]);
	};

protected:
	bool eventFilter(QObject *obj, QEvent *e);
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);

signals:
	void signalOpen(QString);

private slots:
	void onTabChanged(int index);
	void onClickKernelMode();
	void onRefreshKernelMode();
	void onOpenFile(QString path);
	void onSignDriver();
	void onInstallNormallyDriver();
	void onInstallUnsignedDriver();
	void onInstallExpiredDriver();
	void onUninstallDriver();

private:
	void InitKernelEntryView();
	void InitDriversView();
	void InitDriverKitView();
	void InitNotifyView();
	void InitHotkeyView();
	void InitMemoryView();
	bool InstallDriver(QString driver, QString name);
	bool UninstallDriver(QString service);
	void ShowDrivers();
	void ShowSystemNotify();
	void ShowSystemHotkey();
	void ShowDumpMemory(ULONG64 addr, ULONG size);
	int DriversCurRow();
	QString DriversItemData(int column);
	QString NotifyItemData(int column);
	QString HotkeyItemData(int column);

private:
	bool arkdrv_conn_;
	KernelNetwork *network_;
	KernelStorage *storage_;

private:
	Ui::Kernel ui;
	OpenArk *parent_;
	QMenu *drivers_menu_;
	QMenu *notify_menu_;
	QMenu *hotkey_menu_;
	QStandardItemModel *kerninfo_model_;
	QStandardItemModel *drivers_model_;
	QStandardItemModel *notify_model_;
	QStandardItemModel *hotkey_model_;
	DriversSortFilterProxyModel *proxy_drivers_;
	NotifySortFilterProxyModel *proxy_notify_;
	HotkeySortFilterProxyModel *proxy_hotkey_;
};