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
#include "../../common/cpp-wrapper/cpp-wrapper.h"
#include <QString>

#pragma once
#include <windows.h>
#include <vector>
#include "ui_kernel.h"
#include "../kernel.h"
#include "../common/common.h"

class Ui::Kernel;
class Kernel;

enum {
	TAB_KERNEL_DRIVER_LIST,
	TAB_KERNEL_DRIVER_KIT,
};

PROXY_FILTER(DriversSortFilterProxyModel);
class KernelDriver : public CommonTabObject {
	Q_OBJECT

public:
	enum {
		Region,
		View,
	};
	KernelDriver();
	~KernelDriver();
public:
	bool eventFilter(QObject *obj, QEvent *e);
	void ModuleInit(Ui::Kernel *mainui, Kernel *kernel);
	bool InstallDriver(QString driver, QString name);
	bool UninstallDriver(QString service);
	void ShowDrivers();

private slots:
	void onTabChanged(int index);
	void onSignDriver();
	void onInstallNormallyDriver();
	void onInstallUnsignedDriver();
	void onInstallExpiredDriver();
	void onUninstallDriver();

private:
	void InitDriversView();
	void InitDriverKitView();
	QString DriversItemData(int column);

private:
	Ui::Kernel *ui;
	Kernel *kernel_;
	QMenu *drivers_menu_;
	QStandardItemModel *drivers_model_;
	DriversSortFilterProxyModel *proxy_drivers_;
};

bool SignExpiredDriver(QString driver);
std::wstring ParseDriverPath(UCHAR *symlnk);

#define RECOVER_SIGN_TIME() \
		SYSTEMTIME saved = { 0 };\
		GetLocalTime(&saved);\
		auto ticks = GetTickCount();\
		SYSTEMTIME cur = { 0 };\
		cur.wYear = 2012;\
		cur.wMonth = 1;\
		cur.wDay = 1;\
		SetLocalTime(&cur);\
		ON_SCOPE_EXIT([&] {	saved.wSecond += (GetTickCount() - ticks)/1000; SetLocalTime(&saved); });