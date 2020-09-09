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
#include <WinSock2.h>
#include <arkdrv-api/arkdrv-api.h>
#include "network.h"

/*
//https://social.msdn.microsoft.com/Forums/windowsdesktop/en-US/8fd93a3d-a794-4233-9ff7-09b89eed6b1f/compiling-with-wfp?forum=wfp
#include "include/fwpmu.h"
#pragma comment(lib, "fwpuclnt.lib")
#pragma comment(lib, "Rpcrt4.lib")


BOOLEAN GuidEqual(_In_ const GUID* pGUIDAlpha, _In_ const GUID* pGUIDOmega)
{
	RPC_STATUS status = RPC_S_OK;
	UINT32     areEqual = FALSE;

	do
	{
		if (pGUIDAlpha == 0 ||
			pGUIDOmega == 0)
		{
			if ((pGUIDAlpha == 0 &&
				pGUIDOmega) ||
				(pGUIDAlpha &&
					pGUIDOmega == 0))
				break;
		}

		if (pGUIDAlpha == 0 &&
			pGUIDOmega == 0)
		{
			areEqual = TRUE;
			break;
		}

		areEqual = UuidEqual((UUID*)pGUIDAlpha,
			(UUID*)pGUIDOmega,
			&status);

	} while (false);

	return (BOOLEAN)areEqual;
}

bool EnumWfpCallouts(std::vector<CALLOUT_INFO>& CalloutIDs)
{
	bool Result = false;
	HANDLE EngineHandle = NULL;
	UINT32       status = NO_ERROR;
	FWPM_SESSION session = { 0 };
	HANDLE EnumHandle = NULL;
	FWPM_CALLOUT_ENUM_TEMPLATE* pCalloutEnumTemplate = NULL;
	session.displayData.name = L"WFPSampler's User Mode Session";
	session.flags = 0;

	do
	{
		status = FwpmEngineOpen0(0,
			RPC_C_AUTHN_WINNT,
			0,
			&session,
			&EngineHandle);
		if (status != NO_ERROR) {
			break;
		}

		status = FwpmCalloutCreateEnumHandle(EngineHandle,
			pCalloutEnumTemplate,
			&EnumHandle);
		if (status != NO_ERROR) {
			break;
		}

		UINT32 NumEntries = 0;
		FWPM_CALLOUT** ppCallouts = 0;
		status = FwpmCalloutEnum0(EngineHandle,
			EnumHandle,
			0xFFFFFFFF,
			&ppCallouts,
			&NumEntries);
		if (status != NO_ERROR) {
			break;
		}

		if (ppCallouts)
		{
			for (DWORD Index = 0; Index < NumEntries; Index++)
			{
				CALLOUT_INFO CalloutInfo;
				CalloutInfo.CalloutId = ppCallouts[Index]->calloutId;
				RtlCopyMemory(&CalloutInfo.CalloutKey, &ppCallouts[Index]->calloutKey, sizeof(GUID));
				CalloutIDs.push_back(CalloutInfo);
			}
			Result = true;
		}

	} while (false);

	if (EnumHandle) {
		FwpmCalloutDestroyEnumHandle(EngineHandle, EnumHandle);
	}

	if (EngineHandle) {
		status = FwpmEngineClose(EngineHandle);
	}

	if (pCalloutEnumTemplate) {
		delete pCalloutEnumTemplate;
		pCalloutEnumTemplate = NULL;
	}

	return Result;
}

UINT64 GetFilterIDByCalloutKey(const GUID* CalloutKey)
{
	UINT64 Result = 0;
	HANDLE EngineHandle = NULL;
	LONG  Status = NO_ERROR;
	FWPM_SESSION Session = { 0 };
	HANDLE EnumHandle = 0;
	FWPM_FILTER_ENUM_TEMPLATE* pFilterEnumTemplate = NULL;
	Session.displayData.name = L"WFPSampler's User Mode Session";
	Session.flags = 0;

	do
	{
		Status = FwpmEngineOpen(0,
			RPC_C_AUTHN_WINNT,
			0,
			&Session,
			&EngineHandle);
		if (Status != NO_ERROR)
		{
			break;
		}

		UINT32                               NumEntries = 0;
		FWPM_FILTER**                        ppFilters = 0;
		Status = FwpmFilterCreateEnumHandle0(EngineHandle,
			pFilterEnumTemplate,
			&EnumHandle);
		if (Status != NO_ERROR)
		{
			break;
		}

		Status = FwpmFilterEnum0(EngineHandle,
			EnumHandle,
			0xFFFFFFFF,
			&ppFilters,
			&NumEntries);
		if (Status != NO_ERROR)
		{
			break;
		}

		if (Status == NO_ERROR && ppFilters && NumEntries)
		{
			for (UINT32 Index = 0; Index < NumEntries; Index++)
			{
				if (GuidEqual(&ppFilters[Index]->action.calloutKey,
					CalloutKey))
				{
					Result = ppFilters[Index]->filterId;
					break;
				}
			}
		}

	} while (false);

	if (EngineHandle) {
		Status = FwpmEngineClose(EngineHandle);
	}

	if (pFilterEnumTemplate) {
		delete pFilterEnumTemplate;
		pFilterEnumTemplate = NULL;
	}

	return Result;
}

bool DeleteFilterById(UINT64 FilterId)
{
	bool Result = false;
	HANDLE EngineHandle = NULL;
	LONG       Status = NO_ERROR;
	FWPM_SESSION Session = { 0 };
	Session.displayData.name = L"WFPSampler's User Mode Session";
	Session.flags = 0;

	do
	{
		Status = FwpmEngineOpen(0,
			RPC_C_AUTHN_WINNT,
			0,
			&Session,
			&EngineHandle);
		if (Status != NO_ERROR) {
			break;
		}

		Status = FwpmFilterDeleteById(EngineHandle, FilterId);
		if (Status != NO_ERROR) {
			break;
		}

		Result = true;
	} while (false);

	if (EngineHandle) {
		Status = FwpmEngineClose(EngineHandle);
	}

	return Result;
}
*/

bool WfpSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const {
	auto s1 = sourceModel()->data(left); auto s2 = sourceModel()->data(right);
	return QString::compare(s1.toString(), s2.toString(), Qt::CaseInsensitive) < 0;
}

#define QVariantHex(s1) s1.toString().toULongLong(nullptr, 16)
#define QVariantStrcmp(s1, s2)  QString::compare(s1.toString(), s2.toString(), Qt::CaseInsensitive)
bool PortSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const {
	auto s1 = sourceModel()->data(left); auto s2 = sourceModel()->data(right);
	auto column = left.column();
	if ((column == 1 || column == 2)) {
		auto list1 = s1.toString().split(":");
		auto list2 = s2.toString().split(":");
		auto ip1 = list1[0]; auto ip2 = list2[0];
		if (ip1 != ip2) return ip1 < ip2;
		return QHexToDWord(list1[1]) < QHexToDWord(list2[1]);
	}
	if ((column == 4)) return QVariantHex(s1) < QVariantHex(s2);
	return QVariantStrcmp(s1, s2) < 0;
}

KernelNetwork::KernelNetwork()
{

}

KernelNetwork::~KernelNetwork()
{

}

void KernelNetwork::onTabChanged(int index)
{
	switch (index) {
	//case TAB_KERNEL_NETWORK_WFP: ShowWfpInfo(); break;
	case TAB_KERNEL_NETWORK_PORT: onShowPortInfo(); break;
	default: break;
	}
	CommonTabObject::onTabChanged(index);
}

bool KernelNetwork::eventFilter(QObject *obj, QEvent *e)
{
	if (e->type() == QEvent::ContextMenu) {
		QMenu *menu = nullptr;
		if (obj == ui_->hostsFileListWidget) menu = hosts_menu_;
		if (obj == ui_->portView) menu = port_menu_;
		QContextMenuEvent *ctxevt = dynamic_cast<QContextMenuEvent*>(e);
		if (ctxevt && menu) {
			menu->move(ctxevt->globalPos());
			menu->show();
		}

	}
	if (e->type() == QEvent::KeyPress) {
		QKeyEvent *keyevt = dynamic_cast<QKeyEvent*>(e);
		if (keyevt->matches(QKeySequence::Delete)) {
			for (auto &action : hosts_menu_->actions()) {
				if (action->text() == tr("Delete")) emit action->trigger();
			}
		}
		if (keyevt->matches(QKeySequence::Refresh)) {
			onShowPortInfo();
		}
	}
	return QWidget::eventFilter(obj, e);
}

void KernelNetwork::ModuleInit(Ui::Kernel *ui, Kernel *kernel)
{
	this->ui_ = ui;
	this->kernel_ = kernel;
	Init(ui->tabNetwork, TAB_KERNEL, TAB_KERNEL_NETWORK);

	//InitWfpView();

	InitHostsView();
	InitPortView();

	//onTabChanged(ui_->tabNetwork->currentIndex());
}

void KernelNetwork::InitWfpView()
{
	/*
	wfp_model_ = new QStandardItemModel;
	QTreeView *view = ui_->wfpView;
	proxy_wfp_ = new WfpSortFilterProxyModel(view);
	proxy_wfp_->setSourceModel(wfp_model_);
	proxy_wfp_->setDynamicSortFilter(true);
	proxy_wfp_->setFilterKeyColumn(1);
	view->setModel(proxy_wfp_);
	view->selectionModel()->setModel(proxy_wfp_);
	view->header()->setSortIndicator(-1, Qt::AscendingOrder);
	view->setSortingEnabled(true);
	view->viewport()->installEventFilter(kernel_);
	view->installEventFilter(kernel_);
	std::pair<int, QString> colum_layout[] = {
		{ 130, tr("ID") },
		{ 100, tr("Key") },
		{ 200, tr("Name") },
	};
	QStringList name_list;
	for (auto p : colum_layout) {
		name_list << p.second;
	}
	wfp_model_->setHorizontalHeaderLabels(name_list);
	for (int i = 0; i < _countof(colum_layout); i++) {
		view->setColumnWidth(i, colum_layout[i].first);
	}
	view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	*/
}

void KernelNetwork::InitHostsView()
{
	hosts_dir_ = UNONE::OsSystem32DirW() + L"\\drivers\\etc";
	hosts_file_ = hosts_dir_ + L"\\hosts";

	auto GetCurrentHostsName = [=]()->std::wstring {
		std::wstring hosts;
		auto cur = ui_->hostsFileListWidget->currentItem();
		if (cur) {
			hosts = cur->text().toStdWString();
		}
		return std::move(hosts);
	};

	auto GetCurrentHostsPath = [=]()->std::wstring {
		std::wstring hosts = GetCurrentHostsName();
		if (!hosts.empty()) hosts = hosts_dir_ + L"\\" + hosts;
		return std::move(hosts);
	};

	auto RefreshHostsData = [=]() {
		std::string data;
		auto &&hosts = GetCurrentHostsPath();
		UNONE::FsReadFileDataW(hosts, data);
		ui_->hostsDataEdit->setText(StrToQ(data));
	};

	auto WriteHostsData = [=](std::wstring path = L"") {
		std::string data = ui_->hostsDataEdit->toPlainText().toStdString();
		std::wstring hosts;
		if (path.empty()) hosts = GetCurrentHostsPath();
		else hosts = path;
		UNONE::StrReplaceA(data, "\n", "\r\n");
		UNONE::FsWriteFileDataW(hosts, data);
	};

	auto RefreshHostsList = [=]() {
		auto row = ui_->hostsFileListWidget->currentRow();
		ui_->hostsFileListWidget->clear();
		std::vector<std::wstring> names;
		UNONE::DirEnumCallbackW fcb = [&](wchar_t* path, wchar_t* name, void* param)->bool {
			if (UNONE::FsIsDirW(path)) return true;
			size_t yy=UNONE::StrIndexIW(std::wstring(name), std::wstring(L"hosts"));
			if (UNONE::StrIndexIW(std::wstring(name), std::wstring(L"hosts")) != 0) return true;
			names.push_back(name);
			return true;
		};
		UNONE::FsEnumDirectoryW(hosts_dir_, fcb);
		for (auto &n : names) {
			ui_->hostsFileListWidget->addItem(WStrToQ(n));
		}
		ui_->hostsFileListWidget->setCurrentRow(row);
	};

	connect(ui_->hostsFileListWidget, &QListWidget::itemSelectionChanged, [=] {
		RefreshHostsData();
	});

	connect(ui_->hostsRefreshBtn, &QPushButton::clicked, [=] {
		RefreshHostsData();
		RefreshHostsList();
	});

	connect(ui_->hostsSaveBtn, &QPushButton::clicked, [=] {
		WriteHostsData();
	});

	connect(ui_->hostsBackupBtn, &QPushButton::clicked, [=] {
		bool ok;
		SYSTEMTIME systime;
		GetSystemTime(&systime);
		QString def = WStrToQ(UNONE::TmFormatSystemTimeW(systime, L"YMD-HWS"));
		QString text = QInputDialog::getText(this, tr("Hosts Backup"), tr("Please input file name: (hosts-***)"), QLineEdit::Normal, def, &ok);
		if (ok && !text.isEmpty()) {
			auto &&hosts = hosts_dir_ + L"\\hosts-" + text.toStdWString();
			WriteHostsData(hosts);
			RefreshHostsList();
		}
	});

	connect(ui_->hostsClearBtn, &QPushButton::clicked, [=] {
		ui_->hostsDataEdit->clear();
	});

	connect(ui_->hostsDirBtn, &QPushButton::clicked, [&] {
		ShellRun(WStrToQ(hosts_dir_), "");
	});

	if (!UNONE::FsIsExistedW(hosts_file_)) UNONE::FsWriteFileDataW(hosts_file_, "# 127.0.0.1 localhost\n# ::1 localhost");		
	RefreshHostsList();
	ui_->hostsFileListWidget->setCurrentRow(0);

	ui_->hostsFileListWidget->installEventFilter(this);
	hosts_menu_ = new QMenu();
	hosts_menu_->addAction(tr("Mark as Main"), kernel_, [=] {
		WriteHostsData(hosts_file_);
		RefreshHostsList();
		ui_->hostsFileListWidget->setCurrentRow(0);
	});
	hosts_menu_->addAction(tr("Rename"), kernel_, [=] {
		bool ok;
		std::wstring &&old = GetCurrentHostsPath();
		auto && name = UNONE::FsPathToNameW(old);
		UNONE::StrReplaceIW(name, L"hosts-");
		QString text = QInputDialog::getText(this, tr("Hosts Rename"), tr("Please input file name: (hosts-***)"), QLineEdit::Normal, WStrToQ(name), &ok);
		if (ok) {
			DeleteFileW(old.c_str());
			std::wstring hosts;
			if (!text.isEmpty()) {
				hosts = hosts_dir_ + L"\\hosts-" + text.toStdWString();
			} else {
				hosts = hosts_dir_ + L"\\hosts";
			}
			WriteHostsData(hosts);
			RefreshHostsList();
		}
	});
	hosts_menu_->addAction(tr("Backup"), kernel_, [=] {
		emit ui_->hostsBackupBtn->click();
	});
	auto copy_menu = new QMenu();
	copy_menu->addAction(tr("File Name"))->setData(0);
	copy_menu->addAction(tr("File Path"))->setData(1);
	copy_menu->setTitle(tr("Copy"));
	connect(copy_menu, &QMenu::triggered, [=](QAction* action) {
		auto idx = action->data().toInt();
		std::wstring data;
		switch (idx) {
		case 0: data = GetCurrentHostsName(); break;
		case 1: data = GetCurrentHostsPath(); break;
		}
		ClipboardCopyData(UNONE::StrToA(data));
	});
	hosts_menu_->addAction(tr("Refresh"), kernel_, [=] {
		emit ui_->hostsRefreshBtn->click();
	});

	hosts_menu_->addAction(copy_menu->menuAction());
	hosts_menu_->addSeparator();
	hosts_menu_->addAction(tr("Delete"), kernel_, [=] {
		DeleteFileW(GetCurrentHostsPath().c_str());
		emit ui_->hostsRefreshBtn->click();
	}, QKeySequence::Delete);
	hosts_menu_->addAction(tr("Delete Non-Main"), kernel_, [=] {
		if (QMessageBox::warning(this, tr("Warning"), tr("Are you sure to delete all hosts file(include backups)?"),
			QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
			return;
		}
		for (int i = 0; i < ui_->hostsFileListWidget->count(); i++) {
			auto name = ui_->hostsFileListWidget->item(i)->text();
			if (!name.compare("hosts", Qt::CaseInsensitive)) continue;
			auto path = hosts_dir_ + L"\\" + QToWStr(name);
			DeleteFileW(path.c_str());
		}
		emit ui_->hostsRefreshBtn->click();
	});
}

void KernelNetwork::InitPortView()
{
	QTreeView *view = ui_->portView;
	port_model_ = new QStandardItemModel;
	proxy_port_ = new PortSortFilterProxyModel(view);
	std::vector<std::pair<int, QString>> layout = {
		{ 50, tr("Protocol") },
		{ 135, tr("Local address") },
		{ 145, tr("Foreign address") },
		{ 100, tr("State") },
		{ 50, tr("PID") },
		{ 530, tr("Process Path") },
	};

	SetDefaultTreeViewStyle(view, port_model_, proxy_port_, layout);
	view->viewport()->installEventFilter(this);
	view->installEventFilter(this);

	port_menu_ = new QMenu();
	port_menu_->addAction(tr("Refresh"), this, [&] {
		onShowPortInfo();
	}, QKeySequence::Refresh);
	port_menu_->addAction(tr("Copy"), this, [&] {
		auto view = ui_->portView;
		ClipboardCopyData(GetCurItemViewData(view, GetCurViewColumn(view)).toStdString());
	});
	port_menu_->addSeparator();
	port_menu_->addAction(tr("Kill Process"), this, [&] {
		auto pid = GetCurItemViewData(ui_->portView, 4).toInt();
		PsKillProcess(pid);
		onShowPortInfo();
	});
	port_menu_->addSeparator();
	port_menu_->addAction(tr("Sendto Scanner"), this, [&] {
		kernel_->GetParent()->SetActiveTab(TAB_SCANNER);
		emit kernel_->signalOpen(GetCurItemViewData(ui_->portView, 5));
	});
	port_menu_->addAction(tr("Explore File"), this, [&] {
		ExploreFile(GetCurItemViewData(ui_->portView, 5));
	});
	port_menu_->addAction(tr("Properties..."), this, [&]() {
		WinShowProperties(GetCurItemViewData(ui_->portView, 5).toStdWString());
	});
	connect(ui_->locaIPv4Btn, &QPushButton::clicked, [] {ShellRun("cmd.exe", "/k ipconfig|findstr /i ipv4"); });
	connect(ui_->locaIPv6Btn, &QPushButton::clicked, [] {ShellRun("cmd.exe", "/k ipconfig|findstr /i ipv6"); });
	connect(ui_->ipv4CheckBox, SIGNAL(clicked()), this, SLOT(onShowPortInfo()));
	connect(ui_->ipv6CheckBox, SIGNAL(clicked()), this, SLOT(onShowPortInfo()));
	connect(ui_->tcpListenCheckBox, SIGNAL(clicked()), this, SLOT(onShowPortInfo()));
	connect(ui_->tcpConnCheckBox, SIGNAL(clicked()), this, SLOT(onShowPortInfo()));
	connect(ui_->udpListenCheckBox, SIGNAL(clicked()), this, SLOT(onShowPortInfo()));
	connect(ui_->portFilterEdit, &QLineEdit::textChanged, [&](QString str) {onShowPortInfo(); });
	
	ui_->ipv4CheckBox->setChecked(true);
	ui_->tcpListenCheckBox->setChecked(true);
}

void KernelNetwork::ShowWfpInfo()
{
	DISABLE_RECOVER();
	ClearItemModelData(wfp_model_, 0);

	std::vector<CALLOUT_INFO> infos;
	//EnumWfpCallouts(infos);

	for (auto item : infos) {
		auto id_item = new QStandardItem(DWordToHexQ(item.CalloutId));
		auto key_item = new QStandardItem(DWordToHexQ(item.CalloutKey));
		auto name_item = new QStandardItem(item.ModuleName);
		auto count = wfp_model_->rowCount();
		wfp_model_->setItem(count, 0, id_item);
		wfp_model_->setItem(count, 1, key_item);
		wfp_model_->setItem(count, 2, name_item);
	}
}

void KernelNetwork::onShowPortInfo()
{
	DISABLE_RECOVER();
	ClearItemModelData(port_model_, 0);

	auto ipv4 = ui_->ipv4CheckBox->isChecked();
	auto ipv6 = ui_->ipv6CheckBox->isChecked();
	auto tcpls = ui_->tcpListenCheckBox->isChecked();
	auto tcpconn = ui_->tcpConnCheckBox->isChecked();
	auto udpls = ui_->udpListenCheckBox->isChecked();

	std::vector<ARK_NETWORK_ENDPOINT_ITEM> items;
	std::vector<ARK_NETWORK_ENDPOINT_ITEM> newers;
	if (tcpls || tcpconn) {
		if (ipv4) ArkDrvApi::Network::EnumTcp4Endpoints(items);
		if (ipv6) ArkDrvApi::Network::EnumTcp6Endpoints(items);
		for (auto &item : items) {
			if (tcpls && item.state == 2) newers.push_back(item);
			if (tcpconn && item.state != 2) newers.push_back(item);
		}
	}
	if (udpls) {
		items.clear();
		if (ipv4) ArkDrvApi::Network::EnumUdp4Endpoints(items);
		if (ipv6) ArkDrvApi::Network::EnumUdp6Endpoints(items);
		newers.insert(newers.end(), items.begin(), items.end());
	}

	auto flt = ui_->portFilterEdit->text();
	for (auto &item : newers) {
		auto protocol = CharsToQ(item.protocol);
		auto local = CharsToQ(item.local);
		auto remote = CharsToQ(item.remote);
		auto readable_state = (item.tran_ver == ARK_NETWORK_TCP) ? CharsToQ(item.readable_state) : "";
		auto pidstr = WStrToQ(UNONE::StrFormatW(L"%d", item.pid));
		ProcInfo pi;
		CacheGetProcInfo(item.pid, pi);
		if (!flt.isEmpty()) {
			if (!protocol.contains(flt, Qt::CaseInsensitive) && 
				!local.contains(flt, Qt::CaseInsensitive) &&
				!remote.contains(flt, Qt::CaseInsensitive) &&
				!readable_state.contains(flt, Qt::CaseInsensitive) &&
				!pidstr.contains(flt, Qt::CaseInsensitive) &&
				!pi.path.contains(flt, Qt::CaseInsensitive)
				) continue;
		}
		auto item_0 = new QStandardItem(protocol);
		auto item_1 = new QStandardItem(local);
		auto item_2 = new QStandardItem(remote);
		auto item_3 = new QStandardItem(readable_state);
		auto item_4 = new QStandardItem(pidstr);
		auto item_5 = new QStandardItem(LoadIcon(pi.path), pi.path);
		auto count = port_model_->rowCount();
		port_model_->setItem(count, 0, item_0);
		port_model_->setItem(count, 1, item_1);
		port_model_->setItem(count, 2, item_2);
		port_model_->setItem(count, 3, item_3);
		port_model_->setItem(count, 4, item_4);
		port_model_->setItem(count, 5, item_5);
	}
}