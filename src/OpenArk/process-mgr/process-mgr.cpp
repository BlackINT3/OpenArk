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
#include "process-mgr.h"
#include "../common/common.h"
#include "../openark/openark.h"
#include "process-properties.h"
#include "process-selection.h"

#define CMOD_NAME		0
#define CMOD_BASE		1
#define CMOD_SIZE		2
#define CMOD_PATH		3

ProcSortFilterProxyModel::ProcSortFilterProxyModel(QWidget *parent)
	: QSortFilterProxyModel(parent)
{}

ProcSortFilterProxyModel::~ProcSortFilterProxyModel()
{}

bool ProcSortFilterProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
	if (!left.isValid() || !right.isValid())
		return false;
	auto s1 = sourceModel()->data(left);
	auto s2 = sourceModel()->data(right);
	if ((left.column() == 1 || left.column() == 2)) {
		return s1.toUInt() < s2.toUInt();
	}
	return QString::compare(s1.toString(), s2.toString(), Qt::CaseInsensitive) < 0;
}

ProcessMgr::ProcessMgr(QWidget* parent) :
	parent_((OpenArk*)parent),
	cntproc_lable_(nullptr),
	proxy_model_(nullptr)
{
	ui.setupUi(this);
	ui.splitter->setStretchFactor(0, 2);
	ui.splitter->setStretchFactor(1, 1);

	//process list
	auto copy_menu_ = new QMenu();
	copy_menu_->addAction(tr("Process Name"))->setData(0);
	copy_menu_->addAction(tr("Process ID"))->setData(1);
	copy_menu_->addAction(tr("Parent ID"))->setData(2);
	copy_menu_->addAction(tr("Process Path"))->setData(3);
	copy_menu_->addAction(tr("Created Time"))->setData(4);
	copy_menu_->setTitle(tr("Copy"));
	connect(copy_menu_, SIGNAL(triggered(QAction*)), SLOT(onCopyActionTriggerd(QAction*)));

	auto dump_menu_ = new QMenu();
	dump_menu_->addAction(tr("Create Minidump..."), this, SLOT(onCreateMiniDump()));
	dump_menu_->addAction(tr("Create Fulldump..."), this, SLOT(onCreateFullDump()));
	dump_menu_->setTitle(tr("Create Dump"));
	proc_menu_ = new QMenu();
	proc_menu_->addAction(tr("Refresh"), this, SLOT(onRefresh()), QKeySequence::Refresh);
	proc_menu_->addAction(copy_menu_->menuAction());
	proc_menu_->addAction(tr("Kill Process"), this, SLOT(onKillProcess()), QKeySequence::Delete);
	proc_menu_->addAction(tr("Kill Process Tree"), this, SLOT(onKillProcessTree()), QKeySequence("SHIFT+Delete"));
	//proc_menu_->addAction(tr("Suspend"), this, SLOT(onSuspendProcess()));
	proc_menu_->addAction(tr("Select PID"), this, SLOT(onSelectPid()), QKeySequence("CTRL+G"));
	proc_menu_->addAction(tr("Explore File"), this, SLOT(onExploreFile()), QKeySequence("CTRL+L"));
	proc_menu_->addAction(tr("Enum Thread"), this, SLOT(onEnumThread()));
	proc_menu_->addAction(tr("Enum Window"), this, SLOT(onEnumWindow()));
	proc_menu_->addAction(tr("Inject Dll"), this, SLOT(onInjectDll()), QKeySequence("CTRL+J"));
	proc_menu_->addAction(tr("Sento Scanner"), this, SLOT(onSendtoScanner()));
	proc_menu_->addAction(dump_menu_->menuAction());
	proc_menu_->addAction(tr("Properties..."), this, SLOT(onShowProperties()), QKeySequence("CTRL+P"));

	proc_model_ = new QStandardItemModel;
	proc_model_->setHorizontalHeaderLabels(QStringList() << tr("Process") << tr("PID") << tr("PPID") << tr("Path") << tr("CreatedTime"));
	QTreeView *processView = ui.processView;
	proxy_model_ = new ProcSortFilterProxyModel(processView);
	proxy_model_->setSourceModel(proc_model_);
	proxy_model_->setDynamicSortFilter(true);
	proxy_model_->setFilterKeyColumn(1);
	processView->setModel(proxy_model_);
	processView->selectionModel()->setModel(proxy_model_);
	processView->header()->setSortIndicator(-1, Qt::AscendingOrder);
	processView->setSortingEnabled(true);
	processView->viewport()->installEventFilter(this);
	processView->installEventFilter(this);
	processView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ShowProcessList();

	mod_menu_ = new QMenu();
	mod_menu_->addAction(tr("Refresh"), this, SLOT(onRefresh()));
	mod_menu_->addAction(tr("Explore File"), this, SLOT(onExploreFile()));
	mod_menu_->addAction(tr("Sento Scanner"), this, SLOT(onSendtoScanner()));
	mod_model_ = new QStandardItemModel;
	QTreeView *moduleView = ui.moduleView;
	mod_model_->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Base") << tr("Size") << tr("Path"));
	SetDefaultTreeViewStyle(moduleView, mod_model_);
	moduleView->header()->setSortIndicator(-1, Qt::AscendingOrder);
	moduleView->viewport()->installEventFilter(this);
	moduleView->installEventFilter(this);

	connect(processView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(onProcDoubleClicked(const QModelIndex&)));
	connect(processView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &ProcessMgr::onProcChanged);
	connect(this, SIGNAL(signalOpen(QString)), parent_, SLOT(onOpen(QString)));

	connect(&timer_, SIGNAL(timeout()), this, SLOT(onTimer()));
	timer_.setInterval(1000);
	timer_.start();
}

ProcessMgr::~ProcessMgr()
{
	timer_.stop();
}

bool ProcessMgr::eventFilter(QObject *obj, QEvent *e)
{
	bool filtered = false;
	if (obj == ui.processView->viewport()) {
		if (e->type() == QEvent::ContextMenu) {
			QContextMenuEvent* ctxevt = dynamic_cast<QContextMenuEvent*>(e);
			if (ctxevt != nullptr) {
				proc_menu_->move(ctxevt->globalPos());
				proc_menu_->show();
			}
		}
	} else if (obj == ui.moduleView->viewport()) {
		if (e->type() == QEvent::ContextMenu) {
			QContextMenuEvent* ctxevt = dynamic_cast<QContextMenuEvent*>(e);
			if (ctxevt != nullptr) {
				mod_menu_->move(ctxevt->globalPos());
				mod_menu_->show();
			}
		}
	} else if (obj == ui.processView) {
		if (e->type() == QEvent::KeyPress) {
			filtered = true;
			QKeyEvent *keyevt = dynamic_cast<QKeyEvent*>(e);
			if (keyevt->matches(QKeySequence::Refresh)) {
				onRefresh();
			}	else if (keyevt->matches(QKeySequence::Delete)) {
				onKillProcess();
			} else if (keyevt->key() == Qt::Key_Delete && keyevt->modifiers() == Qt::ShiftModifier) {
				onKillProcessTree();
			} else if (keyevt->key() == Qt::Key_L && keyevt->modifiers() == Qt::ControlModifier) {
				onExploreFile();
			}	else if (keyevt->key() == Qt::Key_J && keyevt->modifiers() == Qt::ControlModifier) {
				onInjectDll();
			}	else if (keyevt->key() == Qt::Key_G && keyevt->modifiers() == Qt::ControlModifier) {
				onSelectPid();
			} else if (keyevt->key() == Qt::Key_P && keyevt->modifiers() == Qt::ControlModifier) {
				onShowProperties();
			} else {
				filtered = false;
			}
		}
	}
	if (filtered) {
		dynamic_cast<QKeyEvent*>(e)->ignore();
		return true;
	}
	return QWidget::eventFilter(obj, e);
}

void ProcessMgr::onOpenFile(const QString& file)
{
	
}

void ProcessMgr::onRefresh()
{
	auto sender = QObject::sender();
	if (IsContainAction(proc_menu_, sender)) {
		ShowProcessList();
		return;
	}
	if (IsContainAction(mod_menu_, sender)) {
		onShowModules();
		return;
	}
	if (sender == nullptr) {
		ShowProcessList();
		return;
	}
}

void ProcessMgr::onReset()
{
	//proxy_model_->setSortRole(Qt::InitialSortOrderRole);
	//proxy_model_->invalidate();
	ui.processView->header()->setSortIndicator(-1, Qt::AscendingOrder);
	ui.moduleView->header()->setSortIndicator(-1, Qt::AscendingOrder);
	onRefresh();
}

void ProcessMgr::onTimer()
{
	//ShowProcessList();
	PERFORMANCE_INFORMATION perf = { 0 };
	GetPerformanceInfo(&perf, sizeof(perf));
	if (cntproc_lable_ == nullptr) {
		cpu_percent_lable_ = new QLabel(); parent_->StatusBarAdd(cpu_percent_lable_);
		mm_percent_lable_ = new QLabel(); parent_->StatusBarAdd(mm_percent_lable_);
		cntproc_lable_ = new QLabel(); parent_->StatusBarAdd(cntproc_lable_);
		cntthread_lable_ = new QLabel(); parent_->StatusBarAdd(cntthread_lable_);
		cnthandle_lable_ = new QLabel(); parent_->StatusBarAdd(cnthandle_lable_);
	}
	cpu_percent_lable_->setText(WStrToQ(UNONE::StrFormatW(L"CPU:%0.2f%%", GetSystemUsageOfCPU())));
	mm_percent_lable_->setText(WStrToQ(UNONE::StrFormatW(L"Memory:%0.2f%%", GetSystemUsageOfMemory())));
	cntproc_lable_->setText(QString("Processes:%1").arg(perf.ProcessCount));
	cntthread_lable_->setText(QString("Threads:%1").arg(perf.ThreadCount));
	cnthandle_lable_->setText(QString("Handles:%1").arg(perf.HandleCount));
}

void ProcessMgr::onCopyActionTriggerd(QAction* action)
{
	auto idx = action->data().toInt();
	QString data = ProcCurViewItemData(idx);
	if (idx == 0) data.replace(" *32", "");
	ClipboardCopyData(data.toStdString());
}

void ProcessMgr::onKillProcess()
{
	UNONE::PsKillProcess(ProcCurPid());
	onRefresh();
}

void ProcessMgr::onKillProcessTree()
{
	QString tips;
	auto pid = ProcCurPid();
	auto pids = UNONE::PsGetDescendantPids(pid);
	pids.push_back(pid);
	tips.append("Do you kill all processes?\n-------------------\n");
	for (auto d : pids) {
		std::wstring wstr = UNONE::StrFormatW(L"[%d] %s", d, UNONE::PsGetProcessNameW(d).c_str());
		tips.append(WStrToQ(wstr));
		tips.append("\n");
	}
	QMessageBox::StandardButton reply;
	reply = QMessageBox::question(this, tr("Warning"), tips, QMessageBox::Yes | QMessageBox::No);
	if (reply == QMessageBox::Yes) {
		for (auto d : pids) { UNONE::PsKillProcess(d); };
		onRefresh();
	}
}

void ProcessMgr::onSuspendProcess()
{

}

void ProcessMgr::onInjectDll()
{
	QString file = QFileDialog::getOpenFileName(this, tr("Select Dll"), "", tr("Dll Files (*.dll);;All Files (*.*)"));
	if (file.isEmpty()) return;
	std::wstring dll = file.toStdWString();
	std::string buf;
	UNONE::FsReadFileDataW(dll, buf);
	if (buf.empty() || !UNONE::PeValid((CHAR*)buf.c_str())) {
		MsgBoxError(tr("Dll file invalid."));
		return;
	}
	auto pid = ProcCurPid();
	bool is_dll64 = UNONE::PeX64((CHAR*)buf.c_str());
	bool is_exe64 = UNONE::PsIsX64(pid);
	if ((!is_dll64 && is_exe64)) {
		MsgBoxError(tr("Can't inject 32-bit dll to 64-bit process."));
		return;
	}
	if ((is_dll64 && !is_exe64)) {
		MsgBoxWarn(tr("Inject 64-bit dll to 32-bit process, maybe fail."));
	}
	auto thd = UNONE::PsInjectByRemoteThreadW(pid, dll);
	if (thd) {
		MsgBoxInfo(tr("Inject ok."));
	} else {
		MsgBoxError(tr("Inject failed."));
	}
	CloseHandle(thd);
}

void ProcessMgr::onSelectPid()
{
	auto dlg = new ProcessSelection(this);
	dlg->setObjectName("ProcessSelection");
	dlg->raise();
	dlg->show();
}

void ProcessMgr::onExploreFile()
{
	auto sender = QObject::sender();
	if (IsContainAction(proc_menu_, sender)) {
		ExploreFile(ProcCurViewItemData(3));
		return;
	}
	if (IsContainAction(mod_menu_, sender)) {
		ExploreFile(ModCurViewItemData(3));
		return;
	}
	if (sender == nullptr) {
		ExploreFile(ProcCurViewItemData(3));
		return;
	}
}

void ProcessMgr::onEnumThread()
{
	ShowProperties(ProcCurPid(), 1);
}

void ProcessMgr::onEnumWindow()
{
	ShowProperties(ProcCurPid(), 2);
}

void ProcessMgr::onCreateMiniDump()
{
	DWORD pid = ProcCurPid();
	QString name = WStrToQ(UNONE::PsGetProcessNameW(pid)).replace(".exe", ".dmp", Qt::CaseInsensitive);
	QString dmp = QFileDialog::getSaveFileName(this, tr("Save dump file"), name, tr("DumpFile(*.dmp)"));
	if (dmp.isEmpty()) return;
	CreateDump(pid, dmp.toStdWString(), true);
}

void ProcessMgr::onCreateFullDump()
{
	DWORD pid = ProcCurPid();
	QString name = WStrToQ(UNONE::PsGetProcessNameW(pid)).replace(".exe", ".dmp", Qt::CaseInsensitive);
	QString dmp = QFileDialog::getSaveFileName(this, tr("Save dump file"), name, tr("DumpFile(*.dmp)"));
	if (dmp.isEmpty()) return;
	CreateDump(pid, dmp.toStdWString(), false);
}

void ProcessMgr::onSendtoScanner()
{
	QString path;
	auto action = qobject_cast<QAction*>(QObject::sender());
	if (IsContainAction(proc_menu_, action)) {
		path = ProcCurViewItemData(3);
	} else if (IsContainAction(mod_menu_, action)) {
		path = ModCurViewItemData(3);
	}
	parent_->ActivateTab(2);
	emit signalOpen(path);
}

void ProcessMgr::onShowProperties()
{
	ShowProperties(ProcCurPid(), 0);
}

void ProcessMgr::onShowModules()
{
	ClearItemModelData(mod_model_, 0);
	DWORD pid = ProcCurPid();
	UNONE::PsEnumModule(pid, [&](MODULEENTRY32W& entry)->bool{
		QString modname = WCharsToQ(entry.szModule);
		QString modpath = WCharsToQ(entry.szExePath);
		ULONG64 modbase = (ULONG64)entry.modBaseAddr;
		ULONG64 modsize = entry.modBaseSize;
		QStandardItem *item0 = new QStandardItem(LoadIcon(modpath), modname);
		QStandardItem *item1 = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"0x%llX", modbase)));
		QStandardItem *item2 = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"0x%llX", modsize)));
		QStandardItem *item3 = new QStandardItem(modpath);
		auto count = mod_model_->rowCount();
		for (int i = 0; i < count; i++) {
			auto base = mod_model_->data(mod_model_->index(i, CMOD_BASE)).toString().toStdWString();
			if (UNONE::StrToHex64W(base) == modbase) {
				return true;
			}
		}
		mod_model_->setItem(count, CMOD_NAME, item0);
		mod_model_->setItem(count, CMOD_BASE, item1);
		mod_model_->setItem(count, CMOD_SIZE, item2);
		mod_model_->setItem(count, CMOD_PATH, item3);
		return true;
	});

	auto view = ui.moduleView;
	view->setColumnWidth(CMOD_NAME, 180);
	view->resizeColumnToContents(CMOD_BASE);
	view->resizeColumnToContents(CMOD_SIZE);
	view->setColumnWidth(CMOD_PATH, 180);
}

void ProcessMgr::onProcDoubleClicked( const QModelIndex &idx )
{
	onShowProperties();
}

void ProcessMgr::onProcChanged( const QModelIndex &current, const QModelIndex &previous )
{
	onShowModules();
}

void ProcessMgr::onProcSelection(QString pid)
{
	auto view = ui.processView;
	int rows = proc_model_->rowCount();
	for (int i = 0; i < rows; i++) {
		const QModelIndex &idx = view->currentIndex().sibling(i, 1);
		auto qstr = idx.data().toString();
		if (qstr == pid) {
			view->selectionModel()->clearSelection();
			view->selectionModel()->select(idx, QItemSelectionModel::Select | QItemSelectionModel::Rows);
			view->scrollTo(idx);
		}
	}
}

bool ProcessMgr::GetProcCache(unsigned int pid, ProcInfo& info)
{
	QMutexLocker locker(&mutex_);
	if (proc_caches_.isEmpty()) return false;
	if (proc_caches_.contains(pid)) {
		auto it = proc_caches_.find(pid);
		info = it.value();
		return true;
	}
	return false;
}

void ProcessMgr::SetProcCache(unsigned int pid, ProcInfo& info)
{
	QMutexLocker locker(&mutex_);
	proc_caches_.insert(pid, info);
}

DWORD ProcessMgr::ProcCurPid()
{
	auto idx = ui.processView->currentIndex();
	DWORD pid = idx.sibling(idx.row(), 1).data().toString().toUInt();
	return pid;
}

int ProcessMgr::ProcCurRow()
{
	auto idx = ui.processView->currentIndex();
	return idx.row();
}

int ProcessMgr::ProcCurCol()
{
	auto idx = ui.processView->currentIndex();
	return idx.column();
}

QString ProcessMgr::ProcCurViewItemData(int column)
{
	return GetCurItemViewData(ui.processView, column);
}

QString ProcessMgr::ProcViewItemData(int row, int column)
{
	return GetItemViewData(ui.processView, row, column);
}

QString ProcessMgr::ModCurViewItemData(int column)
{
	return GetCurItemViewData(ui.moduleView, column);
}

QString ProcessMgr::ModViewItemData(int row, int column)
{
	return GetItemViewData(ui.moduleView, row, column);
}

void ProcessMgr::ShowProperties(DWORD pid, int tab)
{
	auto properties = new ProcessProperties(this->parent_, pid, tab);
	properties->setObjectName("ProcessProperties");
	properties->raise();
	properties->show();
}

void ProcessMgr::ShowProcessList()
{
	ClearItemModelData(proc_model_);

	std::vector<ProcInfo> pis;
	UNONE::PsEnumProcess([&pis](PROCESSENTRY32W& entry)->bool {
		ProcInfo pi;
		pi.pid = entry.th32ProcessID;
		pi.ppid = entry.th32ParentProcessID;
		pi.name = WCharsToQ(entry.szExeFile);
		pis.push_back(pi);
		return true;
	});
	for (const auto& pi : pis) {
		ProcInfo info;
		if (!GetProcCache(pi.pid, info)) {
			static bool is_os64 = UNONE::OsIs64();
			info.pid = pi.pid;
			info.ppid = pi.ppid;
			info.path = WStrToQ(UNONE::PsGetProcessPathW(pi.pid));
			info.name = pi.name;
			info.ctime = QString::fromStdWString(ProcessCreateTime(pi.pid));
			if (is_os64 && !UNONE::PsIsX64(pi.pid))	info.name.append(" *32");
			SetProcCache(pi.pid, info);
		}	
		QStandardItem *name_item = new QStandardItem(LoadIcon(info.path), info.name);
		QStandardItem *pid_item = new QStandardItem(QString("%1").arg(info.pid));
		QStandardItem *ppid_item = new QStandardItem(QString("%1").arg(info.ppid));
		QStandardItem *path_item = new QStandardItem(info.path);
		QStandardItem *ctime_item = new QStandardItem(info.ctime);
		auto count = proc_model_->rowCount();
		proc_model_->setItem(count, 0, name_item);
		proc_model_->setItem(count, 1, pid_item);
		proc_model_->setItem(count, 2, ppid_item);
		proc_model_->setItem(count, 3, path_item);
		proc_model_->setItem(count, 4, ctime_item);
	}

	auto view = ui.processView;
	view->setColumnWidth(0, 180);
	view->resizeColumnToContents(1);
	view->resizeColumnToContents(2);
	view->setColumnWidth(3, 530);

	QModelIndexList selected = view->selectionModel()->selectedIndexes();
	if (!selected.isEmpty()) {
		const QModelIndex& idx = selected[0];
		view->selectionModel()->select(idx, QItemSelectionModel::Select | QItemSelectionModel::Rows);
		view->scrollTo(idx);
	}
}