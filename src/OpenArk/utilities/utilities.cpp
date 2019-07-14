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
#include "utilities.h"
#include "../common/common.h"
#include "../openark/openark.h"

struct {
	int s = 0;
	int name = s++;
	int path = s++;
	int size = s++;
} JUNKS;
bool JunksSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
	auto s1 = sourceModel()->data(left); auto s2 = sourceModel()->data(right);
	auto column = left.column();
	if ((column == JUNKS.size)) return s1.toUInt() < s2.toUInt();
	return QString::compare(s1.toString(), s2.toString(), Qt::CaseInsensitive) < 0;
}

Utilities::Utilities(QWidget *parent) :
	parent_((OpenArk*)parent),
	scanjunks_thread_(nullptr),
	cleanjunks_thread_(nullptr)
{
	ui.setupUi(this);
	ui.tabWidget->setTabPosition(QTabWidget::West);
	ui.tabWidget->tabBar()->setStyle(new OpenArkTabStyle);
	qRegisterMetaType<QList<JunkItem>>("QList<JunkItem>");

	InitCleanerView();
	InitSystemToolsView();
}

Utilities::~Utilities()
{
	if (scanjunks_thread_) scanjunks_thread_->terminate();
	scanjunks_thread_ = nullptr;
}

void Utilities::onAppendJunkfiles(QList<JunkItem> items)
{
	for (auto &item : items) {
		auto count = junks_model_->rowCount();
		QStandardItem *name_item = new QStandardItem(LoadIcon(item.path), item.name);
		QStandardItem *path_item = new QStandardItem(item.path);
		QStandardItem *size_item = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"%.02f MB", item.size / MB)));
		junks_model_->setItem(count, JUNKS.name, name_item);
		junks_model_->setItem(count, JUNKS.path, path_item);
		junks_model_->setItem(count, JUNKS.size, size_item);
	}
}

void ScanJunksThread::run()
{
	std::function<bool(wchar_t*, wchar_t*, void*)> ScanCallback;
	QList<JunkItem> items;
	ScanCallback = [&](wchar_t* path, wchar_t* name, void* param)->bool {
		if (UNONE::FsIsDirW(path)) {
			return UNONE::FsEnumDirectoryW(path, ScanCallback);
		}
		JunkItem item;
		item.name = WCharsToQ(name);
		item.path = WCharsToQ(path);
		DWORD64 fsize;
		UNONE::FsGetFileSizeW(path, fsize);
		item.size = fsize;
		items.push_back(item);
		if (items.size() >= 2000) {
			emit appendJunks(items);
			items.clear();
			Sleep(1000);
		}
		return true;
	};
	auto &&junkdirs = ConfGetJunksDir();
	for (auto &dir : junkdirs) {
		UNONE::FsEnumDirectoryW(dir.toStdWString(), ScanCallback);
	}
	if (items.size() > 0) {
		emit appendJunks(items);
		items.clear();
	}
}

void CleanJunksThread::run()
{
	for (auto &junk : junks_) {
		auto &&path = junk.toStdWString();
		auto ret = DeleteFileW(path.c_str());
		if (!ret) {
			ERR(L"DleteFile %s err:%d", path.c_str(), GetLastError());
		}
	}
}

void Utilities::InitCleanerView()
{
	junks_model_ = new QStandardItemModel;
	junks_model_->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Path") << tr("Size"));
	QTreeView *view = ui.junksView;
	proxy_junks_ = new JunksSortFilterProxyModel(view);
	proxy_junks_->setSourceModel(junks_model_);
	proxy_junks_->setDynamicSortFilter(true);
	proxy_junks_->setFilterKeyColumn(1);
	view->setModel(proxy_junks_);
	view->selectionModel()->setModel(proxy_junks_);
	view->header()->setSortIndicator(-1, Qt::AscendingOrder);
	view->setSortingEnabled(true);
	view->viewport()->installEventFilter(this);
	view->installEventFilter(this);
	view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	view->setColumnWidth(JUNKS.name, 250);
	view->setColumnWidth(JUNKS.path, 400);
	
	connect(ui.scanBtn, &QPushButton::clicked, this, [&] {
		ClearItemModelData(junks_model_);
		ui.scanBtn->setEnabled(false);
		if (!scanjunks_thread_) scanjunks_thread_ = new ScanJunksThread();
		connect(scanjunks_thread_, SIGNAL(appendJunks(QList<JunkItem>)), this, SLOT(onAppendJunkfiles(QList<JunkItem>)));
		connect(scanjunks_thread_, &QThread::finished, this, [&] {ui.scanBtn->setEnabled(true); });
		scanjunks_thread_->start(QThread::NormalPriority);
	});

	connect(ui.cleanBtn, &QPushButton::clicked, this, [&] {
		ui.cleanBtn->setEnabled(false);
		if (!cleanjunks_thread_) cleanjunks_thread_ = new CleanJunksThread();
		QStringList junks;
		int rows = junks_model_->rowCount();
		for (int i = 0; i < rows; i++) {
			QString qstr;
			qstr = junks_model_->index(i, JUNKS.path).data(Qt::DisplayRole).toString();
			junks.push_back(qstr);
		}
		cleanjunks_thread_->setJunks(junks);
		connect(cleanjunks_thread_, &QThread::finished, this, [&] {ui.cleanBtn->setEnabled(true); ClearItemModelData(junks_model_); });
		cleanjunks_thread_->start(QThread::NormalPriority);
	});
}

void Utilities::InitSystemToolsView()
{
	connect(ui.cmdBtn, &QPushButton::clicked, [] {ShellRun("cmd.exe", "/k cd /d %userprofile%"); });
	connect(ui.wslBtn, &QPushButton::clicked, [] {ShellRun("wsl.exe", ""); });
	connect(ui.powershellBtn, &QPushButton::clicked, [] {ShellRun("powershell.exe", ""); });
	connect(ui.calcBtn, &QPushButton::clicked, [] {ShellRun("calc.exe", ""); });
	connect(ui.regeditBtn, &QPushButton::clicked, [] {ShellRun("regedit.exe", ""); });
	connect(ui.servicesBtn, &QPushButton::clicked, [] {ShellRun("services.msc", ""); });
	connect(ui.taskmgrBtn, &QPushButton::clicked, [] {ShellRun("taskmgr.exe", ""); });
	connect(ui.programsBtn, &QPushButton::clicked, [] {ShellRun("control.exe", "appwiz.cpl"); });
	connect(ui.envBtn, &QPushButton::clicked, [] {ShellRun("SystemPropertiesAdvanced.exe", ""); });
	connect(ui.pcnameBtn, &QPushButton::clicked, [] {ShellRun("SystemPropertiesComputerName.exe", ""); });

	connect(ui.sysinfoBtn, &QPushButton::clicked, [] {ShellRun("cmd.exe", "/c systeminfo |more & pause"); });
	connect(ui.datetimeBtn, &QPushButton::clicked, [] {ShellRun("control.exe", "date/time"); });
	connect(ui.tasksBtn, &QPushButton::clicked, [] {ShellRun("taskschd.msc", "/s"); });
	connect(ui.versionBtn, &QPushButton::clicked, [] {ShellRun("winver.exe", ""); });
	connect(ui.deskiconsBtn, &QPushButton::clicked, [] {ShellRun("rundll32.exe", "shell32.dll,Control_RunDLL desk.cpl,,0"); });
	connect(ui.wallpaperBtn, &QPushButton::clicked, [] {
		if (UNONE::OsMajorVer() <= 5) ShellRun("rundll32.exe", "shell32.dll,Control_RunDLL desk.cpl,,0");
		else ShellRun("control.exe", "/name Microsoft.Personalization /page pageWallpaper"); 
	});
	connect(ui.devmgrBtn, &QPushButton::clicked, [] {ShellRun("devmgmt.msc", ""); });
	connect(ui.diskmgrBtn, &QPushButton::clicked, [] {ShellRun("diskmgmt.msc", ""); });

	connect(ui.resmonBtn, &QPushButton::clicked, [] {ShellRun("resmon.exe", ""); });
	connect(ui.perfBtn, &QPushButton::clicked, [] {ShellRun("perfmon.exe", ""); });
	connect(ui.perfsetBtn, &QPushButton::clicked, [] {ShellRun("SystemPropertiesPerformance.exe", ""); });
	connect(ui.powerBtn, &QPushButton::clicked, [] {ShellRun("control.exe", "powercfg.cpl,,3"); });
	connect(ui.usersBtn, &QPushButton::clicked, [] {ShellRun("lusrmgr.msc", ""); });
	connect(ui.uacBtn, &QPushButton::clicked, [] {ShellRun("UserAccountControlSettings.exe", ""); });
	connect(ui.evtBtn, &QPushButton::clicked, [] {ShellRun("eventvwr.msc", ""); });
	connect(ui.gpoBtn, &QPushButton::clicked, [] {ShellRun("gpedit.msc", ""); });
	connect(ui.secpolBtn, &QPushButton::clicked, [] {ShellRun("secpol.msc", ""); });
	connect(ui.certBtn, &QPushButton::clicked, [] {ShellRun("certmgr.msc", ""); });
	connect(ui.credBtn, &QPushButton::clicked, [] {ShellRun("control.exe", "/name Microsoft.CredentialManager"); });


	connect(ui.firewallBtn, &QPushButton::clicked, [] {ShellRun("control.exe", "firewall.cpl"); });
	connect(ui.proxyBtn, &QPushButton::clicked, [] {ShellRun("rundll32.exe", "shell32.dll,Control_RunDLL inetcpl.cpl,,4"); });
	connect(ui.netconnBtn, &QPushButton::clicked, [] {ShellRun("control.exe", "ncpa.cpl"); });
	connect(ui.hostsBtn, &QPushButton::clicked, [&] {ShellRun("notepad.exe", WStrToQ(UNONE::OsSystem32DirW() + L"\\drivers\\etc\\hosts")); });
	connect(ui.ipv4Btn, &QPushButton::clicked, [] {ShellRun("cmd.exe", "/c ipconfig|findstr /i ipv4 & pause"); });
	connect(ui.ipv6Btn, &QPushButton::clicked, [] {ShellRun("cmd.exe", "/c ipconfig|findstr /i ipv6 & pause"); });
	connect(ui.routeBtn, &QPushButton::clicked, [] {ShellRun("cmd.exe", "/c route print & pause"); });
	connect(ui.sharedBtn, &QPushButton::clicked, [] {ShellRun("fsmgmt.msc", ""); });
}