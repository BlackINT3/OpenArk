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
		if (obj == ui_->driverView->viewport()) menu = unlock_menu_;
		QContextMenuEvent *ctxevt = dynamic_cast<QContextMenuEvent*>(e);
		if (ctxevt && menu) {
			menu->move(ctxevt->globalPos());
			menu->show();
		}
	}
	return QWidget::eventFilter(obj, e);
}

void KernelStorage::ModuleInit(Ui::Kernel *ui, Kernel *kernel)
{
	this->ui_ = ui;

	Init(ui->tabStorage, TAB_KERNEL, TAB_KERNEL_STORAGE);

	InitFileUnlockView();
	InitFileFilterView();
}

void KernelStorage::InitFileUnlockView()
{
	unlock_model_ = new QStandardItemModel;
	QTreeView *view = ui_->unlockView;
	proxy_unlock_ = new UnlockFileSortFilterProxyModel(view);
	proxy_unlock_->setSourceModel(unlock_model_);
	proxy_unlock_->setDynamicSortFilter(true);
	proxy_unlock_->setFilterKeyColumn(1);
	view->setModel(proxy_unlock_);
	view->selectionModel()->setModel(proxy_unlock_);
	view->header()->setSortIndicator(-1, Qt::AscendingOrder);
	view->setSortingEnabled(true);
	view->viewport()->installEventFilter(this);
	view->installEventFilter(this);
	view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	std::pair<int, QString> colum_layout[] = {
		{ 150, tr("ProcessName") },
		{ 50, tr("PID") },
		{ 240, tr("FilePath") },
		{ 240, tr("ProcessPath") },
		{ 150, tr("FileObject") },
		{ 100, tr("FileHandle") },
	};
	QStringList name_list;
	for (auto p : colum_layout)  name_list << p.second;
	unlock_model_->setHorizontalHeaderLabels(name_list);
	for (int i = 0; i < _countof(colum_layout); i++) {
		view->setColumnWidth(i, colum_layout[i].first);
	}
	unlock_menu_ = new QMenu();
	unlock_menu_->addAction(tr("Refresh"), this, [&] {});

	connect(ui_->showHoldBtn, &QPushButton::clicked, [&] {
		DISABLE_RECOVER();
		ClearItemModelData(unlock_model_, 0);

		QString file = ui_->inputPathEdit->text();
		std::wstring path;
		std::vector<HANDLE_ITEM> items;
		UNONE::ObParseToNtPathW(file.toStdWString(), path);
		UNONE::StrLowerW(path);
		ArkDrvApi::Storage::UnlockEnum(path, items);
		for (auto item : items) {
			auto pid = (DWORD)item.pid;
			auto &&ppath = UNONE::PsGetProcessPathW(pid);
			auto &&pname = UNONE::FsPathToNameW(ppath);
			std::wstring fpath;
			UNONE::ObParseToDosPathW(item.name, fpath);
			auto item_pname = new QStandardItem(LoadIcon(WStrToQ(ppath)), WStrToQ(pname));
			auto item_pid = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"%d", pid)));
			auto item_fpath = new QStandardItem(WStrToQ(fpath));
			auto item_fobj = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"0x%p", item.object)));
			auto item_ppath = new QStandardItem(WStrToQ(ppath));
			auto item_fhandle = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"0x%08X", (DWORD)item.handle)));
			
			auto count = unlock_model_->rowCount();
			unlock_model_->setItem(count, 0, item_pname);
			unlock_model_->setItem(count, 1, item_pid);
			unlock_model_->setItem(count, 2, item_fpath);
			unlock_model_->setItem(count, 3, item_ppath);
			unlock_model_->setItem(count, 4, item_fobj);
			unlock_model_->setItem(count, 5, item_fhandle);
		}
	});

	connect(ui_->unlockFileBtn, &QPushButton::clicked, [&]{
		DISABLE_RECOVER();
		auto selected = ui_->unlockView->selectionModel()->selectedIndexes();
		if (selected.empty()) {
			return;
		}
		for (int i = 0; i < selected.size() / 6; i++) {
			auto pname = ui_->unlockView->model()->itemData(selected[i * 6]).values()[0].toString();
			auto pid = ui_->unlockView->model()->itemData(selected[i * 6 + 1]).values()[0].toUInt();
			auto fpath = ui_->unlockView->model()->itemData(selected[i * 6 + 2]).values()[0].toString();
			auto ppath = ui_->unlockView->model()->itemData(selected[i * 6 + 3]).values()[0].toString();
			auto fobj = ui_->unlockView->model()->itemData(selected[i * 6 + 4]).values()[0].toString();
			auto fhandle = ui_->unlockView->model()->itemData(selected[i * 6 + 5]).values()[0].toString();
			HANDLE_ITEM handle_item = {0};
			handle_item.pid = HANDLE(pid);
			QString qshandle = fhandle.replace(QRegExp("0x"), "");
			handle_item.handle = HANDLE(UNONE::StrToHexA(qshandle.toStdString().c_str()));
			ArkDrvApi::Storage::UnlockClose(handle_item);
		}
	});
	

}

void KernelStorage::InitFileFilterView()
{
	fsflt_model_ = new QStandardItemModel;
	fsflt_model_->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Value"));
	SetDefaultTreeViewStyle(ui_->fsfltView, fsflt_model_);
	ui_->fsfltView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui_->fsfltView->viewport()->installEventFilter(this);
	ui_->fsfltView->installEventFilter(this);
	fsflt_menu_ = new QMenu();
	fsflt_menu_->addAction(tr("ExpandAll"), this, SLOT(onExpandAll()));
}

void KernelStorage::ShowUnlockFiles()
{

}