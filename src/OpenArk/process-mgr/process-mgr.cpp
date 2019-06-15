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

// ProcessView's header index
struct {
	int s = 0;
	int name = s++;
	int pid = s++;
	int ppid = s++;
	int path = s++;
	int desc = s++;
	int corp = s++;
	int ctime = s++;
} PDX;

// ModulesView's header index
struct {
	int name = 0;
	int base = 1;
	int size = 2;
	int path = 3;
	int desc = 4;
	int ver = 5;
	int corp = 6;
} MDX;

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
	proxy_model_(nullptr),
	proc_header_idx_(0)
{
	ui.setupUi(this);
	ui.splitter->setStretchFactor(0, 2);
	ui.splitter->setStretchFactor(1, 1);

	//process list
	auto copy_menu_ = new QMenu();
	copy_menu_->addAction(tr("Process Name"))->setData(PDX.name);
	copy_menu_->addAction(tr("Process ID"))->setData(PDX.pid);
	copy_menu_->addAction(tr("Parent ID"))->setData(PDX.ppid);
	copy_menu_->addAction(tr("Process Path"))->setData(PDX.path);
	copy_menu_->addAction(tr("Created Time"))->setData(PDX.ctime);
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
	proc_model_->setHorizontalHeaderLabels(QStringList() << tr("Process") << tr("PID") << tr("PPID") << tr("Path") << tr("Description") << tr("Company Name") << tr("CreatedTime") );
	QTreeView *pview = ui.processView;
	proxy_model_ = new ProcSortFilterProxyModel(pview);
	proxy_model_->setSourceModel(proc_model_);
	proxy_model_->setDynamicSortFilter(true);
	proxy_model_->setFilterKeyColumn(1);
	pview->setModel(proxy_model_);
	pview->selectionModel()->setModel(proxy_model_);
	pview->header()->setSortIndicator(-1, Qt::AscendingOrder);
	pview->setSortingEnabled(true);
	pview->viewport()->installEventFilter(this);
	pview->installEventFilter(this);
	pview->setMouseTracking(true);
	pview->setEditTriggers(QAbstractItemView::NoEditTriggers);
	pview->setExpandsOnDoubleClick(false);
	pview->setColumnWidth(PDX.name, 250);
	pview->setColumnWidth(PDX.path, 440);
	pview->setColumnWidth(PDX.desc, 190);
	pview->setColumnWidth(PDX.corp, 155);
	ShowProcessTree();

	mod_menu_ = new QMenu();
	mod_menu_->addAction(tr("Refresh"), this, SLOT(onRefresh()));
	mod_menu_->addAction(tr("Explore File"), this, SLOT(onExploreFile()));
	mod_menu_->addAction(tr("Sento Scanner"), this, SLOT(onSendtoScanner()));
	mod_model_ = new QStandardItemModel;
	QTreeView *mview = ui.moduleView;
	mod_model_->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Base") << tr("Size") << tr("Path"));
	SetDefaultTreeViewStyle(mview, mod_model_);
	mview->header()->setSortIndicator(-1, Qt::AscendingOrder);
	mview->viewport()->installEventFilter(this);
	mview->installEventFilter(this);

	connect(pview->header(), SIGNAL(sectionClicked(int)), this, SLOT(onProcSectionClicked(int)));
	connect(pview, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(onProcDoubleClicked(const QModelIndex&)));
	connect(pview->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &ProcessMgr::onProcChanged);
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
		} else if (e->type() == QEvent::MouseMove) {
			QMouseEvent *mouse = static_cast<QMouseEvent *>(e);
			QPoint pt = mouse->pos();
			if (pt.rx() <= ui.processView->columnWidth(0)) {
				QModelIndex idx = ui.processView->indexAt(pt);
				const QModelIndex &curidx = idx.sibling(idx.row(), 1);
				auto pid = curidx.data(Qt::DisplayRole).toInt();
				auto info = CacheGetProcessBaseInfo(pid);
				info.CommandLine = UNONE::StrInsertW(info.CommandLine, 120, L"\n    ");
				info.ImagePathName = UNONE::StrInsertW(info.ImagePathName, 120, L"\n    ");
				QString tips = QString("Command Line:\n    %1\nPath:\n    %2"
				).arg(WStrToQ(info.CommandLine)).arg(WStrToQ(info.ImagePathName));
				QToolTip::showText(mouse->globalPos(), tips);
				return true;
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
		if (proc_header_idx_ == 0) ShowProcessTree();
		else ShowProcessList();
		return;
	}
	if (IsContainAction(mod_menu_, sender)) {
		onShowModules();
		return;
	}
	if (sender == nullptr) {
		if (proc_header_idx_ == 0) ShowProcessTree();
		else ShowProcessList();
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
	tips.append(tr("Do you kill all processes?\n-------------------\n"));
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
		ExploreFile(ProcCurViewItemData(PDX.path));
		return;
	}
	if (IsContainAction(mod_menu_, sender)) {
		ExploreFile(ModCurViewItemData(MDX.path));
		return;
	}
	if (sender == nullptr) {
		ExploreFile(ProcCurViewItemData(PDX.path));
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
		path = ProcCurViewItemData(PDX.path);
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
			auto base = mod_model_->data(mod_model_->index(i, MDX.base)).toString().toStdWString();
			if (UNONE::StrToHex64W(base) == modbase) {
				return true;
			}
		}
		mod_model_->setItem(count, MDX.name, item0);
		mod_model_->setItem(count, MDX.base, item1);
		mod_model_->setItem(count, MDX.size, item2);
		mod_model_->setItem(count, MDX.path, item3);
		return true;
	});

	auto view = ui.moduleView;
	view->setColumnWidth(MDX.name, 180);
	view->resizeColumnToContents(MDX.base);
	view->resizeColumnToContents(MDX.size);
	view->setColumnWidth(MDX.path, 180);
}

void ProcessMgr::onProcSectionClicked(int idx)
{
	if (idx == 0) {
		proc_header_idx_++;
		switch (proc_header_idx_) {
		case 3:
			ui.processView->header()->setSortIndicator(-1, Qt::AscendingOrder);
			ShowProcessTree();
			proc_header_idx_ = 0;
			break;
		case 1:
			ShowProcessList();
		}
	}	else {
		if (proc_header_idx_ == 0) {
			ShowProcessList();
			proc_header_idx_ = 1;
		}
	}
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
		const QModelIndex &idx = view->currentIndex().sibling(i, PDX.pid);
		auto qstr = idx.data().toString();
		if (qstr == pid) {
			view->selectionModel()->clearSelection();
			view->selectionModel()->select(idx, QItemSelectionModel::Select | QItemSelectionModel::Rows);
			view->scrollTo(idx);
		}
	}
}

DWORD ProcessMgr::ProcCurPid()
{
	auto idx = ui.processView->currentIndex();
	DWORD pid = idx.sibling(idx.row(), PDX.pid).data().toString().toUInt();
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
	CacheRefreshProcInfo();

	std::vector<ProcInfo> pis;
	UNONE::PsEnumProcess([&pis](PROCESSENTRY32W& entry)->bool {
		ProcInfo info;
		auto pid = info.pid = entry.th32ProcessID;
		auto ppid = info.ppid = entry.th32ParentProcessID;
		info.name = WCharsToQ(entry.szExeFile);
		CacheGetProcInfo(pid, info);
		pis.push_back(info);
		return true;
	});
	for (const auto& pi : pis) {
		QStandardItem *name_item = new QStandardItem(pi.name);
		name_item->setBackground(QColor(240, 240, 240));
		AppendProcessItem(nullptr, name_item, pi, proc_model_->rowCount());
	}

	AjustProcessStyle();
}

void ProcessMgr::ShowProcessTree()
{
	ClearItemModelData(proc_model_);
	CacheRefreshProcInfo();

	std::function<void(QStandardItem *parent, ProcInfo pi, int seq)> AppendProcessTree =
	[&](QStandardItem *parent, ProcInfo pi, int seq) {
		QStandardItem *name_item = new QStandardItem(pi.name);
		name_item->setBackground(QColor(240, 240, 240));
		AppendProcessItem(parent, name_item, pi, seq);
		QVector<ProcInfo> childs;
		CacheGetProcChilds(pi.pid, childs);
		for (size_t i = 0; i < childs.size(); i++) {
			AppendProcessTree(name_item, childs[i], i);
		}
	};

	std::vector<ProcInfo> pis;
	UNONE::PsEnumProcess([&](PROCESSENTRY32W& entry)->bool {
		ProcInfo info;
		auto pid = info.pid = entry.th32ProcessID;
		auto ppid = info.ppid = entry.th32ParentProcessID;
		info.name = WCharsToQ(entry.szExeFile);
		CacheGetProcInfo(pid, info);
		if (ppid == 0 || UNONE::PsIsDeleted(ppid)) {
			pis.push_back(info);
		}
		return true;
	});

	for (const auto& pi : pis) {
		AppendProcessTree(nullptr, pi, proc_model_->rowCount());
	}

	AjustProcessStyle();
	ui.processView->expandAll();
}

void ProcessMgr::AppendProcessItem(QStandardItem *parent, QStandardItem *name_item, ProcInfo info, int seq)
{
	name_item->setIcon(LoadIcon(info.path));
	QStandardItem *pid_item = new QStandardItem(PidFormat(info.pid));
	QStandardItem *ppid_item = new QStandardItem(PidFormat(info.ppid));
	QStandardItem *desc_item = new QStandardItem(info.desc);
	QStandardItem *corp_item = new QStandardItem(info.corp);
	QStandardItem *ctime_item = new QStandardItem(info.ctime);
	QStandardItem *path_item = new QStandardItem(info.path);

	if (parent == nullptr) {
		proc_model_->setItem(seq, PDX.name, name_item);
		proc_model_->setItem(seq, PDX.pid, pid_item);
		proc_model_->setItem(seq, PDX.ppid, ppid_item);
		proc_model_->setItem(seq, PDX.path, path_item);
		proc_model_->setItem(seq, PDX.desc, desc_item);
		proc_model_->setItem(seq, PDX.corp, corp_item);
		proc_model_->setItem(seq, PDX.ctime, ctime_item);
		return;
	}

	parent->appendRow(name_item);
	parent->setChild(seq, PDX.pid, pid_item);
	parent->setChild(seq, PDX.ppid, ppid_item);
	parent->setChild(seq, PDX.path, path_item);
	parent->setChild(seq, PDX.desc, desc_item);
	parent->setChild(seq, PDX.corp, corp_item);
	parent->setChild(seq, PDX.ctime, ctime_item);
}

void ProcessMgr::AjustProcessStyle()
{
	auto view = ui.processView;
	view->resizeColumnToContents(1);
	view->resizeColumnToContents(2);
	QModelIndexList selected = view->selectionModel()->selectedIndexes();
	if (!selected.isEmpty()) {
		const QModelIndex& idx = selected[0];
		view->selectionModel()->select(idx, QItemSelectionModel::Select | QItemSelectionModel::Rows);
		view->scrollTo(idx);
	}
}