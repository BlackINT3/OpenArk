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
#include "kernel.h"
#include "driver/driver.h"
#include "../common/common.h"
#include "../common/utils/disassembly/disassembly.h"
#include "../openark/openark.h"
#include "wingui/wingui.h"
#include "../../../OpenArkDrv/arkdrv-api/arkdrv-api.h"

struct {
	int s = 0;
	int addr = s++;
	int type = s++;
	int path = s++;
	int desc = s++;
	int ver = s++;
	int corp = s++;
} NOTIFY;

bool NotifySortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const {
	bool ok;
	auto s1 = sourceModel()->data(left); auto s2 = sourceModel()->data(right);
	auto column = left.column();
	if ((column == NOTIFY.addr))
		return s1.toString().toULongLong(&ok, 16) < s2.toString().toULongLong(&ok, 16);
	return QString::compare(s1.toString(), s2.toString(), Qt::CaseInsensitive) < 0;
}

bool HotkeySortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const {
	auto s1 = sourceModel()->data(left); auto s2 = sourceModel()->data(right);
	return QString::compare(s1.toString(), s2.toString(), Qt::CaseInsensitive) < 0;
}

Kernel::Kernel(QWidget *parent, int tabid) :
	parent_((OpenArk*)parent)
{
	ui.setupUi(this);
	setAcceptDrops(true);
	network_ = nullptr;
	storage_ = nullptr;
	memory_ = nullptr;
	driver_ = nullptr;

	network_ = new KernelNetwork(); network_->ModuleInit(&ui, this);
	storage_ = new KernelStorage(); storage_->ModuleInit(&ui, this);
	memory_ = new KernelMemory(); memory_->ModuleInit(&ui, this);
	driver_ = new KernelDriver(); driver_->ModuleInit(&ui, this);

	InitKernelEntryView();
	InitNotifyView();
	InitHotkeyView();

	CommonMainTabObject::Init(ui.tabWidget, tabid);

	connect(this, SIGNAL(signalOpen(QString)), parent_, SLOT(onOpen(QString)));
}

Kernel::~Kernel()
{
}

bool Kernel::eventFilter(QObject *obj, QEvent *e)
{
	bool filtered = false;
	if (e->type() == QEvent::ContextMenu) {
		QMenu *menu = nullptr;
		if (obj == ui.notifyView->viewport()) menu = notify_menu_;
		else if (obj == ui.hotkeyView->viewport()) menu = hotkey_menu_;
		QContextMenuEvent *ctxevt = dynamic_cast<QContextMenuEvent*>(e);
		if (ctxevt && menu) {
			menu->move(ctxevt->globalPos());
			menu->show();
		}
	}

	if (network_) network_->EventFilter();
	if (driver_) driver_->eventFilter(obj, e);

	if (filtered) {
		dynamic_cast<QKeyEvent*>(e)->ignore();
		return true;
	}
	return QWidget::eventFilter(obj, e);
}

void Kernel::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->hasFormat("text/uri-list"))
		event->acceptProposedAction();
}

void Kernel::dropEvent(QDropEvent *event)
{
	if (!event->mimeData()->hasUrls())
		return;
	QString path = event->mimeData()->urls()[0].toLocalFile();
	onOpenFile(path);
}

void Kernel::onClickKernelMode()
{
	QString &&drvname = UNONE::OsIs64() ? "OpenArkDrv64.sys" : "OpenArkDrv32.sys";
	QString &&srvname = WStrToQ(UNONE::FsPathToPureNameW(drvname.toStdWString()));
	if (!arkdrv_conn_) {
		QString drvpath;
		drvpath = WStrToQ(UNONE::OsEnvironmentW(QToWStr(L"%Temp%\\" + drvname)));
		ExtractResource(":/OpenArk/driver/" + drvname, drvpath);
		{
			SignExpiredDriver(drvpath);
			RECOVER_SIGN_TIME();
			if (!driver_->InstallDriver(drvpath, srvname)) {
				QERR_W("InstallDriver %s err", QToWChars(drvpath));
				return;
			}
		}
		if (!ArkDrvApi::ConnectDriver()) {
			ERR("ConnectDriver err");
			return;
		}
		ui.kernelModeBtn->setText(tr("Exit KernelMode"));
		INFO("Enter KernelMode ok");
	} else {
		if (!driver_->UninstallDriver(srvname)) {
			QERR_W("UninstallDriver %s err", QToWChars(srvname));
			return;
		}
		if (!ArkDrvApi::DisconnectDriver()) {
			ERR("DisconnectDriver err");
			return;
		}
		ui.kernelModeBtn->setText(tr("Enter KernelMode"));
		INFO("Exit KernelMode ok");
	}
	onRefreshKernelMode();
}

void Kernel::onRefreshKernelMode()
{
	bool conn = ArkDrvApi::HeartBeatPulse();
	if (conn && !arkdrv_conn_) {
		ui.kernelModeStatus->setText(tr("[KernelMode] Enter successfully..."));
		ui.kernelModeStatus->setStyleSheet("color:green");
		ui.kernelModeBtn->setText(tr("Exit KernelMode"));
		arkdrv_conn_ = true;
		onTabChanged(ui.tabWidget->currentIndex());
	}
	if (!conn && arkdrv_conn_) {
		ui.kernelModeStatus->setText(tr("[KernelMode] Exit successfully..."));
		ui.kernelModeStatus->setStyleSheet("color:red");
		ui.kernelModeBtn->setText(tr("Enter KernelMode"));
		arkdrv_conn_ = false;
	}
}

void Kernel::onOpenFile(QString path)
{
	if (!UNONE::FsIsFileW(path.toStdWString()))
		return;
	path = WStrToQ(UNONE::FsPathStandardW(path.toStdWString()));
	ui.driverFileEdit->setText(path);
	auto &&name = UNONE::FsPathToPureNameW(path.toStdWString());
	ui.serviceEdit->setText(WStrToQ(name));
}

void Kernel::onTabChanged(int index)
{
	switch (index) {
	case TAB_KERNEL_NOTIFY:
		ShowSystemNotify();
		break;
	case TAB_KERNEL_HOTKEY:
		ShowSystemHotkey();
		break;
	default:
		break;
	}
	CommonMainTabObject::onTabChanged(index);
}

void Kernel::InitKernelEntryView()
{
	kerninfo_model_ = new QStandardItemModel;
	SetDefaultTableViewStyle(ui.kernelInfoView, kerninfo_model_);
	ui.kernelModeStatus->setText(tr("[KernelMode] Enter kernel mode needed before using the features(Hotkey/Notify/Memory...)"));
	ui.kernelModeStatus->setStyleSheet("color:red");
	kerninfo_model_->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Value"));
	ui.kernelInfoView->setColumnWidth(0, 120);

	int up_seq = 0;
	auto AddSummaryUpItem = [&](QString name, QString value) {
		kerninfo_model_->setItem(up_seq, 0, new QStandardItem(name));
		kerninfo_model_->setItem(up_seq, 1, new QStandardItem(value));
		up_seq++;
	};

	SYSTEM_INFO sys;
	GetSystemInfo(&sys);
	sys.dwNumberOfProcessors;

	OSVERSIONINFOEXW info;
	info.dwOSVersionInfoSize = sizeof(info);
	GetVersionExW((LPOSVERSIONINFOW)&info);

	PERFORMANCE_INFORMATION perf = { 0 };
	GetPerformanceInfo(&perf, sizeof(perf));
	double gb = round((double)(perf.PhysicalTotal*perf.PageSize) / 1024 / 1024 / 1024);

	auto major = UNONE::OsMajorVer();
	AddSummaryUpItem(tr("MajorVersion"), DWordToDecQ(major));
	AddSummaryUpItem(tr("MiniorVersion"), DWordToDecQ(UNONE::OsMinorVer()));
	if (major >= 10) AddSummaryUpItem(tr("ReleaseNumber"), DWordToDecQ(UNONE::OsReleaseNumber()));
	AddSummaryUpItem(tr("BuildNumber"), DWordToDecQ(UNONE::OsBuildNumber()));
	AddSummaryUpItem(tr("MajorServicePack"), DWordToDecQ(info.wServicePackMajor));
	AddSummaryUpItem(tr("MiniorServicePack"), DWordToDecQ(info.wServicePackMinor));
	ULONG64 r3start = (ULONG64)sys.lpMinimumApplicationAddress;
	ULONG64 r3end = (ULONG64)sys.lpMaximumApplicationAddress;
	ULONG64 r0start = 0xFFFF080000000000;
	ULONG64 r0end = 0xFFFFFFFFFFFFFFFF;
	if (!UNONE::OsIs64()) {
		r0start = 0x80000000;
		r0end = 0xFFFFFFFF;
	}
	AddSummaryUpItem(tr("R3 AddressRange"), StrToQ(UNONE::StrFormatA("0x%llX - 0x%llX", r3start, r3end)));
	AddSummaryUpItem(tr("R0 AddressRange"), StrToQ(UNONE::StrFormatA("0x%llX - 0x%llX", r0start, r0end)));
	AddSummaryUpItem(tr("Page Size"), StrToQ(UNONE::StrFormatA("%d KB", sys.dwPageSize / 1024)));
	AddSummaryUpItem(tr("Physical Memory"), StrToQ(UNONE::StrFormatA("%d GB", (int)gb)));
	AddSummaryUpItem(tr("CPU Count"), DWordToDecQ(sys.dwNumberOfProcessors));
	AddSummaryUpItem(tr("SystemRoot"), WStrToQ(UNONE::OsWinDirW()));

	connect(ui.kernelModeBtn, SIGNAL(clicked()), this, SLOT(onClickKernelMode()));

	arkdrv_conn_ = false;
	auto timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(onRefreshKernelMode()));
	timer->setInterval(1000);
	timer->start();
}

void Kernel::InitNotifyView()
{
	notify_model_ = new QStandardItemModel;
	QTreeView *view = ui.notifyView;
	proxy_notify_ = new NotifySortFilterProxyModel(view);
	proxy_notify_->setSourceModel(notify_model_);
	proxy_notify_->setDynamicSortFilter(true);
	proxy_notify_->setFilterKeyColumn(1);
	view->setModel(proxy_notify_);
	view->selectionModel()->setModel(proxy_notify_);
	view->header()->setSortIndicator(-1, Qt::AscendingOrder);
	view->setSortingEnabled(true);
	view->viewport()->installEventFilter(this);
	view->installEventFilter(this);
	notify_model_->setHorizontalHeaderLabels(QStringList() << tr("Callback Entry") << tr("Type") << tr("Path") << tr("Description") << tr("Version") << tr("Company"));
	view->setColumnWidth(NOTIFY.addr, 150);
	view->setColumnWidth(NOTIFY.type, 100);
	view->setColumnWidth(NOTIFY.path, 360);
	view->setColumnWidth(NOTIFY.desc, 230);
	view->setColumnWidth(NOTIFY.ver, 120);
	view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	notify_menu_ = new QMenu();
	notify_menu_->addAction(tr("Refresh"), this, [&] { ShowSystemNotify(); });
	notify_menu_->addSeparator();
	notify_menu_->addAction(tr("Delete Notify"), this, [&] {
		ULONG64 addr;
		addr = QHexToQWord(NotifyItemData(NOTIFY.addr));
		NOTIFY_TYPE type;
		auto &&qstr = NotifyItemData(NOTIFY.type);
		if (qstr == tr("CreateProcess")) type = CREATE_PROCESS;
		else if (qstr == tr("CreateThread")) type = CREATE_THREAD;
		else if (qstr == tr("LoadImage")) type = LOAD_IMAGE;
		else if (qstr == tr("CmpCallback")) type = CM_REGISTRY;
		ArkDrvApi::Notify::NotifyRemove(type, addr);
		ShowSystemNotify();
	});
	notify_menu_->addAction(tr("Disassemble Notify"), this, [&] {
		QString &&qstr = NotifyItemData(NOTIFY.addr);
		ULONG64 addr = QHexToQWord(qstr);
		ULONG size = 0x100;
		auto memrw = new KernelMemoryRW();
		memrw->ViewMemory(addr, size);
		auto memwidget = memrw->GetWidget();
		memwidget->findChild<QLineEdit*>("readAddrEdit")->setText(qstr);
		memwidget->findChild<QLineEdit*>("readSizeEdit")->setText(DWordToHexQ(size));
		memwidget->setParent(qobject_cast<QWidget*>(this->parent()));
		memwidget->setWindowTitle(tr("Memory Read-Write"));
		memwidget->setWindowFlags(Qt::Window);
		memwidget->resize(1000, 530);
		memwidget->show();

		//SetActiveTab(QVector<int>({ KernelTabMemory, KernelMemory::View }));
	});
	notify_menu_->addSeparator();
	notify_menu_->addAction(tr("Copy"), this, [&] {
		ClipboardCopyData(NotifyItemData(GetCurViewColumn(ui.driverView)).toStdString());
	});
	notify_menu_->addAction(tr("Sendto Scanner"), this, [&] {
		parent_->SetActiveTab(TAB_SCANNER);
		emit signalOpen(NotifyItemData(NOTIFY.path));
	});
	notify_menu_->addAction(tr("Explore File"), this, [&] {
		ExploreFile(NotifyItemData(NOTIFY.path));
	});
	notify_menu_->addAction(tr("Properties..."), this, [&]() {
		WinShowProperties(NotifyItemData(NOTIFY.path).toStdWString());
	});
}

void Kernel::InitHotkeyView()
{
	hotkey_model_ = new QStandardItemModel;
	QTreeView *view = ui.hotkeyView;
	proxy_hotkey_ = new HotkeySortFilterProxyModel(view);
	proxy_hotkey_->setSourceModel(hotkey_model_);
	proxy_hotkey_->setDynamicSortFilter(true);
	proxy_hotkey_->setFilterKeyColumn(1);
	view->setModel(proxy_hotkey_);
	view->selectionModel()->setModel(proxy_hotkey_);
	view->header()->setSortIndicator(-1, Qt::AscendingOrder);
	view->setSortingEnabled(true);
	view->viewport()->installEventFilter(this);
	view->installEventFilter(this);
	std::pair<int, QString> colum_layout[] = { 
	{ 150, tr("Name") },
	{ 100, tr("PID.TID") },
	{ 200, tr("Hotkey") },
	{ 100, tr("HotkeyID") },
	{ 100, tr("HWND") },
	{ 180, tr("Title") },
	{ 180, tr("ClassName") },
	{ 300, tr("Path") },
	{ 120, tr("Description") } };
	QStringList name_list;
	for (auto p : colum_layout) {
		name_list << p.second;
	}
	hotkey_model_->setHorizontalHeaderLabels(name_list);
	for (int i = 0; i < _countof(colum_layout); i++) {
		view->setColumnWidth(i, colum_layout[i].first);
	}
	view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	hotkey_menu_ = new QMenu();
	hotkey_menu_->addAction(tr("Refresh"), this, [&] { ShowSystemHotkey(); });
	hotkey_menu_->addSeparator();
	hotkey_menu_->addAction(tr("Delete Hotkey"), this, [&] {
		ULONG32 vkid = QHexToDWord(HotkeyItemData(3));
		auto arr = HotkeyItemData(1).split(".");
		ULONG32 pid = QDecToDWord(arr[0]);
		ULONG32 tid = QDecToDWord(arr[1]);
		HOTKEY_ITEM item;
		item.id = vkid;
		item.pid = pid;
		item.tid = tid;
		if (!ArkDrvApi::WinGUI::HotkeyRemoveInfo(item)) {
			auto err = UNONE::StrFormatW(L"Remove Hotkey %d.%d id:%x err:%s",
				pid, tid, vkid, UNONE::OsDosErrorMsgW(GetLastError()).c_str());
			MsgBoxError(WStrToQ(err));
			return;
		}
		INFO(L"Remove Hotkey %d.%d id:%x ok", pid, tid, vkid);
		proxy_hotkey_->removeRows(ui.hotkeyView->currentIndex().row(), 1);
	});
	hotkey_menu_->addSeparator();
	hotkey_menu_->addAction(tr("Sendto Scanner"), this, [&] {
		parent_->SetActiveTab(TAB_SCANNER);
		emit signalOpen(HotkeyItemData(7));
	});
	hotkey_menu_->addAction(tr("Explore File"), this, [&] {
		ExploreFile(HotkeyItemData(7));
	});
	hotkey_menu_->addAction(tr("Properties..."), this, [&]() {
		WinShowProperties(HotkeyItemData(7).toStdWString());
	});
}

void Kernel::ShowSystemNotify()
{
	DISABLE_RECOVER();
	ClearItemModelData(notify_model_, 0);
	
	std::vector<DRIVER_ITEM> infos;
	ArkDrvApi::Driver::DriverEnumInfo(infos);
	
	auto OutputNotify = [&](std::vector<ULONG64> &routines, QString type) {
		for (auto routine : routines) {
			QString path;
			for (auto info : infos) {
				if (IN_RANGE(routine, info.base, info.size)) {
					path = WStrToQ(ParseDriverPath(info.path));
					break;
				}
			}
			bool microsoft = true;
			bool existed = true;
			auto info = CacheGetFileBaseInfo(path);
			if (info.desc.isEmpty()) {
				if (!UNONE::FsIsExistedW(info.path.toStdWString())) {
					info.desc = tr("[-] Driver file not existed!");
					existed = false;
				}
			}
			if (!info.corp.contains("Microsoft", Qt::CaseInsensitive)) { microsoft = false; }
			QStandardItem *addr_item = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"0x%llX", routine)));
			QStandardItem *type_item = new QStandardItem(type);
			QStandardItem *path_item = new QStandardItem(path);
			QStandardItem *desc_item = new QStandardItem(info.desc);
			QStandardItem *ver_item = new QStandardItem(info.ver);
			QStandardItem *corp_item = new QStandardItem(info.corp);
			auto count = notify_model_->rowCount();
			notify_model_->setItem(count, NOTIFY.addr, addr_item);
			notify_model_->setItem(count, NOTIFY.type, type_item);
			notify_model_->setItem(count, NOTIFY.path, path_item);
			notify_model_->setItem(count, NOTIFY.desc, desc_item);
			notify_model_->setItem(count, NOTIFY.ver, ver_item);
			notify_model_->setItem(count, NOTIFY.corp, corp_item);
			if (!existed) SetLineBgColor(notify_model_, count, Qt::red);
			else if (!microsoft) SetLineBgColor(notify_model_, count, QBrush(0xffffaa));
		}
	};

	std::vector<ULONG64> routines;
	if (!ArkDrvApi::Notify::NotifyEnumProcess(routines)) QERR_W("NotifyEnumProcess err");
	OutputNotify(routines, tr("CreateProcess"));

	if (!ArkDrvApi::Notify::NotifyEnumThread(routines)) QERR_W("NotifyEnumThread err");
	OutputNotify(routines, tr("CreateThread"));

	if (!ArkDrvApi::Notify::NotifyEnumImage(routines)) QERR_W("NotifyEnumImage err");
	OutputNotify(routines, tr("LoadImage"));

	if (!ArkDrvApi::Notify::NotifyEnumRegistry(routines)) QERR_W("NotifyEnumRegistry err");
	OutputNotify(routines, tr("CmpCallback"));
}

void Kernel::ShowSystemHotkey()
{
	DISABLE_RECOVER();
	ClearItemModelData(hotkey_model_, 0);

	std::vector<HOTKEY_ITEM> infos;
	ArkDrvApi::WinGUI::HotkeyEnumInfo(infos);

	for (auto item : infos) {
		auto pid = item.pid;
		auto &&path = UNONE::PsGetProcessPathW(pid);
		auto &&name = UNONE::FsPathToNameW(path);
		if (name.empty()) name = UNONE::StrToW((char*)item.name);
		auto info = CacheGetFileBaseInfo(WStrToQ(path));
		auto name_item = new QStandardItem(LoadIcon(WStrToQ(path)), WStrToQ(name));
		auto wnd_item = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"0x%X", item.wnd)));
		auto title_item = new QStandardItem(WStrToQ(UNONE::PsGetWndTextW((HWND)item.wnd)));
		auto class_item = new QStandardItem(WStrToQ(UNONE::PsGetWndClassNameW((HWND)item.wnd)));
		auto ptid_item = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"%d.%d", item.pid, item.tid)));
		auto vk_item = new QStandardItem(StrToQ(HotkeyVkToString(item.vk, item.mod1, item.mod2)));
		auto vkid_item = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"0x%X", item.id)));
		auto path_item = new QStandardItem(WStrToQ(path));
		auto desc_item = new QStandardItem(info.desc);
		auto count = hotkey_model_->rowCount();
		hotkey_model_->setItem(count, 0, name_item);
		hotkey_model_->setItem(count, 1, ptid_item);
		hotkey_model_->setItem(count, 2, vk_item);
		hotkey_model_->setItem(count, 3, vkid_item);
		hotkey_model_->setItem(count, 4, wnd_item);
		hotkey_model_->setItem(count, 5, title_item);
		hotkey_model_->setItem(count, 6, class_item);
		hotkey_model_->setItem(count, 7, path_item);
		hotkey_model_->setItem(count, 8, desc_item);
	}
}

QString Kernel::NotifyItemData(int column)
{
	return GetCurItemViewData(ui.notifyView, column);
}

QString Kernel::HotkeyItemData(int column)
{
	return GetCurItemViewData(ui.hotkeyView, column);
}