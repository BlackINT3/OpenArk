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
	auto column = left.column();
	if ((column == 2 || column == 3))
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
	switch (index) {
	case TAB_KERNEL_OBJECT_TYPES: ShowObjectTypes(); break;
	case TAB_KERNEL_OBJECT_SECTIONS: ShowObjectSections(); break;
	default: break;
	}
	CommonTabObject::onTabChanged(index);
}

bool KernelObject::eventFilter(QObject *obj, QEvent *e)
{
	if (e->type() == QEvent::ContextMenu) {
		QMenu *menu = nullptr;
		if (obj == ui_->objectTypesView->viewport()) menu = objtypes_menu_;
		if (obj == ui_->objectSectionsView->viewport()) menu = objsections_menu_;
		QContextMenuEvent *ctxevt = dynamic_cast<QContextMenuEvent*>(e);
		if (ctxevt && menu) {
			menu->move(ctxevt->globalPos());
			menu->show();
		}
	}
	if (e->type() == QEvent::KeyPress) {
		QKeyEvent *keyevt = dynamic_cast<QKeyEvent*>(e);
		if (keyevt->matches(QKeySequence::Refresh)) {
			ShowObjectTypes();
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
	std::vector<std::pair<int, QString>> layout = {
		{ 170, tr("TypeObject") },
		{ 80, tr("TypeIndex") },
		{ 227, tr("TypeName") },
		{ 110, tr("TotalObjectsNum") },
		{ 110, tr("TotalHandlesNum") },
	};

	SetDefaultTreeViewStyle(view, objtypes_model_, proxy_objtypes_, layout);
	view->viewport()->installEventFilter(this);
	view->installEventFilter(this);

	objtypes_menu_ = new QMenu();
	objtypes_menu_->addAction(tr("Refresh"), this, [&] {
		ShowObjectTypes();
	}, QKeySequence::Refresh);
	objtypes_menu_->addAction(tr("Copy"), this, [&] {
		auto view = ui_->objectTypesView;
		ClipboardCopyData(GetCurItemViewData(view, GetCurViewColumn(view)).toStdString());
	});
}

void KernelObject::InitObjectSectionsView()
{
	QTreeView *view = ui_->objectSectionsView;
	objsections_model_ = new QStandardItemModel;
	proxy_objsections_ = new ObjectSectionsSortFilterProxyModel(view);
	std::vector<std::pair<int, QString>> layout = {
		{ 220, tr("SectionDirectory") },
		{ 350, tr("SectionName") },
		{ 90, tr("SectionSize") },
		{ 80, tr("SessionID") },
		{ 80, tr("SessionName") },
	};
	SetDefaultTreeViewStyle(view, objsections_model_, proxy_objsections_, layout);
	view->viewport()->installEventFilter(this);
	view->installEventFilter(this);

	objsections_menu_ = new QMenu();
	objsections_menu_->addAction(tr("Refresh"), this, [&] {
		ShowObjectSections();
	});
	objsections_menu_->addAction(tr("Copy"), this, [&] {
		auto view = ui_->objectSectionsView;
		ClipboardCopyData(GetCurItemViewData(view, GetCurViewColumn(view)).toStdString());
	});

	auto GetSectionData = [&](QTreeView *view, ULONG64 &map_addr, ULONG &map_size, HANDLE &map_hd){
		map_size = map_addr = 0;
		std::string section_data;
		auto name = GetCurItemViewData(view, 1);
		auto size = VariantInt(GetCurItemViewData(view, 2).toStdString(), 16);
		auto session = GetCurItemViewData(view, 3);
		std::wstring prefix, section_name;
		std::wstring map_name;
		section_name = name.toStdWString();
		if (session.isEmpty()) {
			prefix = L"Global";
			map_name = UNONE::StrFormatW(L"%s\\%s", prefix.c_str(), section_name.c_str());
		} else {
			prefix = L"";
			map_name = section_name;
		}
		map_hd = OpenFileMappingW(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, map_name.c_str());
		if (!map_hd) {
			auto msg = UNONE::StrFormatW(L"OpenFileMappingW %s err:%d", map_name.c_str(), GetLastError());
			ERR(msg.c_str());
			QMessageBox::critical(NULL, tr("Error"), WStrToQ(msg));
			return;
		}
		map_addr = (ULONG64)MapViewOfFileEx(map_hd, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0, NULL);
		if (!map_addr) map_addr = (ULONG64)MapViewOfFileEx(map_hd, FILE_MAP_READ, 0, 0, 0, NULL);
		if (!map_addr) {
			auto msg = UNONE::StrFormatW(L"MapViewOfFileEx %s err:%d", map_name.c_str(), GetLastError());
			CloseHandle(map_hd);
			ERR(msg.c_str());
			QMessageBox::critical(NULL, tr("Error"), WStrToQ(msg));
			return;
		}
		map_size = size;
	};

	objsections_menu_->addAction(tr("Dump to File"), this, [&] {
		ULONG64 map_addr; ULONG map_size; HANDLE map_hd;
		GetSectionData(ui_->objectSectionsView, map_addr, map_size, map_hd);
		if (!map_addr) return;
		std::string data((char*)map_addr, map_size);
		QString filename = WStrToQ(UNONE::StrFormatW(L"%X_%X", map_addr, map_size));
		QString dumpmem = QFileDialog::getSaveFileName(this, tr("Save to"), filename, tr("DumpMemory(*)"));
		if (!dumpmem.isEmpty()) {
			UNONE::FsWriteFileDataW(dumpmem.toStdWString(), data) ?
				MsgBoxInfo(tr("Dump memory to file ok")) :
				MsgBoxError(tr("Dump memory to file error"));
		}
	});

	objsections_menu_->addAction(tr("Memory Edit"), this, [&] {
		ULONG64 map_addr; ULONG map_size; HANDLE map_hd;
		GetSectionData(ui_->objectSectionsView, map_addr, map_size, map_hd);
		if (!map_addr) return;
		auto memrw = new KernelMemoryRW();
		QList<QVariant> vars{ map_addr, (uint)map_hd};
		memrw->RegFreeCallback([&](QList<QVariant> vars) {
			PVOID addr = (PVOID)vars[0].toLongLong();
			HANDLE hd = (HANDLE)vars[1].toUInt();
			UnmapViewOfFile(addr);
			CloseHandle(hd);
		}, vars);
		memrw->SetMaxSize(map_size);
		map_size = MIN(map_size, PAGE_SIZE);
		memrw->ViewMemory(GetCurrentProcessId(), map_addr, map_size);
		memrw->OpenNewWindow(qobject_cast<QWidget*>(this->parent()), map_addr, map_size);
	});
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
		auto item_0 = new QStandardItem(WStrToQ(item.section_dir));
		auto item_1 = new QStandardItem(WStrToQ(item.section_name));
		auto item_2 = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"0x%x", item.section_size)));
		QString idx;
		if (item.session_id != ARK_SESSION_GLOBAL) idx = WStrToQ(UNONE::StrFormatW(L"%d", item.session_id));
		auto item_3 = new QStandardItem(idx);
		auto item_4 = new QStandardItem(WStrToQ(item.session_name));
		auto count = objsections_model_->rowCount();
		objsections_model_->setItem(count, 0, item_0);
		objsections_model_->setItem(count, 1, item_1);
		objsections_model_->setItem(count, 2, item_2);
		objsections_model_->setItem(count, 3, item_3);
		objsections_model_->setItem(count, 4, item_4);
	}
}