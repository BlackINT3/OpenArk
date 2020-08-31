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
#include <windows.h>
#include <vector>
#include "ui_kernel.h"
#include "../kernel.h"
#include "../common/common.h"
#include "../common/ui-wrapper/ui-wrapper.h"

class Ui::Kernel;
class Kernel;

enum {
	//TAB_KERNEL_NETWORK_WFP,
	//TAB_KERNEL_NETWORK_TDI,
	TAB_KERNEL_NETWORK_PORT,
	TAB_KERNEL_NETWORK_HOSTS,
};

typedef struct _CALLOUT_INFO {
	ULONG CalloutId;
	GUID CalloutKey;
	CHAR ModuleName[MAX_PATH];
}CALLOUT_INFO;

bool EnumWfpCallouts(std::vector<CALLOUT_INFO>& CalloutIDs);

PROXY_FILTER(WfpSortFilterProxyModel);
PROXY_FILTER(PortSortFilterProxyModel);
class KernelNetwork : public CommonTabObject {
	Q_OBJECT

public:
	KernelNetwork();
	~KernelNetwork();
public:
	bool eventFilter(QObject *obj, QEvent *e);
	void ModuleInit(Ui::Kernel *ui, Kernel *kernel);

private slots:
	void onTabChanged(int index);
	void onShowPortInfo();

private:
	void InitWfpView();
	void InitHostsView();
	void InitPortView();
	void ShowWfpInfo();

private:
	Ui::Kernel *ui_;
	Kernel *kernel_;
	std::wstring hosts_dir_;
	std::wstring hosts_file_;
	QMenu *hosts_menu_;
	QMenu *port_menu_;
	QStandardItemModel *wfp_model_;
	QStandardItemModel *port_model_;
	WfpSortFilterProxyModel *proxy_wfp_;
	PortSortFilterProxyModel *proxy_port_;
};