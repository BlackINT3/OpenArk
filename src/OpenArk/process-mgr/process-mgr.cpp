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
#include "../cmds/constants/constants.h"
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
} PS;
// ModuleView's header index
struct {
	int s = 0;
	int name = s++;
	int base = s++;
	int size = s++;
	int path = s++;
	int desc = s++;
	int ver = s++;
	int corp = s++;
	int sign = s++;
} MOD;
// HandleView's header index
struct {
	int s = 0;
	int type = s++;
	int name = s++;
	int value = s++;
	int access = s++;
	int obj = s++;
} HD;
// MemoryView's header index
struct {
	int s = 0;
	int addr = s++;
	int size = s++;
	int property = s++;
	int state = s++;
	int type = s++;
	int base = s++;
	int mod = s++;
} MEM;

#define BOTTOM_HIDE -1
#define BOTTOM_MOD 0
#define BOTTOM_HD 1
#define BOTTOM_MEM 2

bool ProcSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
	auto s1 = sourceModel()->data(left); auto s2 = sourceModel()->data(right);
	auto column = left.column();
	if ((column == PS.pid || column == PS.ppid)) return s1.toUInt() < s2.toUInt();
	return QString::compare(s1.toString(), s2.toString(), Qt::CaseInsensitive) < 0;
}
bool ModSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
	auto s1 = sourceModel()->data(left); auto s2 = sourceModel()->data(right);
	auto column = left.column();
	bool ok;
	if (bottom_idx_ == BOTTOM_MOD) {
		if ((column == MOD.base || column == MOD.size))
			return s1.toString().toULongLong(&ok, 16) < s2.toString().toULongLong(&ok, 16);
	} else if (bottom_idx_ == BOTTOM_HD) {
		if ((column == HD.value || column == HD.access || column == HD.obj))
			return s1.toString().toULongLong(&ok, 16) < s2.toString().toULongLong(&ok, 16);
	} else if (bottom_idx_ == BOTTOM_MEM) {
		if ((column == MEM.addr || column == MEM.size || column == MEM.base))
			return s1.toString().toULongLong(&ok, 16) < s2.toString().toULongLong(&ok, 16);
	}

	return QString::compare(s1.toString(), s2.toString(), Qt::CaseInsensitive) < 0;
}

ProcessMgr::ProcessMgr(QWidget* parent) :
	parent_((OpenArk*)parent),
	cntproc_label_(nullptr),
	proxy_proc_(nullptr),
	proc_header_idx_(0),
	bottom_header_idx_(0),
	bottom_idx_(BOTTOM_HIDE),
	mod_menu_(nullptr),
	hd_menu_(nullptr),
	mem_menu_(nullptr)
{
	unnamed_checked_ = false;
	uncommed_checked_ = false;
	nonexec_checked_ = false;
	imaged_checked_ = false;

	ui.setupUi(this);
	connect(OpenArkLanguage::Instance(), &OpenArkLanguage::languageChaned, this, [this]() {ui.retranslateUi(this); });

	ui.splitter->setStretchFactor(0, 2);
	ui.splitter->setStretchFactor(1, 1);
	
	InitProcessView();
	InitBottomCommon();
	InitModuleView();
	InitHandleView();
	InitMemoryView();
	connect(parent_, SIGNAL(signalShowPtool(int)), this, SLOT(onShowBottom(int)));
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
				DISABLE_RECOVER();
				QModelIndex idx = ui.processView->indexAt(pt);
				const QModelIndex &curidx = idx.sibling(idx.row(), 1);
				if (!curidx.isValid()) return true;
				auto pid = curidx.data(Qt::DisplayRole).toInt();
				if (pid == 0 || pid == 4) return true;
				auto info = CacheGetProcessBaseInfo(pid);
				info.CommandLine = UNONE::StrInsertW(info.CommandLine, 120, L"\n    ");
				info.ImagePathName = UNONE::StrInsertW(info.ImagePathName, 120, L"\n    ");
				QString tips = QString("Command Line:\n    %1\nPath:\n    %2").arg(WStrToQ(info.CommandLine)).arg(WStrToQ(info.ImagePathName));
				QToolTip::showText(mouse->globalPos(), tips);
				return true;
			}
		}
	} else if (obj == ui.moduleView->viewport()) {
		if (e->type() == QEvent::ContextMenu) {
			QContextMenuEvent* ctxevt = dynamic_cast<QContextMenuEvent*>(e);
			if (ctxevt != nullptr) {
				QMenu *menu = nullptr;
				switch (bottom_idx_) {
				case BOTTOM_MOD:
					menu = mod_menu_;
					break;
				case BOTTOM_HD:
					menu = hd_menu_;
					break;
				case BOTTOM_MEM:
					menu = mem_menu_;
					break;
				}
				if (menu) {
					menu->move(ctxevt->globalPos());
					menu->show();
				}
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
		onShowProcess();
		return;
	}
	if (IsContainAction(mod_menu_, sender)) {
		onShowModule();
		return;
	}
	if (sender == nullptr) {
		onShowProcess();
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
	if (cntproc_label_ == nullptr) {
		cpu_percent_label_ = new QLabel(); parent_->StatusBarAdd(cpu_percent_label_);
		mm_percent_label_ = new QLabel(); parent_->StatusBarAdd(mm_percent_label_);
		cntproc_label_ = new QLabel(); parent_->StatusBarAdd(cntproc_label_);
		cntthread_label_ = new QLabel(); parent_->StatusBarAdd(cntthread_label_);
		cnthandle_label_ = new QLabel(); parent_->StatusBarAdd(cnthandle_label_);
	}
	cpu_percent_label_->setText(tr("CPU:") + WStrToQ(UNONE::StrFormatW(L"%0.2f%%", GetSystemUsageOfCPU())));
	mm_percent_label_->setText(tr("Memory:") + WStrToQ(UNONE::StrFormatW(L"%0.2f%%", GetSystemUsageOfMemory())));
	cntproc_label_->setText(tr("Processes:") + QString("%1").arg(perf.ProcessCount));
	cntthread_label_->setText(tr("Threads:") + QString("%1").arg(perf.ThreadCount));
	cnthandle_label_->setText(tr("Handles:") + QString("%1").arg(perf.HandleCount));
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
	auto msbox = QMessageBox::warning(this, tr("Warning"), tips, QMessageBox::Yes | QMessageBox::No);
	if (msbox == QMessageBox::Yes) {
		for (auto d : pids) { UNONE::PsKillProcess(d); };
		onRefresh();
	}
}

void ProcessMgr::onRestartProcess()
{
	UNONE::PsRestartProcess(ProcCurPid());
	onRefresh();
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
		ExploreFile(ProcCurViewItemData(PS.path));
		return;
	}
	if (IsContainAction(mod_menu_, sender)) {
		ExploreFile(BottomCurViewItemData(MOD.path));
		return;
	}
	if (sender == nullptr) {
		ExploreFile(ProcCurViewItemData(PS.path));
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
		path = ProcCurViewItemData(PS.path);
	} else if (IsContainAction(mod_menu_, action)) {
		path = BottomCurViewItemData(MOD.path);
	}
	parent_->SetActiveTab(TAB_SCANNER);
	emit signalOpen(path);
}

void ProcessMgr::onVerifySignature()
{
	QString path;
	path = BottomCurViewItemData(MOD.path);
	auto row = ui.moduleView->currentIndex().row();
	QString owner;
	bool ret = GetCertOwner(path, owner);
	if (!ret) {
		auto err = GetLastError();
		owner = WStrToQ(UNONE::StrFormatW(L"[-] %X %s", err, UNONE::OsDosErrorMsgW(err).c_str()));
		SetLineBgColor(bottom_model_, row, Qt::red);
	}
	SetCurItemViewData(ui.moduleView, MOD.sign, owner);
}

void ProcessMgr::onVerifyAllSignature()
{
	for (int i = 0; i < bottom_model_->rowCount(); i++) {
		auto row = i;
		QString path = bottom_model_->item(row, MOD.path)->data(Qt::DisplayRole).toString();
		QString owner;
		bool ret = GetCertOwner(path, owner);
		if (!ret) {
			auto err = GetLastError();
			owner = WStrToQ(UNONE::StrFormatW(L"[-] %X %s", err, UNONE::OsDosErrorMsgW(err).c_str()));
			SetLineBgColor(bottom_model_, row, Qt::red);
		}
		bottom_model_->item(row, MOD.sign)->setData(owner, Qt::DisplayRole);
	}
	ui.moduleView->header()->setSortIndicator(MOD.sign, Qt::AscendingOrder);
}

void ProcessMgr::onShowProperties()
{
	ShowProperties(ProcCurPid(), 0);
}

void ProcessMgr::onCloseHandle()
{
	auto src_hd = (HANDLE)(UNONE::StrToHex64A(BottomCurViewItemData(HD.value).toStdString()));
	DWORD pid = ProcCurPid();
	HANDLE phd = OpenProcessWrapper(PROCESS_DUP_HANDLE | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	if (!phd) {
		ERR(L"OpenProcess pid:%d err:%d", pid, GetLastError());
		return;
	}
	HANDLE dup = NULL;
	if (!DuplicateHandle(phd, (HANDLE)src_hd, GetCurrentProcess(), &dup, 0, FALSE, DUPLICATE_CLOSE_SOURCE)) {
		ERR(L"DuplicateHandle pid:%d hd:%x err:%d", pid, src_hd, GetLastError());
		CloseHandle(phd);
		return;
	}
	INFO(L"DuplicateHandle pid:%d hd:%x ok", pid, src_hd);
	CloseHandle(dup);
	CloseHandle(phd);
	onShowHandle();
}

void ProcessMgr::onHideUnnamedHandles(bool checked)
{
	unnamed_checked_ = checked;
	for (int i = 0; i < bottom_model_->rowCount(); i++) {
		if (!checked) {
			SetLineHidden(ui.moduleView, i, false);
			continue;
		}
		if (bottom_model_->item(i, HD.name)->data(Qt::DisplayRole).toString().isEmpty()) {
			QModelIndex idx = bottom_model_->index(i, HD.name);
			SetLineHidden(ui.moduleView, proxy_bottom_->mapFromSource(idx).row(), true);
		}
	}
}

void ProcessMgr::onHideMemoryItem(bool checked)
{
	auto sender = QObject::sender();
	if (sender == ui.actionHideUncommited)
		uncommed_checked_ = checked;
	else if (sender == ui.actionHideNonExecute)
		nonexec_checked_ = checked;
	else if (sender == ui.actionHideImage)
		imaged_checked_ = checked;

	for (int i = 0; i < bottom_model_->rowCount(); i++) {
		bool hidden = false;
		if (uncommed_checked_) {
			if (bottom_model_->item(i, MEM.state)->data(Qt::DisplayRole).toString() != "MEM_COMMIT")
				hidden = true;
		}
		if (nonexec_checked_) {
			if (!bottom_model_->item(i, MEM.property)->data(Qt::DisplayRole).toString().contains("EXECUTE"))
				hidden = true;
		}
		if (imaged_checked_) {
			if (bottom_model_->item(i, MEM.type)->data(Qt::DisplayRole).toString() == "MEM_IMAGE")
				hidden = true;
		}
		QModelIndex idx = bottom_model_->index(i, HD.name);
		if (hidden) {
			SetLineHidden(ui.moduleView, proxy_bottom_->mapFromSource(idx).row(), true);
		} else {
			SetLineHidden(ui.moduleView, proxy_bottom_->mapFromSource(idx).row(), false);
		}
	}
}

void ProcessMgr::onDumpMemory()
{
	DWORD pid = ProcCurPid();
	HANDLE phd = OpenProcessWrapper(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	if (!phd) {
		ERR(L"OpenProcess pid:%d err:%d", pid, GetLastError());
		return;
	}
	DWORD size = UNONE::StrToHexA(BottomCurViewItemData(MEM.size).toStdString());
	DWORD64 addr = UNONE::StrToHex64A(BottomCurViewItemData(MEM.base).toStdString());
	std::string data;
	data.resize(size);
	SIZE_T readlen;
	bool ret = ReadProcessMemory(phd, (LPCVOID)addr, (LPVOID)data.data(), size, &readlen);
	if (!ret && size != readlen) {
		ERR(L"ReadProcessMemory pid:%d err:%d, expect:%d readlen:%d", pid, GetLastError(), size, readlen);
		CloseHandle(phd);
		return;
	}
	QString filename = StrToQ(UNONE::StrFormatA("%X_%X", addr, size));
	QString dumpmem = QFileDialog::getSaveFileName(this, tr("Save to"), filename, tr("DumpMemory(*)"));
	if (!dumpmem.isEmpty()) {
		ret = UNONE::FsWriteFileDataW(dumpmem.toStdWString(), data);
		if (ret) {
			MsgBoxInfo(tr("Dump memory ok"));
		}
	}
	CloseHandle(phd);
}

void ProcessMgr::onShowBottom(int idx)
{
	ui.moduleView->show();
	switch (idx) {
	case BOTTOM_MOD:
		onShowModule();
		break;
	case BOTTOM_HD:
		onShowHandle();
		break;
	case BOTTOM_MEM:
		onShowMemory();
		break;
	default:
		ui.moduleView->hide();
		break;
	}
	bottom_idx_ = idx;
	proxy_bottom_->bottom_idx_ = idx;
}

void ProcessMgr::onShowProcess()
{
	DISABLE_RECOVER();
	auto view = ui.processView;
	auto selected = view->selectionModel()->selectedIndexes();
	if (!selected.empty()) {
		QRect rect = view->visualRect(selected[0]);
		proc_sel_ = rect.center();
	}
	ClearItemModelData(proc_model_);
	CacheRefreshProcInfo();
	if (proc_header_idx_ == 0) ShowProcessTree();
	else ShowProcessList();
	AjustProcessStyle();
}

void ProcessMgr::onShowModule()
{
	DISABLE_RECOVER();
	ClearItemModelData(bottom_model_, 0);
	InitModuleView();
	DWORD pid = ProcCurPid();
	UNONE::PsEnumModule(pid, [&](MODULEENTRY32W& entry)->bool{
		QString modname = WCharsToQ(entry.szModule);
		QString modpath = WCharsToQ(entry.szExePath);
		ULONG64 modbase = (ULONG64)entry.modBaseAddr;
		ULONG64 modsize = entry.modBaseSize;
		auto count = bottom_model_->rowCount();
		for (int i = 0; i < count; i++) {
			auto base = bottom_model_->data(bottom_model_->index(i, MOD.base)).toString().toStdWString();
			if (UNONE::StrToHex64W(base) == modbase) {
				return true;
			}
		}
		auto info = CacheGetFileBaseInfo(modpath);
		QStandardItem *name_item = new QStandardItem(LoadIcon(modpath), modname);
		QStandardItem *base_item = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"0x%llX", modbase)));
		QStandardItem *size_item = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"0x%llX", modsize)));
		QStandardItem *path_item = new QStandardItem(modpath);
		QStandardItem *desc_item = new QStandardItem(info.desc);
		QStandardItem *ver_item = new QStandardItem(info.ver);
		QStandardItem *corp_item = new QStandardItem(info.corp);
		QStandardItem *sign_item = new QStandardItem("");
		bottom_model_->setItem(count, MOD.name, name_item);
		bottom_model_->setItem(count, MOD.base, base_item);
		bottom_model_->setItem(count, MOD.size, size_item);
		bottom_model_->setItem(count, MOD.path, path_item);
		bottom_model_->setItem(count, MOD.desc, desc_item);
		bottom_model_->setItem(count, MOD.ver, ver_item);
		bottom_model_->setItem(count, MOD.corp, corp_item);
		bottom_model_->setItem(count, MOD.sign, sign_item);
		return true;
	});
	auto view = ui.moduleView;
	view->setColumnWidth(MOD.name, 150);
	view->resizeColumnToContents(MOD.base);
	view->resizeColumnToContents(MOD.size);
	view->setColumnWidth(MOD.path, 290);
	view->setColumnWidth(MOD.desc, 200);
	view->setColumnWidth(MOD.corp, 150);
}

void ProcessMgr::onShowHandle()
{
	DISABLE_RECOVER();
	ClearItemModelData(bottom_model_, 0);
	InitHandleView();
	InitObjectTypeTable();
	DWORD pid = ProcCurPid();
	HANDLE phd = OpenProcessWrapper(PROCESS_DUP_HANDLE | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	UNONE::PsEnumHandle(pid, [&](SYSTEM_HANDLE_TABLE_ENTRY_INFO &info)->bool {
		auto count = bottom_model_->rowCount();
		auto idx = info.ObjectTypeIndex;
		QStandardItem *type_item = new QStandardItem(StrToQ(UNONE::StrFormatA("%s (%02d)",ObjectTypeTable[idx].c_str(), idx)));
		std::string name;
		if (phd != NULL) {
			HANDLE dup = NULL;
			if (DuplicateHandle(phd, (HANDLE)info.HandleValue, GetCurrentProcess(), &dup, 0, FALSE, DUPLICATE_SAME_ACCESS)) {
				switch (idx) {
				case 7: {
					DWORD pid = GetProcessId(dup);
					name = UNONE::StrFormatA("%s(%d)", UNONE::PsGetProcessNameA(pid).c_str(), pid);
					break;
				}
				case 8: {
					typedef DWORD (WINAPI *__GetThreadId)(HANDLE Thread);
					auto pGetThreadId = (__GetThreadId)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "GetThreadId");
					DWORD tid = 0;
					if (pGetThreadId) tid = pGetThreadId(dup);
					DWORD pid = UNONE::PsGetPidByThread(tid);
					name = UNONE::StrFormatA("%s(%d) %d", UNONE::PsGetProcessNameA(pid).c_str(), pid, tid);
				}
				default:
					ObGetObjectName((HANDLE)dup, name);
					static int file_idx = GetObjectTypeIndex("File");
					if (idx == file_idx) UNONE::ObParseToDosPathA(name, name);
					break;
				}
				CloseHandle(dup);
			}
		}

		QStandardItem *name_item = new QStandardItem(StrToQ(name));
		QStandardItem *value_item = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"0x%llX", info.HandleValue)));
		QStandardItem *access_item = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"0x%llX", info.GrantedAccess)));
		QStandardItem *obj_item = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"0x%llX", info.Object)));
		bottom_model_->setItem(count, HD.type, type_item);
		bottom_model_->setItem(count, HD.name, name_item);
		bottom_model_->setItem(count, HD.value, value_item);
		bottom_model_->setItem(count, HD.access, access_item);
		bottom_model_->setItem(count, HD.obj, obj_item);
		return true;
	});
	CloseHandle(phd);
}

void ProcessMgr::onShowMemory()
{
	DISABLE_RECOVER();
	ClearItemModelData(bottom_model_, 0);
	InitMemoryView();

	DWORD pid = ProcCurPid();
	HANDLE phd = OpenProcessWrapper(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	UNONE::PsEnumMemory(pid, [&](MEMORY_BASIC_INFORMATION &mbi)->bool {
		
		std::wstring mod_name;
		WCHAR name[MAX_PATH + 1] = { 0 };
		if (mbi.Type & MEM_IMAGE) {
			GetMappedFileNameW(phd, mbi.BaseAddress, name, MAX_PATH);
			UNONE::ObParseToDosPathW(name, mod_name);
		}

		auto count = bottom_model_->rowCount();
		QStandardItem *addr_item = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"0x%llX", mbi.BaseAddress)));
		QStandardItem *size_item = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"0x%X", mbi.RegionSize)));
		QStandardItem *property_item = new QStandardItem(StrToQ(MbiPageProtectToString(mbi.Protect)));
		QStandardItem *state_item = new QStandardItem(StrToQ(MbiStateToString(mbi.State)));
		QStandardItem *type_item = new QStandardItem(StrToQ(MbiTypeToString(mbi.Type)));
		QStandardItem *base_item = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"0x%llX", mbi.AllocationBase)));
		QStandardItem *mod_item = new QStandardItem(WStrToQ(mod_name));

		bottom_model_->setItem(count, MEM.addr, addr_item);
		bottom_model_->setItem(count, MEM.size, size_item);
		bottom_model_->setItem(count, MEM.property, property_item);
		bottom_model_->setItem(count, MEM.state, state_item);
		bottom_model_->setItem(count, MEM.type, type_item);
		bottom_model_->setItem(count, MEM.base, base_item);
		bottom_model_->setItem(count, MEM.mod, mod_item);

		if ((mbi.Protect & PAGE_NOACCESS) || (mbi.State & MEM_RESERVE)) {
			SetLineBgColor(bottom_model_, count, Qt::gray);
		}

		return true;
	});
}

void ProcessMgr::onSectionClicked(int idx)
{
	auto sender = QObject::sender();
	if (sender == ui.processView->header()) {
		if (idx == PS.name) {
			proc_header_idx_++;
			switch (proc_header_idx_) {
			case 3:
				ui.processView->header()->setSortIndicator(-1, Qt::AscendingOrder);
				proc_header_idx_ = 0;
				onShowProcess();
				break;
			case 1:
				onShowProcess();
			}
		} else {
			if (proc_header_idx_ == 0) {
				proc_header_idx_ = 1;
				onShowProcess();
			}
		}
	}	else if (sender == ui.moduleView->header()) {
		if (idx == bottom_header_last_) {
			bottom_header_idx_ = 1;
		} else {
			bottom_header_idx_++;
		}
		if (bottom_header_idx_ == 3) {
			bottom_header_idx_ = 0;
			ui.moduleView->header()->setSortIndicator(-1, Qt::AscendingOrder);
		}
	}
	onProcSelection(DWordToDecQ(cur_pid_));
}

void ProcessMgr::onProcDoubleClicked( const QModelIndex &idx )
{
	onShowProperties();
}

void ProcessMgr::onProcChanged(const QModelIndex &current, const QModelIndex &previous)
{
	if (current.isValid()) {
		//auto row = current.row();
		cur_pid_ = current.sibling(current.row(), PS.pid).data().toUInt();
	}
	onShowBottom(bottom_idx_);
}

void ProcessMgr::onProcSelection(QString pid)
{
	auto view = ui.processView;
	std::function<bool(QModelIndex idx)> LocateProcess = [&](QModelIndex idx)->bool {
		int rows = proc_model_->rowCount(idx);
		for (int i = 0; i < rows; i++) {
			QString qstr;
			QModelIndex child_name;
			QStandardItem *item;
			if (idx == view->rootIndex()) {
				child_name = proc_model_->index(i, PS.name);
				item = proc_model_->itemFromIndex(child_name);
				qstr = proc_model_->index(i, PS.pid).data(Qt::DisplayRole).toString();
			} else {
				item = proc_model_->itemFromIndex(idx);
				child_name = item->child(i, PS.name)->index();
				qstr = item->child(i, PS.pid)->data(Qt::DisplayRole).toString();
			}
			if (qstr == pid) {
				auto idx = proxy_proc_->mapFromSource(child_name);
				view->selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
				view->scrollTo(idx);
				return true;
			}
			if (proc_model_->itemFromIndex(child_name)->hasChildren()) {
				if (LocateProcess(child_name)) {
					return true;
				}
			}
		}
		return false;
	};

	LocateProcess(view->rootIndex());
}

DWORD ProcessMgr::ProcCurPid()
{
	//auto idx = ui.processView->currentIndex();
	//DWORD pid = idx.sibling(idx.row(), PS.pid).data().toUInt();
	return cur_pid_;
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

QString ProcessMgr::BottomCurViewItemData(int column)
{
	return GetCurItemViewData(ui.moduleView, column);
}

QString ProcessMgr::ModViewItemData(int row, int column)
{
	return GetItemViewData(ui.moduleView, row, column);
}

void ProcessMgr::InitProcessView()
{
	//process list
	auto copy_menu_ = new QMenu();
	copy_menu_->addAction(tr("Process Name"))->setData(PS.name);
	copy_menu_->addAction(tr("Process ID"))->setData(PS.pid);
	copy_menu_->addAction(tr("Parent ID"))->setData(PS.ppid);
	copy_menu_->addAction(tr("Process Path"))->setData(PS.path);
	copy_menu_->addAction(tr("Created Time"))->setData(PS.ctime);
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
	proc_menu_->addAction(tr("Restart Process"), this, SLOT(onRestartProcess()));
	//proc_menu_->addAction(tr("Suspend"), this, SLOT(onSuspendProcess()));
	proc_menu_->addAction(tr("Select PID"), this, SLOT(onSelectPid()), QKeySequence("CTRL+G"));
	proc_menu_->addAction(tr("Explore File"), this, SLOT(onExploreFile()), QKeySequence("CTRL+L"));
	proc_menu_->addAction(tr("Enum Thread"), this, SLOT(onEnumThread()));
	proc_menu_->addAction(tr("Enum Window"), this, SLOT(onEnumWindow()));
	proc_menu_->addAction(tr("Inject Dll"), this, SLOT(onInjectDll()), QKeySequence("CTRL+J"));
	proc_menu_->addAction(tr("Sendto Scanner"), this, SLOT(onSendtoScanner()));
	proc_menu_->addAction(dump_menu_->menuAction());
	proc_menu_->addAction(tr("Properties..."), this, SLOT(onShowProperties()), QKeySequence("CTRL+P"));

	proc_model_ = new QStandardItemModel;
	proc_model_->setHorizontalHeaderLabels(QStringList() << tr("Process") << tr("PID") << tr("PPID") << tr("Path") << tr("Description") << tr("Company Name") << tr("CreatedTime"));
	QTreeView *pview = ui.processView;
	proxy_proc_ = new ProcSortFilterProxyModel(pview);
	proxy_proc_->setSourceModel(proc_model_);
	proxy_proc_->setDynamicSortFilter(true);
	proxy_proc_->setFilterKeyColumn(1);
	pview->setModel(proxy_proc_);
	pview->selectionModel()->setModel(proxy_proc_);
	pview->header()->setSortIndicator(-1, Qt::AscendingOrder);
	pview->setSortingEnabled(true);
	pview->viewport()->installEventFilter(this);
	pview->installEventFilter(this);
	pview->setMouseTracking(true);
	pview->setEditTriggers(QAbstractItemView::NoEditTriggers);
	pview->setExpandsOnDoubleClick(false);
	pview->setColumnWidth(PS.name, 250);
	pview->setColumnWidth(PS.path, 400);
	pview->setColumnWidth(PS.desc, 190);
	pview->setColumnWidth(PS.corp, 155);
	connect(pview->header(), SIGNAL(sectionClicked(int)), this, SLOT(onSectionClicked(int)));
	connect(pview, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(onProcDoubleClicked(const QModelIndex&)));
	connect(pview->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &ProcessMgr::onProcChanged);
}

void ProcessMgr::InitBottomCommon()
{
	bottom_model_ = new QStandardItemModel;
	QTreeView *view = ui.moduleView;
	proxy_bottom_ = new ModSortFilterProxyModel(view);
	proxy_bottom_->setSourceModel(bottom_model_);
	proxy_bottom_->setDynamicSortFilter(true);
	proxy_bottom_->setFilterKeyColumn(1);
	view->setModel(proxy_bottom_);
	view->selectionModel()->setModel(proxy_bottom_);
	view->header()->setSortIndicator(-1, Qt::AscendingOrder);
	view->setSortingEnabled(true);
	view->viewport()->installEventFilter(this);
	view->installEventFilter(this);
	view->hide();
	connect(view->header(), SIGNAL(sectionClicked(int)), this, SLOT(onSectionClicked(int)));
}

void ProcessMgr::InitModuleView()
{
	if (!mod_menu_) {
		mod_menu_ = new QMenu();
		mod_menu_->addAction(tr("Refresh"), this, SLOT(onRefresh()));
		mod_menu_->addAction(tr("Explore File"), this, SLOT(onExploreFile()));
		mod_menu_->addAction(tr("Sendto Scanner"), this, SLOT(onSendtoScanner()));
		mod_menu_->addAction(tr("Verify Signature"), this, SLOT(onVerifySignature()));
		mod_menu_->addAction(tr("Verify All Signature"), this, SLOT(onVerifyAllSignature()));
		mod_menu_->addAction(tr("Properties..."), this, [&]() {WinShowProperties(BottomCurViewItemData(MOD.path).toStdWString()); });
	} else {
		bottom_model_->clear();
		bottom_model_->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Base") << tr("Size") << tr("Path") << tr("Description") << tr("Version") << tr("Company Name")<<tr("Signature"));
		auto bview = ui.moduleView;
		bview->header()->setSortIndicator(-1, Qt::AscendingOrder);
	}
}

void ProcessMgr::InitHandleView()
{
	if (!hd_menu_) {
		hd_menu_ = new QMenu();
		hd_menu_->addAction(tr("Close Handle"), this, SLOT(onCloseHandle()));
		hd_menu_->addAction(ui.actionHideUnnamed);
		connect(ui.actionHideUnnamed, SIGNAL(triggered(bool)), this, SLOT(onHideUnnamedHandles(bool)));
	} else {
		bottom_model_->clear();
		bottom_model_->setHorizontalHeaderLabels(QStringList() << tr("Type") << tr("Name") << tr("Value") << tr("Access") << tr("Object Address"));
		auto bview = ui.moduleView;
		bview->setColumnWidth(HD.type, 170);
		bview->setColumnWidth(HD.name, 700);
		bview->header()->setSortIndicator(-1, Qt::AscendingOrder);
	}
}

void ProcessMgr::InitMemoryView()
{
	if (!mem_menu_) {
		mem_menu_ = new QMenu();
		mem_menu_->addAction(ui.actionHideUncommited);
		mem_menu_->addAction(ui.actionHideNonExecute);
		mem_menu_->addAction(ui.actionHideImage);
		mem_menu_->addSeparator();
		mem_menu_->addAction(tr("Dump Memory"), this, SLOT(onDumpMemory()));
		connect(ui.actionHideUncommited, SIGNAL(triggered(bool)), this, SLOT(onHideMemoryItem(bool)));
		connect(ui.actionHideNonExecute, SIGNAL(triggered(bool)), this, SLOT(onHideMemoryItem(bool)));
		connect(ui.actionHideImage, SIGNAL(triggered(bool)), this, SLOT(onHideMemoryItem(bool)));
	} else {
		bottom_model_->clear();
		bottom_model_->setHorizontalHeaderLabels(QStringList() << tr("Address") << tr("Size") << tr("Property") << tr("State") << tr("Type") << tr("Base") << tr("Module"));
		auto bview = ui.moduleView;
		bview->setColumnWidth(MEM.addr, 150);
		bview->setColumnWidth(MEM.property, 180);
		bview->setColumnWidth(MEM.state, 180);
		bview->setColumnWidth(MEM.type, 180);
		bview->setColumnWidth(MEM.base, 150);
		bview->header()->setSortIndicator(-1, Qt::AscendingOrder);
	}
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
	DISABLE_RECOVER();
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

}

void ProcessMgr::ShowProcessTree()
{
	DISABLE_RECOVER();
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
		if (ppid == 0 || !info.parent_existed) {
			pis.push_back(info);
		}
		return true;
	});

	for (const auto& pi : pis) {
		AppendProcessTree(nullptr, pi, proc_model_->rowCount());
	}

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
		proc_model_->setItem(seq, PS.name, name_item);
		proc_model_->setItem(seq, PS.pid, pid_item);
		proc_model_->setItem(seq, PS.ppid, ppid_item);
		proc_model_->setItem(seq, PS.path, path_item);
		proc_model_->setItem(seq, PS.desc, desc_item);
		proc_model_->setItem(seq, PS.corp, corp_item);
		proc_model_->setItem(seq, PS.ctime, ctime_item);
		return;
	}

	parent->appendRow(name_item);
	parent->setChild(seq, PS.pid, pid_item);
	parent->setChild(seq, PS.ppid, ppid_item);
	parent->setChild(seq, PS.path, path_item);
	parent->setChild(seq, PS.desc, desc_item);
	parent->setChild(seq, PS.corp, corp_item);
	parent->setChild(seq, PS.ctime, ctime_item);
}

void ProcessMgr::AjustProcessStyle()
{
	auto view = ui.processView;
	view->resizeColumnToContents(1);
	view->resizeColumnToContents(2);

	QModelIndex idx = view->indexAt(proc_sel_);
	view->selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
	view->scrollTo(idx);
}