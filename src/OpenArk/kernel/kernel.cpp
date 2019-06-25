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
#include "kernel.h"
#include "../common/common.h"
#include "../openark/openark.h"

#define KernelTabDrivers 0

struct {
	int s = 0;
	int name = s++;
	int base = s++;
	int path = s++;
	int number = s++;
	int desc = s++;
	int ver = s++;
	int corp = s++;
} DRV;

bool DriversSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
	bool ok;
	auto s1 = sourceModel()->data(left); auto s2 = sourceModel()->data(right);
	auto column = left.column();
	if ((column == DRV.base || column == DRV.number)) 
		return s1.toString().toULongLong(&ok, 16) < s2.toString().toULongLong(&ok, 16);
	return QString::compare(s1.toString(), s2.toString(), Qt::CaseInsensitive) < 0;
}

Kernel::Kernel(QWidget* parent) :
	parent_((OpenArk*)parent)
{
	ui.setupUi(this);
	ui.tabWidget->setTabPosition(QTabWidget::West);
	ui.tabWidget->tabBar()->setStyle(new OpenArkTabStyle);
	setAcceptDrops(true);

	drivers_model_ = new QStandardItemModel;
	QTreeView *dview = ui.driverView;
	proxy_drivers_ = new DriversSortFilterProxyModel(dview);
	proxy_drivers_->setSourceModel(drivers_model_);
	proxy_drivers_->setDynamicSortFilter(true);
	proxy_drivers_->setFilterKeyColumn(1);
	dview->setModel(proxy_drivers_);
	dview->selectionModel()->setModel(proxy_drivers_);
	dview->header()->setSortIndicator(-1, Qt::AscendingOrder);
	dview->setSortingEnabled(true);
	dview->viewport()->installEventFilter(this);
	dview->installEventFilter(this);
	drivers_model_->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Base") << tr("Path") << tr("Number") << tr("Description") << tr("Version") << tr("Company"));
	dview->setColumnWidth(DRV.name, 138);
	dview->setColumnWidth(DRV.base, 135);
	dview->setColumnWidth(DRV.path, 285);
	dview->setColumnWidth(DRV.number, 60);
	dview->setColumnWidth(DRV.desc, 180);
	dview->setColumnWidth(DRV.corp, 155);
	dview->setColumnWidth(DRV.ver, 120);
	dview->setEditTriggers(QAbstractItemView::NoEditTriggers);
	drivers_menu_ = new QMenu();
	drivers_menu_->addAction(tr("Refresh"), this, [&] { ShowDrivers(); });
	drivers_menu_->addAction(tr("Explore File"), this, [&] { 
		auto path = DriversCurViewItemData(DRV.path);
		ExploreFile(path);
	});
	drivers_menu_->addAction(tr("Sendto Scanner"), this, [&] { 
		auto path = DriversCurViewItemData(DRV.path);
		parent_->ActivateTab(TAB_SCANNER);
		emit signalOpen(path);
	});
	drivers_menu_->addAction(tr("Properties..."), this, [&]() {
		auto path = DriversCurViewItemData(DRV.path);
		WinShowProperties(path.toStdWString());
	});

	connect(ui.installBtn, &QPushButton::clicked, this, [&]() {
		InstallDriver(ui.driverFileEdit->text());
	});
	connect(ui.browseBtn, &QPushButton::clicked, this, [&]() {
		QString file = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("Driver Files (*.sys);;All Files (*.*)"));
		onOpenFile(file);
	});
	connect(ui.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onTabChanged(int)));
	connect(this, SIGNAL(signalOpen(QString)), parent_, SLOT(onOpen(QString)));
}

Kernel::~Kernel()
{
}


bool Kernel::eventFilter(QObject *obj, QEvent *e)
{
	bool filtered = false;
	if (obj == ui.driverView->viewport()) {
		if (e->type() == QEvent::ContextMenu) {
			QContextMenuEvent* ctxevt = dynamic_cast<QContextMenuEvent*>(e);
			if (ctxevt != nullptr) {
				drivers_menu_->move(ctxevt->globalPos());
				drivers_menu_->show();
			}
		}
	}
	if (filtered) {
		dynamic_cast<QKeyEvent*>(e)->ignore();
		return true;
	}
	return QWidget::eventFilter(obj, e);
}

void Kernel::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->hasFormat("text/uri-list"))
		event->acceptProposedAction();
}

void Kernel::dropEvent(QDropEvent *event)
{
	if (!event->mimeData()->hasUrls())
		return;
	QString& path = event->mimeData()->urls()[0].toLocalFile();
	onOpenFile(path);
}

void Kernel::onOpenFile(QString path)
{
	if (!UNONE::FsIsFileW(path.toStdWString()))
		return;
	ui.driverFileEdit->setText(path);
	auto &&name = UNONE::FsPathToPureNameW(path.toStdWString());
	ui.serviceEdit->setText(WStrToQ(name));
}

void Kernel::onTabChanged(int index)
{
	if (index == KernelTabDrivers) {
		ShowDrivers();
	}
}

void Kernel::InstallDriver(QString driver)
{
	if (driver.isEmpty()) return;
	auto &&path = driver.toStdWString();
	auto &&name = UNONE::FsPathToPureNameW(path);
	if (UNONE::ObLoadDriverW(path, name)) {
		ui.infoLabel->setText(tr("Install ok..."));
		ui.infoLabel->setStyleSheet("color:green");
	} else {
		ui.infoLabel->setText(tr("Install failed, open console window to view detail..."));
		ui.infoLabel->setStyleSheet("color:red");
	}
}

void Kernel::ShowDrivers()
{
	DISABLE_RECOVER();
	ClearItemModelData(drivers_model_, 0);

	std::vector<LPVOID> drivers;
	UNONE::ObGetDriverList(drivers);
	int number = 0;
	for (auto d : drivers) {
		auto &&path = WStrToQ(UNONE::ObGetDriverPathW(d));
		auto &&name = WStrToQ(UNONE::ObGetDriverNameW(d));
		if (name.compare("ntdll.dll", Qt::CaseInsensitive) == 0 ||
			name.compare("smss.exe", Qt::CaseInsensitive) == 0 ||
			name.compare("apisetschema.dll", Qt::CaseInsensitive) == 0) {
			continue;
		}
		bool existed = true;
		auto info = CacheGetFileBaseInfo(path);
		if (info.desc.isEmpty()) {
			if (!UNONE::FsIsExistedW(info.path.toStdWString())) {
				info.desc = tr("[-] Driver file not existed!");
				existed = false;
			}
		}
		QStandardItem *name_item = new QStandardItem(name);
		QStandardItem *base_item = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"0x%llX", d)));
		QStandardItem *path_item = new QStandardItem(path);
		QStandardItem *number_item = new QStandardItem(QString("%1").arg(number));
		QStandardItem *desc_item = new QStandardItem(info.desc);
		QStandardItem *ver_item = new QStandardItem(info.ver);
		QStandardItem *corp_item = new QStandardItem(info.corp);

		auto count = drivers_model_->rowCount();
		drivers_model_->setItem(count, DRV.name, name_item);
		drivers_model_->setItem(count, DRV.base, base_item);
		drivers_model_->setItem(count, DRV.path, path_item);
		drivers_model_->setItem(count, DRV.number, number_item);
		drivers_model_->setItem(count, DRV.desc, desc_item);
		drivers_model_->setItem(count, DRV.ver, ver_item);
		drivers_model_->setItem(count, DRV.corp, corp_item);
		if (!existed) SetLineBgColor(drivers_model_, count, Qt::red);
		number++;
	}
}

int Kernel::DriversCurRow()
{
	auto idx = ui.driverView->currentIndex();
	return idx.row();
}

QString Kernel::DriversCurViewItemData(int column)
{
	return GetCurItemViewData(ui.driverView, column);
}
