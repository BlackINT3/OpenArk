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
}

bool KernelStorage::EventFilter()
{
	return true;
}

void KernelStorage::ModuleInit(Ui::Kernel *ui, Kernel *kernel)
{
	unlockfile_model_ = new QStandardItemModel;
	QTreeView *view = ui->wfpView;
	proxy_unlockfile_ = new UnlockFileSortFilterProxyModel(view);
	proxy_unlockfile_->setSourceModel(unlockfile_model_);
	proxy_unlockfile_->setDynamicSortFilter(true);
	proxy_unlockfile_->setFilterKeyColumn(1);
	view->setModel(proxy_unlockfile_);
	view->selectionModel()->setModel(proxy_unlockfile_);
	view->header()->setSortIndicator(-1, Qt::AscendingOrder);
	view->setSortingEnabled(true);
	view->viewport()->installEventFilter(kernel);
	view->installEventFilter(kernel);
	std::pair<int, QString> colum_layout[] = {
		{ 130, tr("ID") },
		{ 100, tr("Key") },
		{ 200, tr("Name") },
	};
	QStringList name_list;
	for (auto p : colum_layout) {
		name_list << p.second;
	}
	unlockfile_model_->setHorizontalHeaderLabels(name_list);
	for (int i = 0; i < _countof(colum_layout); i++) {
		view->setColumnWidth(i, colum_layout[i].first);
	}
	view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	unlockfile_menu_ = new QMenu();
	unlockfile_menu_->addAction(tr("Refresh"), kernel, [&] {});

	connect(ui->tabNetwork, SIGNAL(currentChanged(int)), this, SLOT(onTabChanged(int)));
}

void KernelStorage::ShowUnlockFiles()
{
}