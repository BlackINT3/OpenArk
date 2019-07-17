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
#include "../../../OpenArkDrv/arkdrv-api/arkdrv-api.h"

#define KernelTabEntry 0
#define KernelTabDrivers 1
#define KernelTabDriverKit 2
#define KernelTabNotify 3
#define KernelTabMemory 4

struct {
	int s = 0;
	int name = s++;
	int base = s++;
	int path = s++;
	int number = s++;
	int desc = s++;
	int ver = s++;
	int corp = s++;
} DRV;

struct {
	int s = 0;
	int addr = s++;
	int type = s++;
	int path = s++;
	int desc = s++;
	int ver = s++;
	int corp = s++;
} NOTIFY;

bool DriversSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
	bool ok;
	auto s1 = sourceModel()->data(left); auto s2 = sourceModel()->data(right);
	auto column = left.column();
	if ((column == DRV.base || column == DRV.number))
		return s1.toString().toULongLong(&ok, 16) < s2.toString().toULongLong(&ok, 16);
	return QString::compare(s1.toString(), s2.toString(), Qt::CaseInsensitive) < 0;
}

bool NotifySortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const {
	bool ok;
	auto s1 = sourceModel()->data(left); auto s2 = sourceModel()->data(right);
	auto column = left.column();
	if ((column == NOTIFY.addr))
		return s1.toString().toULongLong(&ok, 16) < s2.toString().toULongLong(&ok, 16);
	return QString::compare(s1.toString(), s2.toString(), Qt::CaseInsensitive) < 0;
}

Kernel::Kernel(QWidget *parent) :
	parent_((OpenArk*)parent)
{
	ui.setupUi(this);
	ui.tabWidget->setTabPosition(QTabWidget::West);
	ui.tabWidget->tabBar()->setStyle(new OpenArkTabStyle);
	setAcceptDrops(true);
	connect(ui.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onTabChanged(int)));

	InitKernelEntryView();
	InitDriversView();
	InitDriverKitView();
	InitNotifyView();
	InitMemoryView();
}

Kernel::~Kernel()
{
}

bool Kernel::eventFilter(QObject *obj, QEvent *e)
{
	bool filtered = false;
	if (obj == ui.driverView->viewport()) {
		if (e->type() == QEvent::ContextMenu) {
			QContextMenuEvent *ctxevt = dynamic_cast<QContextMenuEvent*>(e);
			if (ctxevt) {
				drivers_menu_->move(ctxevt->globalPos());
				drivers_menu_->show();
			}
		}
	} else if (obj == ui.notifyView->viewport()) {
		if (e->type() == QEvent::ContextMenu) {
			QContextMenuEvent *ctxevt = dynamic_cast<QContextMenuEvent*>(e);
			if (ctxevt) {
				notify_menu_->move(ctxevt->globalPos());
				notify_menu_->show();
			}
		}
	}

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
	case KernelTabDrivers:
		ShowDrivers();
		break;
	case KernelTabNotify:
		ShowSystemNotify();
		break;
	default:
		break;
	}
}

void Kernel::onSignDriver()
{
	QString driver = ui.driverFileEdit->text();
	if (SignExpiredDriver(driver)) {
		ui.infoLabel->setText(tr("Sign ok..."));
		ui.infoLabel->setStyleSheet("color:green");
	} else {
		ui.infoLabel->setText(tr("Sign failed, open console window to view detail..."));
		ui.infoLabel->setStyleSheet("color:red");
	}
}

void Kernel::onInstallNormallyDriver()
{
	if (InstallDriver(ui.driverFileEdit->text(), ui.serviceEdit->text())) {
		ui.infoLabel->setText(tr("Install ok..."));
		ui.infoLabel->setStyleSheet("color:green");
	} else {
		ui.infoLabel->setText(tr("Install failed, open console window to view detail..."));
		ui.infoLabel->setStyleSheet("color:red");
	}
}

void Kernel::onInstallUnsignedDriver()
{
	onSignDriver();
	RECOVER_SIGN_TIME();
	onInstallNormallyDriver();
}

void Kernel::onInstallExpiredDriver()
{
	RECOVER_SIGN_TIME();
	onInstallNormallyDriver();
}

void Kernel::onUninstallDriver()
{
	if (UninstallDriver(ui.serviceEdit->text())) {
		ui.infoLabel->setText(tr("Uninstall ok..."));
		ui.infoLabel->setStyleSheet("color:green");
	} else {
		ui.infoLabel->setText(tr("Uninstall failed, open console window to view detail..."));
		ui.infoLabel->setStyleSheet("color:red");
	}
}

void Kernel::InitKernelEntryView()
{
	kerninfo_model_ = new QStandardItemModel;
	SetDefaultTableViewStyle(ui.kernelInfoView, kerninfo_model_);
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

	AddSummaryUpItem(tr("MajorVersion"), DWordToDecQ(UNONE::OsMajorVer()));
	AddSummaryUpItem(tr("MiniorVersion"), DWordToDecQ(UNONE::OsMinorVer()));
	AddSummaryUpItem(tr("BuildNumber"), DWordToDecQ(UNONE::OsBuildNumber()));
	AddSummaryUpItem(tr("MajorServicePack"), DWordToDecQ(info.wServicePackMajor));
	AddSummaryUpItem(tr("MiniorServicePack"), DWordToDecQ(info.wServicePackMinor));
	AddSummaryUpItem(tr("R3 AddressRange"), StrToQ(UNONE::StrFormatA("%p - %p", sys.lpMinimumApplicationAddress, sys.lpMaximumApplicationAddress)));
	AddSummaryUpItem(tr("Page Size"), StrToQ(UNONE::StrFormatA("%d KB", sys.dwPageSize/1024)));
	AddSummaryUpItem(tr("Physical Memory"), StrToQ(UNONE::StrFormatA("%d GB", (int)gb)));
	AddSummaryUpItem(tr("CPU Count"), DWordToDecQ(sys.dwNumberOfProcessors));
	AddSummaryUpItem(tr("SystemRoot"), WStrToQ(UNONE::OsWinDirW()));

	connect(ui.kernelModeBtn, &QPushButton::clicked, this, [&]() {
		if (!arkdrv_conn_) {
			QString driver;
			if (UNONE::OsIs64()) {
				driver = WStrToQ(UNONE::OsEnvironmentW(L"%Temp%\\OpenArkDrv64.sys"));
				ExtractResource(":/OpenArk/driver/OpenArkDrv64.sys", driver);
			} else {
				driver = WStrToQ(UNONE::OsEnvironmentW(L"%Temp%\\OpenArkDrv32.sys"));
				ExtractResource(":/OpenArk/driver/OpenArkDrv32.sys", driver);
			}
			{
				SignExpiredDriver(driver);
				RECOVER_SIGN_TIME();
				auto &&name = UNONE::FsPathToPureNameW(driver.toStdWString());
				if (!InstallDriver(driver, WStrToQ(name))) {
					QERR_W("InstallDriver %s err", driver);
					return;
				}
			}
			bool ret = ArkDrvApi::ConnectDriver();
			if (!ret) {
				ERR("ConnectDriver err");
				return;
			}
			INFO("Enter KernelMode ok");
		}		
	});

	arkdrv_conn_ = false;
	auto timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, [&]() {
		bool conn = ArkDrvApi::HeartBeatPulse();
		if (conn && !arkdrv_conn_) {
			ui.kernelModeStatus->setText(tr("[KernelMode] Connect successfully..."));
			ui.kernelModeStatus->setStyleSheet("color:green");
			ui.kernelModeBtn->setEnabled(false);
			arkdrv_conn_ = true;
		}
		if (!conn && arkdrv_conn_) {
			ui.kernelModeStatus->setText(tr("[KernelMode] Disconnected..."));
			ui.kernelModeStatus->setStyleSheet("color:red");
			ui.kernelModeBtn->setEnabled(true);
			arkdrv_conn_ = false;
		}
	});
	timer->setInterval(1000);
	timer->start();
}

void Kernel::InitDriversView()
{
	drivers_model_ = new QStandardItemModel;
	QTreeView *view = ui.driverView;
	proxy_drivers_ = new DriversSortFilterProxyModel(view);
	proxy_drivers_->setSourceModel(drivers_model_);
	proxy_drivers_->setDynamicSortFilter(true);
	proxy_drivers_->setFilterKeyColumn(1);
	view->setModel(proxy_drivers_);
	view->selectionModel()->setModel(proxy_drivers_);
	view->header()->setSortIndicator(-1, Qt::AscendingOrder);
	view->setSortingEnabled(true);
	view->viewport()->installEventFilter(this);
	view->installEventFilter(this);
	drivers_model_->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Base") << tr("Path") << tr("Number") << tr("Description") << tr("Version") << tr("Company"));
	view->setColumnWidth(DRV.name, 138);
	view->setColumnWidth(DRV.base, 135);
	view->setColumnWidth(DRV.path, 285);
	view->setColumnWidth(DRV.number, 60);
	view->setColumnWidth(DRV.desc, 180);
	//dview->setColumnWidth(DRV.corp, 155);
	view->setColumnWidth(DRV.ver, 120);
	view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	drivers_menu_ = new QMenu();
	drivers_menu_->addAction(tr("Refresh"), this, [&] { ShowDrivers(); });
	drivers_menu_->addAction(tr("Copy"), this, [&] {
		ClipboardCopyData(DriversItemData(GetCurViewColumn(ui.driverView)).toStdString());
	});
	drivers_menu_->addAction(tr("Sendto Scanner"), this, [&] {
		parent_->ActivateTab(TAB_SCANNER);
		emit signalOpen(DriversItemData(DRV.path));
	});
	drivers_menu_->addAction(tr("Explore File"), this, [&] {
		ExploreFile(DriversItemData(DRV.path));
	});
	drivers_menu_->addAction(tr("Properties..."), this, [&] {
		WinShowProperties(DriversItemData(DRV.path).toStdWString());
	});
}

void Kernel::InitDriverKitView()
{
	connect(ui.browseBtn, &QPushButton::clicked, this, [&]() {
		QString file = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("Driver Files (*.sys);;All Files (*.*)"));
		onOpenFile(file);
	});
	connect(ui.signBtn, SIGNAL(clicked()), this, SLOT(onSignDriver()));
	connect(ui.installNormallyBtn, SIGNAL(clicked()), this, SLOT(onInstallNormallyDriver()));
	connect(ui.installUnsignedBtn, SIGNAL(clicked()), this, SLOT(onInstallUnsignedDriver()));
	connect(ui.installExpiredBtn, SIGNAL(clicked()), this, SLOT(onInstallExpiredDriver()));
	connect(ui.uninstallBtn, SIGNAL(clicked()), this, SLOT(onUninstallDriver()));
	connect(this, SIGNAL(signalOpen(QString)), parent_, SLOT(onOpen(QString)));
}

void Kernel::InitNotifyView()
{
	notify_model_ = new QStandardItemModel;
	QTreeView *view = ui.notifyView;
	proxy_notify_ = new DriversSortFilterProxyModel(view);
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
		ArkDrvApi::NotifyRemove(type, addr);
		ShowSystemNotify();
	});
	notify_menu_->addAction(tr("Disassemble Notify"), this, [&] {
		QString &&qstr = NotifyItemData(NOTIFY.addr);
		ULONG64 addr;
		ULONG size;
		addr = QHexToQWord(qstr);
		size = 0x100;
		ui.addrEdit->setText(qstr);
		ui.sizeEdit->setText(DWordToHexQ(size));
		ShowDumpMemory(addr, size);
		ui.tabWidget->setCurrentIndex(KernelTabMemory);
	});
	notify_menu_->addSeparator();
	notify_menu_->addAction(tr("Copy"), this, [&] {
		ClipboardCopyData(NotifyItemData(GetCurViewColumn(ui.driverView)).toStdString());
	});
	notify_menu_->addAction(tr("Sendto Scanner"), this, [&] {
		parent_->ActivateTab(TAB_SCANNER);
		emit signalOpen(NotifyItemData(NOTIFY.path));
	});
	notify_menu_->addAction(tr("Explore File"), this, [&] {
		ExploreFile(NotifyItemData(NOTIFY.path));
	});
	notify_menu_->addAction(tr("Properties..."), this, [&]() {
		WinShowProperties(NotifyItemData(NOTIFY.path).toStdWString());
	});
}

void Kernel::InitMemoryView()
{
	connect(ui.dumpmemBtn, &QPushButton::clicked, this, [&] {
		ULONG64 addr = VariantInt64(ui.addrEdit->text().toStdString());
		ULONG size = VariantInt(ui.sizeEdit->text().toStdString());
		ShowDumpMemory(addr, size);
	});

}

bool Kernel::InstallDriver(QString driver, QString name)
{
	if (driver.isEmpty()) {
		QERR_W("driver path is empty");
		return false;
	}
	auto &&path = driver.toStdWString();
	return UNONE::ObLoadDriverW(path, name.toStdWString());
}

bool Kernel::UninstallDriver(QString service)
{
	if (service.isEmpty()) {
		QERR_W("service is empty");
		return false;
	}
	return UNONE::ObUnloadDriverW(service.toStdWString());
}

void Kernel::ShowDrivers()
{
	DISABLE_RECOVER();
	ClearItemModelData(drivers_model_, 0);

	std::vector<LPVOID> drivers;
	UNONE::ObGetDriverList(drivers);
	int number = 0;
	for (auto d : drivers) {
		static int major = UNONE::OsMajorVer();
		auto &&w_path = UNONE::ObGetDriverPathW(d);
		if (major <= 5) {
			if (UNONE::StrIndexIW(w_path, L"\\Windows") == 0) {
				static auto &&drive = UNONE::OsEnvironmentW(L"%SystemDrive%");
				w_path = drive + w_path;
			} else if (w_path.find(L'\\') == std::wstring::npos && w_path.find(L'/') == std::wstring::npos) {
				static auto &&driverdir = UNONE::OsSystem32DirW() + L"\\drivers\\";
				w_path = driverdir + w_path;
			}
		}

		auto &&path = WStrToQ(w_path);
		auto &&name = WStrToQ(UNONE::ObGetDriverNameW(d));

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

		QStandardItem *name_item = new QStandardItem(name);
		QStandardItem *base_item = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"0x%p", d)));
		QStandardItem *path_item = new QStandardItem(path);
		QStandardItem *number_item = new QStandardItem(QString("%1").arg(number));
		QStandardItem *desc_item = new QStandardItem(info.desc);
		QStandardItem *ver_item = new QStandardItem(info.ver);
		QStandardItem *corp_item = new QStandardItem(info.corp);

		auto count = drivers_model_->rowCount();
		drivers_model_->setItem(count, DRV.name, name_item);
		drivers_model_->setItem(count, DRV.base, base_item);
		drivers_model_->setItem(count, DRV.path, path_item);
		drivers_model_->setItem(count, DRV.number, number_item);
		drivers_model_->setItem(count, DRV.desc, desc_item);
		drivers_model_->setItem(count, DRV.ver, ver_item);
		drivers_model_->setItem(count, DRV.corp, corp_item);
		if (!existed) SetLineBgColor(drivers_model_, count, Qt::red);
		else if (!microsoft) SetLineBgColor(drivers_model_, count, QBrush(0xffffaa));
		number++;
	}
}

void Kernel::ShowSystemNotify()
{
	DISABLE_RECOVER();
	ClearItemModelData(notify_model_, 0);
	
	std::vector<DRIVER_ITEM> infos;
	ArkDrvApi::DriverEnumInfo(infos);
	
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
	if (!ArkDrvApi::NotifyEnumProcess(routines)) QERR_W("NotifyEnumProcess err");
	OutputNotify(routines, tr("CreateProcess"));

	if (!ArkDrvApi::NotifyEnumThread(routines)) QERR_W("NotifyEnumThread err");
	OutputNotify(routines, tr("CreateThread"));

	if (!ArkDrvApi::NotifyEnumImage(routines)) QERR_W("NotifyEnumImage err");
	OutputNotify(routines, tr("LoadImage"));

	if (!ArkDrvApi::NotifyEnumRegistry(routines)) QERR_W("NotifyEnumRegistry err");
	OutputNotify(routines, tr("CmpCallback"));
}

void Kernel::ShowDumpMemory(ULONG64 addr, ULONG size)
{
	std::vector<DRIVER_ITEM> infos;
	ArkDrvApi::DriverEnumInfo(infos);
	QString path;
	for (auto info : infos) {
		if (IN_RANGE(addr, info.base, info.size)) {
			path = WStrToQ(ParseDriverPath(info.path));
			break;
		}
	}
	if (!path.isEmpty()) ui.regionLabel->setText(path);
	char *mem = nullptr;
	ULONG memsize = 0;
	std::string buf;
	if (ArkDrvApi::MemoryRead(addr, size, buf)) {
		mem = (char*)buf.c_str();
		memsize = buf.size();
	}
	auto hexdump = HexDumpMemory(addr, mem, size);
	auto disasm = DisasmMemory(addr, mem, size);
	ui.hexEdit->setText(StrToQ(hexdump));
	ui.disasmEdit->setText(StrToQ(disasm));
}

int Kernel::DriversCurRow()
{
	auto idx = ui.driverView->currentIndex();
	return idx.row();
}

QString Kernel::DriversItemData(int column)
{
	return GetCurItemViewData(ui.driverView, column);
}

QString Kernel::NotifyItemData(int column)
{
	return GetCurItemViewData(ui.notifyView, column);
}