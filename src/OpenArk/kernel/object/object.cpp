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
#include "object.h"
#include <arkdrv-api/arkdrv-api.h>

bool ObjectTypesSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const {
	auto s1 = sourceModel()->data(left); auto s2 = sourceModel()->data(right);
	return QString::compare(s1.toString(), s2.toString(), Qt::CaseInsensitive) < 0;
}


KernelObject::KernelObject()
{

}

KernelObject::~KernelObject()
{

}

void KernelObject::onTabChanged(int index)
{
	CommonTabObject::onTabChanged(index);
}

bool KernelObject::eventFilter(QObject *obj, QEvent *e)
{
	if (e->type() == QEvent::ContextMenu) {
		QMenu *menu = nullptr;
		if (obj == ui_->driverView->viewport()) menu = objtypes_menu_;
		QContextMenuEvent *ctxevt = dynamic_cast<QContextMenuEvent*>(e);
		if (ctxevt && menu) {
			menu->move(ctxevt->globalPos());
			menu->show();
		}
	}
	return QWidget::eventFilter(obj, e);
}

void KernelObject::ModuleInit(Ui::Kernel *ui, Kernel *kernel)
{
	this->ui_ = ui;

	Init(ui->tabStorage, TAB_KERNEL, TAB_KERNEL_STORAGE);

	InitObjectTypesView();
	InitFileFilterView();
}

void KernelObject::InitObjectTypesView()
{
	objtypes_model_ = new QStandardItemModel;
	QTreeView *view = ui_->objectTypesView;
	proxy_unlock_ = new ObjectTypesSortFilterProxyModel(view);
	proxy_unlock_->setSourceModel(objtypes_model_);
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
		{ 150, tr("TypeObject") },
		{ 50, tr("TypeIndex") },
		{ 150, tr("TypeName") },
		{ 50, tr("TotalObjectsNum") },
		{ 50, tr("TotalHandlesNum") },
	};
	QStringList name_list;
	for (auto p : colum_layout)  name_list << p.second;
	objtypes_model_->setHorizontalHeaderLabels(name_list);
	for (int i = 0; i < _countof(colum_layout); i++) {
		view->setColumnWidth(i, colum_layout[i].first);
	}
	objtypes_menu_ = new QMenu();
	objtypes_menu_->addAction(tr("Refresh"), this, [&] {});

	ShowObjectTypes();
}

void KernelObject::InitFileFilterView()
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

void KernelObject::ShowObjectTypes()
{
	DISABLE_RECOVER();
	ClearItemModelData(objtypes_model_, 0);

	QString file = ui_->inputPathEdit->text();
	std::wstring path;
	std::vector<OBJECT_TYPE_ITEM> items;
	ArkDrvApi::Object::ObjectTypeEnum(items);
	for (auto item : items) {
		auto item_0 = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"0x%p", item.type_object)));
		auto item_1 = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"%d", item.type_index)));
		auto item_2 = new QStandardItem(WStrToQ(item.type_name));
		auto item_3 = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"%d", item.total_objects)));
		auto item_4 = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"%d", item.total_handles)));
		auto count = objtypes_model_->rowCount();
		objtypes_model_->setItem(count, 0, item_0);
		objtypes_model_->setItem(count, 1, item_1);
		objtypes_model_->setItem(count, 2, item_2);
		objtypes_model_->setItem(count, 3, item_3);
		objtypes_model_->setItem(count, 4, item_4);
	}
}