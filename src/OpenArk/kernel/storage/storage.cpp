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
#include "storage.h"
#include "../../../OpenArkDrv/arkdrv-api/arkdrv-api.h"

bool UnlockFileSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const {
	auto s1 = sourceModel()->data(left); auto s2 = sourceModel()->data(right);
	return QString::compare(s1.toString(), s2.toString(), Qt::CaseInsensitive) < 0;
}

KernelStorage::KernelStorage()
{

}

KernelStorage::~KernelStorage()
{

}

void KernelStorage::onTabChanged(int index)
{
	CommonTabObject::onTabChanged(index);
}

bool KernelStorage::eventFilter(QObject *obj, QEvent *e)
{
	if (e->type() == QEvent::ContextMenu) {
		QMenu *menu = nullptr;
		if (obj == ui_->unlockView->viewport()) menu = unlock_menu_;
		//if (obj == ui_->fsfltView->viewport()) menu = fsflt_menu_;
		QContextMenuEvent *ctxevt = dynamic_cast<QContextMenuEvent*>(e);
		if (ctxevt && menu) {
			menu->move(ctxevt->globalPos());
			menu->show();
		}
	} else if (e->type() == QEvent::MouseMove) {
		QMouseEvent *mouse = static_cast<QMouseEvent *>(e);
		if (obj == ui_->inputPathEdit) {
			if (ui_->inputPathEdit->text().isEmpty()) {
				QString tips(tr("Tips: \n1. You can copy file or directory and paste to here(Enter key to ShowHold).\n"
					"2. You need enter kernel mode to view FileHold.\n"
					"3. Path is case insensitive."));
				QToolTip::showText(mouse->globalPos(), tips);
				return true;
			}
		}
	} else if (e->type() == QEvent::KeyPress) {
		QKeyEvent *keyevt = dynamic_cast<QKeyEvent*>(e);
		if ((keyevt->key() == Qt::Key_Enter) || (keyevt->key() == Qt::Key_Return)) {
			ui_->showHoldBtn->click();
		}
	}
	return QWidget::eventFilter(obj, e);
}

void KernelStorage::ModuleInit(Ui::Kernel *ui, Kernel *kernel)
{
	this->ui_ = ui;
	this->kernel_ = kernel;

	Init(ui->tabStorage, TAB_KERNEL, TAB_KERNEL_STORAGE);

	InitFileUnlockView();
	//InitFileFilterView();
}

void KernelStorage::InitFileUnlockView()
{
	unlock_model_ = new QStandardItemModel;
	QTreeView *view = ui_->unlockView;
	proxy_unlock_ = new UnlockFileSortFilterProxyModel(view);
	std::vector<std::pair<int, QString>> layout = {
		{ 150, tr("ProcessName") },
		{ 50, tr("PID") },
		{ 340, tr("FilePath") },
		{ 250, tr("ProcessPath") },
		{ 50, tr("Type") },
		{ 150, tr("FileObject/DllBase") },
		{ 70, tr("FileHandle") },
	};
	SetDefaultTreeViewStyle(view, unlock_model_, proxy_unlock_, layout);
	view->viewport()->installEventFilter(this);
	view->installEventFilter(this);
	ui_->inputPathEdit->installEventFilter(this);
	ui_->inputPathEdit->setMouseTracking(true);
	unlock_menu_ = new QMenu();
	unlock_menu_->addAction(tr("Refresh"), this, [&] {
		ui_->showHoldBtn->click();
	});

	unlock_menu_->addAction(tr("Copy"), this, [&] {
		auto view = ui_->unlockView;
		ClipboardCopyData(GetCurItemViewData(view, GetCurViewColumn(view)).toStdString());
	});

	unlock_menu_->addSeparator();
	unlock_menu_->addAction(tr("Unlock"), this, [&] {
		ui_->unlockFileBtn->click();
	});

	unlock_menu_->addAction(tr("Unlock All"), this, [&] {
		ui_->unlockFileAllBtn->click();
	});

	unlock_menu_->addAction(tr("Kill Process"), this, [&] {
		ui_->killProcessBtn->click();
	});

	unlock_menu_->addSeparator();
	unlock_menu_->addAction(tr("Scan Selected"), this, [&] {
		kernel_->GetParent()->SetActiveTab(TAB_SCANNER);
		auto column = GetCurViewColumn(ui_->unlockView);
		if (column == 0) column = 3;
		emit kernel_->signalOpen(GetCurItemViewData(ui_->unlockView, column));
	});
	unlock_menu_->addAction(tr("Explore Selected"), this, [&] {
		auto column = GetCurViewColumn(ui_->unlockView);
		if (column == 0) column = 3; 
		ExploreFile(GetCurItemViewData(ui_->unlockView, column));
	});
	unlock_menu_->addAction(tr("Properties Selected..."), this, [&]() {
		auto column = GetCurViewColumn(ui_->unlockView);
		if (column == 0) column = 3;
		WinShowProperties(GetCurItemViewData(ui_->unlockView, column).toStdWString());
	});
	connect(ui_->inputPathEdit, &QLineEdit::textChanged, [&](QString str) { 
		if (str.contains("file:///")) {
			ui_->inputPathEdit->setText(str.replace("file:///", ""));
			ui_->inputPathEdit->setText(str.replace("/", "\\"));
		}
	});

	connect(ui_->showHoldBtn, &QPushButton::clicked, [&] {
		DISABLE_RECOVER();
		ClearItemModelData(unlock_model_, 0);

		QString file = ui_->inputPathEdit->text();
		if (file.isEmpty()) {
			ERR(L"Please input the file path...");
			return;
		}
		std::wstring origin, path;
		std::vector<HANDLE_ITEM> items;
		origin = UNONE::FsPathStandardW(file.toStdWString());
		UNONE::ObParseToNtPathW(origin, path);
		UNONE::StrLowerW(path);
		ArkDrvApi::Storage::UnlockEnum(path, items);
		for (auto item : items) {
			auto pid = (DWORD)item.pid;
			ProcInfo info;
			CacheGetProcInfo(pid, info);
			auto &&ppath = info.path;
			auto &&pname = info.name;
			std::wstring fpath;
			UNONE::ObParseToDosPathW(item.name, fpath);
			auto item_pname = new QStandardItem(LoadIcon(ppath), pname);
			auto item_pid = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"%d", pid)));
			auto item_fpath = new QStandardItem(WStrToQ(fpath));
			auto item_ftype = new QStandardItem("FILE");
			auto item_fobj = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"0x%p", item.object)));
			auto item_ppath = new QStandardItem(ppath);
			auto item_fhandle = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"0x%X", (DWORD)item.handle)));
			
			auto count = unlock_model_->rowCount();
			unlock_model_->setItem(count, 0, item_pname);
			unlock_model_->setItem(count, 1, item_pid);
			unlock_model_->setItem(count, 2, item_fpath);
			unlock_model_->setItem(count, 3, item_ppath);
			unlock_model_->setItem(count, 4, item_ftype);
			unlock_model_->setItem(count, 5, item_fobj);
			unlock_model_->setItem(count, 6, item_fhandle);
		}
		// Add process dll path
		std::vector<DWORD> pids;
		UNONE::PsGetAllProcess(pids);
		for (auto pid : pids) {
			UNONE::PsEnumModule(pid, [&](MODULEENTRY32W& entry)->bool {
				if (UNONE::StrContainIW(entry.szExePath, origin)) {
					QString modname = WCharsToQ(entry.szModule);
					QString modpath = WCharsToQ(entry.szExePath);
					HANDLE  hmodule = entry.hModule;
					ProcInfo info;
					CacheGetProcInfo(pid, info);
					auto &&ppath = info.path;
					auto &&pname = info.name;

					auto item_pname = new QStandardItem(LoadIcon(ppath), pname);
					auto item_pid = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"%d", pid)));
					auto item_fpath = new QStandardItem(modpath);
					auto item_ppath = new QStandardItem(ppath);
					auto item_ftype = new QStandardItem("DLL");
					auto item_fobj = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"0x%p", hmodule))); 
					auto item_fhandle = new QStandardItem("0");

					auto count = unlock_model_->rowCount();
					unlock_model_->setItem(count, 0, item_pname);
					unlock_model_->setItem(count, 1, item_pid);
					unlock_model_->setItem(count, 2, item_fpath);
					unlock_model_->setItem(count, 3, item_ppath);
					unlock_model_->setItem(count, 4, item_ftype);
					unlock_model_->setItem(count, 5, item_fobj);
					unlock_model_->setItem(count, 6, item_fhandle);
				}
				return true;
			});
		}
	});

	auto CommonUnlock = [&](QString type, DWORD pid, QString fobj, QString fhandle, QString fpath="") {
		if (type == "DLL") {
			// Close by kill process
			if (fpath.compare(CacheGetProcInfo(pid).path, Qt::CaseInsensitive)) {
				PsKillProcess(pid);
				return;
			}
			// Close by free library
			ULONG64 remote_routine = GetFreeLibraryAddress(pid);
			if (remote_routine) {
				ULONG64 pararm = QHexToQWord(fobj);
				for (int i = 0; i < 10; i++) {
					UNONE::PsCreateRemoteThread((DWORD)pid, remote_routine, pararm, 0);
					INFO("%d %lld %lld", pid, remote_routine, pararm);
				}
			}
		} else {
			// Close by driver
			HANDLE_ITEM handle_item = { 0 };
			handle_item.pid = HANDLE(pid);
			handle_item.handle = HANDLE(QHexToDWord(fhandle));
			ArkDrvApi::Storage::UnlockClose(handle_item);
		}
	};

	connect(ui_->unlockFileBtn, &QPushButton::clicked, [&]{
		DISABLE_RECOVER();
		auto selected = ui_->unlockView->selectionModel()->selectedIndexes();
		if (selected.empty()) return;
		int count = unlock_model_->columnCount();
		for (int i = 0; i < selected.size() / count; i++) {
			auto pid = ui_->unlockView->model()->itemData(selected[i * count + 1]).values()[0].toUInt();
			auto fpath = ui_->unlockView->model()->itemData(selected[i * count + 2]).values()[0].toString();
			auto type = ui_->unlockView->model()->itemData(selected[i * count + 4]).values()[0].toString();
			auto fobj = ui_->unlockView->model()->itemData(selected[i * count + 5]).values()[0].toString();
			auto fhandle = ui_->unlockView->model()->itemData(selected[i * count + 6]).values()[0].toString();
			CommonUnlock(type, pid, fobj, fhandle, fpath);
		}
		ui_->showHoldBtn->click();
	});

	connect(ui_->unlockFileAllBtn, &QPushButton::clicked, [&] {
		DISABLE_RECOVER();
		for (int i = 0; i < unlock_model_->rowCount(); i++) {
			QStandardItem *item = unlock_model_->item(i, 1); //pid
			auto pid = item->text().toUInt();
			auto type = unlock_model_->item(i, 4)->text();
			auto fobj = unlock_model_->item(i, 5)->text();
			auto fhandle = unlock_model_->item(i, 6)->text();
			CommonUnlock(type, pid, fobj, fhandle);
		}
		ui_->showHoldBtn->click();
	});

	connect(ui_->killProcessBtn, &QPushButton::clicked, [&] {
		DISABLE_RECOVER();
		auto selected = ui_->unlockView->selectionModel()->selectedIndexes();
		if (selected.empty()) return;
		int count = unlock_model_->columnCount();
		for (int i = 0; i < selected.size() / count; i++) {
			auto pid = ui_->unlockView->model()->itemData(selected[i * count + 1]).values()[0].toUInt();
			PsKillProcess(pid);
		}
		ui_->showHoldBtn->click();
	});
}

void KernelStorage::InitFileFilterView()
{
/*
	fsflt_model_ = new QStandardItemModel;
	fsflt_model_->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Value"));
	SetDefaultTreeViewStyle(ui_->fsfltView, fsflt_model_);
	ui_->fsfltView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui_->fsfltView->viewport()->installEventFilter(this);
	ui_->fsfltView->installEventFilter(this);
	fsflt_menu_ = new QMenu();
	fsflt_menu_->addAction(tr("ExpandAll"), this, SLOT(onExpandAll()));*/
}

void KernelStorage::ShowUnlockFiles()
{

}
