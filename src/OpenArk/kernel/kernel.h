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
#include "../common/ui-wrapper/ui-wrapper.h"
#include "network/network.h"
#include "storage/storage.h"
#include "memory/memory.h"
#include "driver/driver.h"

enum {
	TAB_KERNEL_ENTRY,
	TAB_KERNEL_DRIVER,
	TAB_KERNEL_NOTIFY,
	TAB_KERNEL_HOTKEY,
	TAB_KERNEL_OBJECT,
	TAB_KERNEL_MEMORY,
	TAB_KERNEL_STORAGE,
	TAB_KERNEL_NETWORK,
	TAB_KERNEL_MAX,
};

class KernelNetwork;
class KernelStorage;
class KernelMemory;
class KernelDriver;

class OpenArk;
class Ui::Kernel;
PROXY_FILTER(NotifySortFilterProxyModel);
PROXY_FILTER(HotkeySortFilterProxyModel);

class Kernel : public CommonMainTabObject {
	Q_OBJECT
public:
	Kernel(QWidget *parent, int tabid);
	~Kernel();
	OpenArk *GetParent() const { return parent_; };

protected:
	bool eventFilter(QObject *obj, QEvent *e);
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);

private slots:
	void onTabChanged(int index);
	void onClickKernelMode();
	void onRefreshKernelMode();

public slots:
	void onOpenFile(QString path);

signals:
	void signalOpen(QString);

private:
	void InitKernelEntryView();
	void InitNotifyView();
	void InitHotkeyView();
	void ShowSystemNotify();
	void ShowSystemHotkey();
	QString NotifyItemData(int column);
	QString HotkeyItemData(int column);

private:
	bool arkdrv_conn_;
	KernelNetwork *network_;
	KernelStorage *storage_;
	KernelMemory *memory_;
	KernelDriver *driver_;

	Ui::Kernel ui;
	OpenArk *parent_;
	QMenu *notify_menu_;
	QMenu *hotkey_menu_;
	QStandardItemModel *kerninfo_model_;
	QStandardItemModel *notify_model_;
	QStandardItemModel *hotkey_model_;
	NotifySortFilterProxyModel *proxy_notify_;
	HotkeySortFilterProxyModel *proxy_hotkey_;
};