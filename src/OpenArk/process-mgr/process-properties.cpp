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
#include "process-properties.h"
#include "../common/common.h"
#include "../common/cache/cache.h"

ProcessProperties::ProcessProperties(QWidget* parent, DWORD pid, int tab) :
	pid_(pid)
{
	setAttribute(Qt::WA_ShowModal, true);
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowFlags(windowFlags()& ~Qt::WindowMaximizeButtonHint);
	ui.setupUi(this);
	connect(OpenArkLanguage::Instance(), &OpenArkLanguage::languageChaned, this, [this]() {ui.retranslateUi(this); });

	std::wstring wstr = UNONE::PsGetProcessPathW(pid_);
	proc_path_ = WStrToQ(wstr);
	proc_name_ = WStrToQ(UNONE::FsPathToNameW(wstr));
	QString title = QString(tr("%1:%2 Properties")).arg(proc_name_).arg(pid_);
	setWindowTitle(title);
	setWindowIcon(LoadIcon(proc_path_));

	threads_model_ = new QStandardItemModel;
	wnds_model_ = new QStandardItemModel;
	threads_model_->setHorizontalHeaderLabels(QStringList() << tr("TID") << tr("KernelTime") << tr("UserTime") << tr("CreateTime"));
	wnds_model_->setHorizontalHeaderLabels(QStringList() << tr("HWND") << tr("Title") << tr("ClassName") << tr("Visible") << tr("TID") << tr("PID"));
	SetDefaultTreeViewStyle(ui.threadView, threads_model_);
	SetDefaultTreeViewStyle(ui.wndsView, wnds_model_);
	menu_ = new QMenu();
	menu_->addAction(tr("Refresh"), this, SLOT(onRefresh()));
	ui.threadView->installEventFilter(this);
	ui.wndsView->installEventFilter(this);


	connect(ui.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onTabChanged()));
	connect(ui.exploreButton, SIGNAL(clicked()), this, SLOT(onExploreFile()));
	ui.tabWidget->setCurrentIndex(tab);
	if (tab == 0) ShowImageDetail();
	else if (tab == 1) ShowThreads();
	else if (tab == 2) ShowWindowList();
	this->installEventFilter(this);

// 	timer_.setInterval(2000);
// 	timer_.start();
// 	connect(&timer_, SIGNAL(timeout()), this, SLOT(onTimer()));
}

ProcessProperties::~ProcessProperties()
{
//	timer_.stop();
}

bool ProcessProperties::eventFilter(QObject *obj, QEvent *e)
{
	if (obj == this) {
		if (e->type() == QEvent::KeyPress) {
			QKeyEvent* keyevt = dynamic_cast<QKeyEvent*>(e);
			if (keyevt->key() == Qt::Key_Escape) {
				this->close();
				return true;
			}
		}
	} else if (obj == ui.threadView || obj == ui.wndsView) {
		if (e->type() == QEvent::ContextMenu) {
			QContextMenuEvent* ctxevt = dynamic_cast<QContextMenuEvent*>(e);
			if (ctxevt != nullptr) {
				menu_->move(ctxevt->globalPos());
				menu_->show();
			}
		}
	}
	return QWidget::eventFilter(obj, e);
}

void ProcessProperties::onRefresh()
{
	onTabChanged();
}

void ProcessProperties::onTimer()
{
	onTabChanged();
}

void ProcessProperties::onTabChanged()
{
	auto curidx = ui.tabWidget->currentIndex();
	switch (curidx) {
	case 0:
		ShowImageDetail(); break;
	case 1:
		ShowThreads(); break;
	case 2:
		ShowWindowList(); break;
	}
}

void ProcessProperties::onExploreFile()
{
	ExploreFile(proc_path_);
}

void ProcessProperties::ShowImageDetail()
{
	ui.pathEdit->setText(proc_path_);
	ui.iconLabel->setPixmap(LoadIcon(proc_path_).pixmap(QSize(48, 48)));
	std::wstring prod_ver, file_ver, descript, copyright;
	std::wstring path = proc_path_.toStdWString();
	UNONE::FsGetFileInfoW(path, L"ProductVersion", prod_ver);
	UNONE::FsGetFileVersionW(path, file_ver);
	UNONE::FsGetFileInfoW(path, L"FileDescription", descript);
	UNONE::FsGetFileInfoW(path, L"LegalCopyright", copyright);
	ui.prodVerLabel->setText(WStrToQ(prod_ver));
	ui.fileVerLabel->setText(WStrToQ(file_ver));
	ui.descLabel->setText(WStrToQ(descript));
	ui.copyrightLabel->setText(WStrToQ(copyright));
	ui.bitsLabel->setText(UNONE::PsIsX64(pid_) ? WCharsToQ(L"64-bits") : WCharsToQ(L"32-bits"));
	
	auto image = UNONE::PeMapImageByPathW(path);
	if (image) {
		std::string cptime;
		auto stamp = UNONE::PeGetTimeStamp(image);
		if (stamp)
			cptime = UNONE::TmFormatUnixTimeA(stamp, "Y-M-D H:W:S");
		ui.buildLabel->setText(StrToQ(cptime));
		UNONE::PeUnmapImage(image);
	}
	auto info = CacheGetProcessBaseInfo(pid_);
	ui.cmdlineEdit->setText(WStrToQ(info.CommandLine));
	ui.curdirEdit->setText(WStrToQ(info.CurrentDirectory));
	auto ppid = UNONE::PsGetParentPid(pid_);
	auto pname = UNONE::PsGetProcessNameW(ppid);
	ui.parentLabel->setText(WStrToQ(UNONE::StrFormatW(L"%s(%d)", pname.c_str(), ppid)));;
	ui.userLabel->setText(WStrToQ(UNONE::OsHostNameW() + L"\\" + UNONE::OsUserNameW()));
	ui.startLabel->setText(WStrToQ(ProcessCreateTime(pid_)));
}

void ProcessProperties::ShowThreads()
{
	ClearItemModelData(threads_model_);
	std::vector<DWORD> tids;
	UNONE::PsGetAllThread(pid_, tids);
	for (auto tid : tids) {
		auto tid_item = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"%04X(%d)", tid, tid)));
		std::wstring ct, kt, ut;
		RetrieveThreadTimes(tid, ct, kt, ut);
		auto ct_item = new QStandardItem(WStrToQ(ct));
		auto kt_item = new QStandardItem(WStrToQ(kt));
		auto ut_item = new QStandardItem(WStrToQ(ut));
		auto count = threads_model_->rowCount();
		threads_model_->setItem(count, 0, tid_item);
		threads_model_->setItem(count, 1, kt_item);
		threads_model_->setItem(count, 2, ut_item);
		threads_model_->setItem(count, 3, ct_item);
	}
	QString qstr = WStrToQ(UNONE::StrFormatW(L"%d", tids.size()));
	ui.threadCountLabel->setText(qstr);
}

void ProcessProperties::ShowWindowList()
{
	ClearItemModelData(wnds_model_);
	std::vector<HWND> wnds;
	wnds = UNONE::PsGetWnds(pid_);
	for (auto wnd : wnds) {
		DWORD tid, pid;
		tid = GetWindowThreadProcessId(wnd, &pid);
		auto hwnd_item = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"%08X", wnd)));
		auto title_item = new QStandardItem(WStrToQ(UNONE::PsGetWndTextW(wnd)));
		auto class_item = new QStandardItem(WStrToQ(UNONE::PsGetWndClassNameW(wnd)));
		auto visible_item = new QStandardItem(WStrToQ(IsWindowVisible(wnd) ? L"+":L"-"));
		auto tid_item = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"%04X(%d)", tid, tid)));
		auto pid_item = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"%04X(%d)", pid, pid)));
		auto count = wnds_model_->rowCount();
		wnds_model_->setItem(count, 0, hwnd_item);
		wnds_model_->setItem(count, 1, title_item);
		wnds_model_->setItem(count, 2, class_item);
		wnds_model_->setItem(count, 3, visible_item);
		wnds_model_->setItem(count, 4, tid_item);
		wnds_model_->setItem(count, 5, pid_item);
	}
}