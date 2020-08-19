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
	auto column = left.column();
	if ((column == 1 || column == 3 || column == 4))
		return s1.toString().toULongLong(nullptr, 16) < s2.toString().toULongLong(nullptr, 16);
	return QString::compare(s1.toString(), s2.toString(), Qt::CaseInsensitive) < 0;
}

bool ObjectSectionsSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const {
	auto s1 = sourceModel()->data(left); auto s2 = sourceModel()->data(right);
	auto column = left.column(); bool ok;
	if ((column == 1 || column == 2))
		return s1.toString().toULongLong(nullptr, 16) < s2.toString().toULongLong(nullptr, 16);
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
		if (obj == ui_->objectTypesView->viewport()) menu = objtypes_menu_;
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

	Init(ui->tabObject, TAB_KERNEL, TAB_KERNEL_OBJECT);

	InitObjectTypesView();
	InitObjectSectionsView();
}

void KernelObject::InitObjectTypesView()
{
	QTreeView *view = ui_->objectTypesView;
	objtypes_model_ = new QStandardItemModel;
	proxy_objtypes_ = new ObjectTypesSortFilterProxyModel(view);
	std::pair<int, QString> colum_layout[] = {
		{ 170, tr("TypeObject") },
		{ 80, tr("TypeIndex") },
		{ 227, tr("TypeName") },
		{ 110, tr("TotalObjectsNum") },
		{ 110, tr("TotalHandlesNum") },
	};

	SetDefaultTreeViewStyle(view, objtypes_model_, proxy_objtypes_, colum_layout, _countof(colum_layout));
	view->viewport()->installEventFilter(this);
	view->installEventFilter(this);

	objtypes_menu_ = new QMenu();
	objtypes_menu_->addAction(tr("Refresh"), this, [&] {
		ShowObjectTypes();
	});
	objtypes_menu_->addAction(tr("Copy"), this, [&] {
		auto view = ui_->objectTypesView;
		ClipboardCopyData(GetCurItemViewData(view, GetCurViewColumn(view)).toStdString());
	});
	ShowObjectTypes();
}

void KernelObject::InitObjectSectionsView()
{
	QTreeView *view = ui_->objectSectionsView;
	objsections_model_ = new QStandardItemModel;
	proxy_objsections_ = new ObjectSectionsSortFilterProxyModel(view);
	std::pair<int, QString> colum_layout[] = {
		{ 170, tr("SectionName") },
		{ 80, tr("SectionSize") },
		{ 80, tr("Session") },
		{ 227, tr("SessionName") },
	};

	SetDefaultTreeViewStyle(view, objsections_model_, proxy_objsections_, colum_layout, _countof(colum_layout));
	view->viewport()->installEventFilter(this);
	view->installEventFilter(this);

	objsections_menu_ = new QMenu();
	objsections_menu_->addAction(tr("Refresh"), this, [&] {
		ShowObjectTypes();
	});
	objsections_menu_->addAction(tr("Copy"), this, [&] {
		auto view = ui_->objectTypesView;
		ClipboardCopyData(GetCurItemViewData(view, GetCurViewColumn(view)).toStdString());
	});

	ShowObjectTypes();
	ShowObjectSections();
}

void KernelObject::ShowObjectTypes()
{
	DISABLE_RECOVER();
	ClearItemModelData(objtypes_model_, 0);

	std::vector<ARK_OBJECT_TYPE_ITEM> items;
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

void KernelObject::ShowObjectSections()
{
	DISABLE_RECOVER();
	ClearItemModelData(objsections_model_, 0);

	std::vector<ARK_OBJECT_SECTION_ITEM> items;
	ArkDrvApi::Object::ObjectSectionEnum(items);
	for (auto item : items) {
		auto item_0 = new QStandardItem(WStrToQ(item.section_name));
		auto item_1 = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"%d", item.section_size)));
		auto item_2 = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"%d", item.session)));
		auto item_3 = new QStandardItem(WStrToQ(item.session_name));
		auto count = objsections_model_->rowCount();
		objsections_model_->setItem(count, 0, item_0);
		objsections_model_->setItem(count, 1, item_1);
		objsections_model_->setItem(count, 2, item_2);
		objsections_model_->setItem(count, 3, item_3);
	}
}