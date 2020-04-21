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

#define MODEL_STRING(model, row, columm) (model->index(row, columm).data(Qt::DisplayRole).toString())
#define RECYCLEBIN "RecycleBin"

struct {
	int s = 0;
	int dir = s++;
	int filecnt = s++;
	int detail = s++;
	int sumsize = s++;
} JUNKS;
bool JunksSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
	auto s1 = sourceModel()->data(left); auto s2 = sourceModel()->data(right);
	auto column = left.column();
	if ((column == JUNKS.filecnt || column == JUNKS.sumsize)) return s1.toUInt() < s2.toUInt();
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
	connect(ui.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onTabChanged(int)));

	qRegisterMetaType<JunkCluster>("JunkCluster");

	InitCleanerView();
	InitSystemToolsView();
}

Utilities::~Utilities()
{
	if (scanjunks_thread_) scanjunks_thread_->terminate();
	scanjunks_thread_ = nullptr;
}

void Utilities::RecordAppServer(const QString &svr)
{
	app_server_ = svr;
}

void Utilities::onTabChanged(int index)
{
	OpenArkConfig::Instance()->SetPrefLevel2Tab(index);
}

void Utilities::onOpJunkfiles(int op, JunkCluster cluster)
{
	bool existed = false;
	int rows = junks_model_->rowCount();
	int seq = rows;
	for (int i = 0; i < rows; i++) {
		if (cluster.dir == MODEL_STRING(junks_model_, i, JUNKS.dir)) {
			seq = i;
			existed = true;
			break;
		}
	}
	
	auto &items = cluster.items;
	DWORD64 sumsize = cluster.sumsize;
	DWORD filecnt = items.size();
	if (existed) {
		if (op == 0) {
			filecnt += MODEL_STRING(junks_model_, seq, JUNKS.filecnt).toULong();
			sumsize += MODEL_STRING(junks_model_, seq, JUNKS.sumsize).toULongLong();
		}	else {
			filecnt = MODEL_STRING(junks_model_, seq, JUNKS.filecnt).toULong() - filecnt;
			sumsize = MODEL_STRING(junks_model_, seq, JUNKS.sumsize).toULongLong() - sumsize;
		}
	}

	QStandardItem *dir_item;
	if (cluster.dir == RECYCLEBIN) {
		dir_item = new QStandardItem(QIcon(":/OpenArk/systools/recyclebin.png"), cluster.dir);
	} else {
		dir_item = new QStandardItem(LoadIcon(cluster.dir), cluster.dir);
	}
	dir_item->setCheckable(true);
	dir_item->setCheckState(Qt::Checked);
	QStandardItem *filecnt_item = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"%d", filecnt)));
	QStandardItem *sumsize_item = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"%lld B", sumsize)));
	QStandardItem *detail_item;
	if (sumsize > GB) {
		detail_item = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"%.2f GB", (double)sumsize / GB)));
	} else if (sumsize > MB) {
		detail_item = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"%.2f MB", (double)sumsize / MB)));
	} else if (sumsize > KB) {
		detail_item = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"%.2f KB", (double)sumsize/KB)));
	} else {
		detail_item = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"%lld Bytes", sumsize)));
	}
	junks_model_->setItem(seq, JUNKS.dir, dir_item);
	junks_model_->setItem(seq, JUNKS.filecnt, filecnt_item);
	junks_model_->setItem(seq, JUNKS.sumsize, sumsize_item);
	junks_model_->setItem(seq, JUNKS.detail, detail_item);

	if (items.size() > 0) {
		auto path = items.first().path;
		int maxcnt = 65;
		int ellipsis = 5;
		if (path.size() > maxcnt) {
			int s1 = (maxcnt - ellipsis) / 2;
			auto p1 = path.mid(0, s1);
			auto p2 = QString(".").repeated(ellipsis);
			auto p3 = path.right(s1);
			path = p1 + p2 + p3;
		}
		ui.fileLabel->setText(path);
	}
}

void Utilities::onAppendJunkfiles(JunkCluster cluster)
{
	onOpJunkfiles(0, cluster);
}

void Utilities::onCleanJunkfiles(JunkCluster cluster)
{
	onOpJunkfiles(1, cluster);
}

void ScanJunksThread::run()
{
	QList<JunkItem> items;
	auto SendToUI = [&](QString dir, DWORD64 sumsize = 0) {
		JunkCluster cluster;
		if (!sumsize) {
			for (auto &item : items) { sumsize += item.size; }
		}
		cluster.dir = dir;
		cluster.sumsize = sumsize;
		cluster.items = items;
		emit appendJunks(cluster);
		bool existed = false;
		for (auto &c : junks_cluster_) {
			if (cluster.dir == c.dir) {
				c.sumsize = sumsize;
				c.items.append(items);
				existed = true;
			}
		}
		if (!existed)junks_cluster_.push_back(cluster);
		items.clear();
	};
	std::function<bool(wchar_t*, wchar_t*, void*)> ScanCallback;
	ScanCallback = [&](wchar_t* path, wchar_t* name, void* param)->bool {
		if (UNONE::FsIsDirW(path)) {
			UNONE::FsEnumDirectoryW(path, ScanCallback, param);
		}
		JunkItem item;
		item.name = WCharsToQ(name);
		item.path = WCharsToQ(path);
		DWORD64 fsize = 0;
		UNONE::FsGetFileSizeW(path, fsize);
		item.size = fsize;
		items.push_back(item);
		if (items.size() >= 50) {
			SendToUI(*(QString*)param);
		}
		return true;
	};

	std::function<bool(wchar_t*, wchar_t*, void*)> ScanCallbackCustom;

	QStringList clear_suffixes_list = custom_suffex_.split(QRegExp("[,;]"));
	ScanCallbackCustom = [&](wchar_t* path, wchar_t* name, void* param)->bool {
		if (UNONE::FsIsDirW(path)) {
			UNONE::FsEnumDirectoryW(path, ScanCallbackCustom, param);
		}
		QFileInfo file_info = QFileInfo(WCharsToQ(path));
		QString suffix = "." + file_info.suffix();
		if (custom_suffex_.isEmpty() || clear_suffixes_list.contains(suffix, Qt::CaseInsensitive)){
			JunkItem item;
			item.name = WCharsToQ(name);
			item.path = WCharsToQ(path);
			DWORD64 fsize = 0;
			UNONE::FsGetFileSizeW(path, fsize);
			item.size = fsize;
			items.push_back(item);
			SendToUI(*(QString*)param);
		}
		return true;
	};
	junks_cluster_.clear();
	if (is_custom_scan_) {
		for (int i = 0; i < custom_path_.size(); i++) {
			SendToUI(custom_path_[i]);
			UNONE::FsEnumDirectoryW(custom_path_[i].toStdWString(), ScanCallbackCustom, &custom_path_[i]);
		}
	}
	if (is_builtin_scan_) {
		auto &&junkdirs = OpenArkConfig::Instance()->GetJunkDirs();
		for (auto &dir : junkdirs) {
			if (!UNONE::FsIsExistedW(dir.toStdWString())) continue;
			UNONE::FsEnumDirectoryW(dir.toStdWString(), ScanCallback, &dir);
			if (items.size() >= 0) {
				SendToUI(dir);
			}
		}
		// RecycleBin
		SHQUERYRBINFO bi;
		bi.cbSize = sizeof(SHQUERYRBINFO);
		HRESULT hr = SHQueryRecycleBin(NULL, &bi);
		if (hr == S_OK) {
			items.clear();
			auto nums = bi.i64NumItems;
			while (nums--) {
				JunkItem junk;
				items.append(junk);
			}
			SendToUI(RECYCLEBIN, bi.i64Size);
		}
	}
}

void CleanJunksThread::run()
{
	for (auto &cluster : junks_cluster_) {
		int cnt = 0;
		JunkCluster c;
		c.dir = cluster.dir;
		c.sumsize = 0;
		if (cluster.dir == RECYCLEBIN) {
			auto flags = SHERB_NOCONFIRMATION | SHERB_NOPROGRESSUI | SHERB_NOSOUND;
			SHEmptyRecycleBin(NULL, NULL, flags);
			continue;
		}
		for (auto &item : cluster.items) {
			bool ret;
			auto &&path = item.path.toStdWString();
			if (UNONE::FsIsDirW(path)) {
				ret = UNONE::FsDeleteDirectoryW(path);
			} else {
				ret = DeleteFileW(path.c_str());
			}
			c.sumsize += item.size;
			c.items.append(item);
			if (c.items.size() > 199) {
				emit cleanJunks(c);
				c.sumsize = 0;
				c.items.clear();
			}
		}
		if (c.items.size() > 0) {
			emit cleanJunks(c);
			c.sumsize = 0;
			c.items.clear();
		}
	}
}

void Utilities::InitCleanerView()
{
	junks_model_ = new QStandardItemModel;
	junks_model_->setHorizontalHeaderLabels(QStringList() << tr("Directory") << tr("FileCount") << tr("Detail") << tr("SumSize"));
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
	view->setColumnWidth(JUNKS.dir, 700);
	view->setColumnWidth(JUNKS.filecnt, 100);
	view->setColumnWidth(JUNKS.sumsize, 170);
	connect(ui.scanBtn, &QPushButton::clicked, this, [&] {
		ClearItemModelData(junks_model_);
		ui.cleanBtn->setEnabled(false);
		ui.scanBtn->setEnabled(false);
		ui.statusLabel->setText(tr("[STATUS] Scanning..."));
		ui.statusLabel->setStyleSheet("color:purple");
		if (!scanjunks_thread_) {
			scanjunks_thread_ = new ScanJunksThread();
			connect(scanjunks_thread_, SIGNAL(appendJunks(JunkCluster)), this, SLOT(onAppendJunkfiles(JunkCluster)));
			connect(scanjunks_thread_, &QThread::finished, this, [&] {
				ui.cleanBtn->setEnabled(true);
				ui.scanBtn->setEnabled(true); 
				ui.statusLabel->setText(tr("[STATUS] Scan completed..."));
				ui.statusLabel->setStyleSheet("color:green");
			});
		}
		scanjunks_thread_->is_custom_scan_ = (ui.customScanCheckBox->checkState() == Qt::Checked);
		scanjunks_thread_->is_builtin_scan_ = (ui.builtinScanCheckBox->checkState() == Qt::Checked);
		scanjunks_thread_->custom_path_ = OpenArkConfig::Instance()->GetValue("clean_path_list").toStringList();
		scanjunks_thread_->custom_suffex_ = OpenArkConfig::Instance()->GetValue("clean_file_suffix").toString();
		scanjunks_thread_->start(QThread::NormalPriority);
	});

	connect(ui.cleanBtn, &QPushButton::clicked, this, [&] {
		ui.cleanBtn->setEnabled(false);
		ui.scanBtn->setEnabled(false);
		ui.statusLabel->setText(tr("[STATUS] Cleaning..."));
		ui.statusLabel->setStyleSheet("color:purple");
		if (!cleanjunks_thread_) {
			cleanjunks_thread_ = new CleanJunksThread();
			connect(cleanjunks_thread_, SIGNAL(cleanJunks(JunkCluster)), this, SLOT(onCleanJunkfiles(JunkCluster)));
			connect(cleanjunks_thread_, &QThread::finished, this, [&] {
				ui.cleanBtn->setEnabled(true); 
				ui.scanBtn->setEnabled(true);
				RemoveCleanerItems();
				ui.statusLabel->setText(tr("[STATUS] Clean completed..."));
				ui.statusLabel->setStyleSheet("color:green");
			});
		}
		removed_rows_.clear();
		QList<JunkCluster> clusters;
		int rows = junks_model_->rowCount();
		for (int i = 0; i < rows; i++) {
			auto stat = junks_model_->item(i, JUNKS.dir)->checkState();
			if (stat == Qt::Checked) {
				removed_rows_.push_back(i);
				for (auto &c : scanjunks_thread_->junks_cluster_) {
					if (c.dir == MODEL_STRING(junks_model_, i, JUNKS.dir)) {
						clusters.append(c);
						break;
					}
				}
			}
		}
		cleanjunks_thread_->setJunkCluster(clusters);
		cleanjunks_thread_->start(QThread::NormalPriority);
	});




}

bool PsKillProcess(__in DWORD pid)
{
	bool result = false;
	HANDLE phd = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
	if (phd) {
		if (TerminateProcess(phd, 1))
			result = true;
		CloseHandle(phd);
	}
	return result;

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
	connect(ui.fastrebootBtn, &QPushButton::clicked, [&] {
		if (QMessageBox::warning(this, tr("Warning"), tr("Are you sure to reboot?"), 
			QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
			OsFastReboot();
		}
	});
	connect(ui.fastpoweroffBtn, &QPushButton::clicked, [&] {
		if (QMessageBox::warning(this, tr("Warning"), tr("Are you sure to poweroff?"),
			QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
			OsFastPoweroff();
		}
	});
	connect(ui.resetexplorerBtn, &QPushButton::clicked, [&] {
		auto pid = OsGetExplorerPid();
		auto path = UNONE::OsWinDirW() + L"\\explorer.exe";
		if (pid != -1) {
			PsKillProcess(pid);
		}
		UNONE::PsCreateProcessW(path);
	});
	connect(ui.killexplorerBtn, &QPushButton::clicked, [&] {
		auto pid = OsGetExplorerPid();
		PsKillProcess(pid);
	});

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
	connect(ui.ipv4Btn, &QPushButton::clicked, [] {ShellRun("cmd.exe", "/k ipconfig|findstr /i ipv4"); });
	connect(ui.ipv6Btn, &QPushButton::clicked, [] {ShellRun("cmd.exe", "/k ipconfig|findstr /i ipv6"); });
	connect(ui.routeBtn, &QPushButton::clicked, [] {ShellRun("cmd.exe", "/k route print"); });
	connect(ui.sharedBtn, &QPushButton::clicked, [] {ShellRun("fsmgmt.msc", ""); });
}

void Utilities::RemoveCleanerItems()
{
	int delta = 0;
	for (auto r : removed_rows_) {
		junks_model_->removeRows(r - delta, 1);
		delta++;
	}
	removed_rows_.clear();
}