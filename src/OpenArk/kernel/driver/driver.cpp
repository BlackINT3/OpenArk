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
#include "../../common/common.h"
#include "../../../OpenArkDrv/arkdrv-api/arkdrv-api.h"
#include <Windows.h>
#include <tchar.h>
#include "driver.h"
#include "Wincrypt.h"  
#pragma comment(lib, "Crypt32.lib")

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

KernelDriver::KernelDriver()
{

}

KernelDriver::~KernelDriver()
{

}

void KernelDriver::onTabChanged(int index)
{
	switch (index) {
	case TAB_KERNEL_DRIVER_LIST:
		ShowDrivers();
		break;
	default:
		break;
	}
	CommonTabObject::onTabChanged(index);
}

bool KernelDriver::eventFilter(QObject *obj, QEvent *e)
{
	if (e->type() == QEvent::ContextMenu) {
		QMenu *menu = nullptr;
		if (obj == ui->driverView->viewport()) menu = drivers_menu_;
		QContextMenuEvent *ctxevt = dynamic_cast<QContextMenuEvent*>(e);
		if (ctxevt && menu) {
			menu->move(ctxevt->globalPos());
			menu->show();
		}
	}
	return QWidget::eventFilter(obj, e);
}

void KernelDriver::ModuleInit(Ui::Kernel *mainui, Kernel *kernel)
{
	this->ui = mainui;
	this->kernel_ = kernel;

	Init(ui->tabDriver, TAB_KERNEL, TAB_KERNEL_DRIVER);

	InitDriversView();
	InitDriverKitView();
}

void KernelDriver::InitDriversView()
{
	drivers_model_ = new QStandardItemModel;
	QTreeView *view = ui->driverView;
	proxy_drivers_ = new DriversSortFilterProxyModel(view);
	proxy_drivers_->setSourceModel(drivers_model_);
	proxy_drivers_->setDynamicSortFilter(true);
	proxy_drivers_->setFilterKeyColumn(1);
	view->setModel(proxy_drivers_);
	view->selectionModel()->setModel(proxy_drivers_);
	view->header()->setSortIndicator(-1, Qt::AscendingOrder);
	view->setSortingEnabled(true);
	view->viewport()->installEventFilter(this);
	view->installEventFilter(this);
	drivers_model_->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Base") << tr("Path") << tr("Number") << tr("Description") << tr("Version") << tr("Company"));
	view->setColumnWidth(DRV.name, 138);
	view->setColumnWidth(DRV.base, 135);
	view->setColumnWidth(DRV.path, 285);
	view->setColumnWidth(DRV.number, 60);
	view->setColumnWidth(DRV.desc, 180);
	//dview->setColumnWidth(DRV.corp, 155);
	view->setColumnWidth(DRV.ver, 120);
	view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	drivers_menu_ = new QMenu();
	drivers_menu_->addAction(tr("Refresh"), this, [&] { ShowDrivers(); });
	drivers_menu_->addAction(tr("Copy"), this, [&] {
		ClipboardCopyData(DriversItemData(GetCurViewColumn(ui->driverView)).toStdString());
	});
	drivers_menu_->addAction(tr("Sendto Scanner"), this, [&] {
		kernel_->GetParent()->SetActiveTab(TAB_SCANNER);
		emit kernel_->signalOpen(DriversItemData(DRV.path));
	});
	drivers_menu_->addAction(tr("Explore File"), this, [&] {
		ExploreFile(DriversItemData(DRV.path));
	});
	drivers_menu_->addAction(tr("Properties..."), this, [&] {
		WinShowProperties(DriversItemData(DRV.path).toStdWString());
	});
}

void KernelDriver::InitDriverKitView()
{
	ui->groupWDF->setVisible(false);
	connect(ui->browseBtn, &QPushButton::clicked, this, [&]() {
		QString file = QFileDialog::getOpenFileName(kernel_, tr("Open File"), "", tr("Driver Files (*.sys);;All Files (*.*)"));
		kernel_->onOpenFile(file);
	});
	connect(ui->signBtn, SIGNAL(clicked()), this, SLOT(onSignDriver()));
	connect(ui->installNormallyBtn, SIGNAL(clicked()), this, SLOT(onInstallNormallyDriver()));
	connect(ui->installUnsignedBtn, SIGNAL(clicked()), this, SLOT(onInstallUnsignedDriver()));
	connect(ui->installExpiredBtn, SIGNAL(clicked()), this, SLOT(onInstallExpiredDriver()));
	connect(ui->uninstallBtn, SIGNAL(clicked()), this, SLOT(onUninstallDriver()));
}


void KernelDriver::onSignDriver()
{
	QString driver = ui->driverFileEdit->text();
	if (SignExpiredDriver(driver)) {
		ui->infoLabel->setText(tr("Sign ok..."));
		ui->infoLabel->setStyleSheet("color:green");
	}
	else {
		ui->infoLabel->setText(tr("Sign failed, open console window to view detail..."));
		ui->infoLabel->setStyleSheet("color:red");
	}
}

void KernelDriver::onInstallNormallyDriver()
{
	if (InstallDriver(ui->driverFileEdit->text(), ui->serviceEdit->text())) {
		ui->infoLabel->setText(tr("Install ok..."));
		ui->infoLabel->setStyleSheet("color:green");
	}
	else {
		ui->infoLabel->setText(tr("Install failed, open console window to view detail..."));
		ui->infoLabel->setStyleSheet("color:red");
	}
}

void KernelDriver::onInstallUnsignedDriver()
{
	onSignDriver();
	RECOVER_SIGN_TIME();
	onInstallNormallyDriver();
}

void KernelDriver::onInstallExpiredDriver()
{
	RECOVER_SIGN_TIME();
	onInstallNormallyDriver();
}

void KernelDriver::onUninstallDriver()
{
	if (UninstallDriver(ui->serviceEdit->text())) {
		ui->infoLabel->setText(tr("Uninstall ok..."));
		ui->infoLabel->setStyleSheet("color:green");
	}
	else {
		ui->infoLabel->setText(tr("Uninstall failed, open console window to view detail..."));
		ui->infoLabel->setStyleSheet("color:red");
	}
}

bool KernelDriver::InstallDriver(QString driver, QString name)
{
	if (driver.isEmpty()) {
		QERR_W("driver path is empty");
		return false;
	}
	auto &&path = driver.toStdWString();
	return UNONE::ObLoadDriverW(path, name.toStdWString());
}

bool KernelDriver::UninstallDriver(QString service)
{
	if (service.isEmpty()) {
		QERR_W("service is empty");
		return false;
	}
	return UNONE::ObUnloadDriverW(service.toStdWString());
}

void KernelDriver::ShowDrivers()
{
	DISABLE_RECOVER();
	ClearItemModelData(drivers_model_, 0);

	std::vector<LPVOID> drivers;
	UNONE::ObGetDriverList(drivers);
	int number = 0;
	for (auto d : drivers) {
		static int major = UNONE::OsMajorVer();
		auto &&w_path = UNONE::ObGetDriverPathW(d);
		if (major <= 5) {
			if (UNONE::StrIndexIW(w_path, L"\\Windows") == 0) {
				static auto &&drive = UNONE::OsEnvironmentW(L"%SystemDrive%");
				w_path = drive + w_path;
			}
			else if (w_path.find(L'\\') == std::wstring::npos && w_path.find(L'/') == std::wstring::npos) {
				static auto &&driverdir = UNONE::OsSystem32DirW() + L"\\drivers\\";
				w_path = driverdir + w_path;
			}
		}

		auto &&path = WStrToQ(w_path);
		auto &&name = WStrToQ(UNONE::ObGetDriverNameW(d));

		bool microsoft = true;
		bool existed = true;
		auto info = CacheGetFileBaseInfo(path);
		if (info.desc.isEmpty()) {
			if (!UNONE::FsIsExistedW(info.path.toStdWString())) {
				info.desc = tr("[-] Driver file not existed!");
				existed = false;
			}
		}
		if (!info.corp.contains("Microsoft", Qt::CaseInsensitive)) { microsoft = false; }

		auto name_item = new QStandardItem(name);
		auto base_item = new QStandardItem(WStrToQ(UNONE::StrFormatW(L"0x%p", d)));
		auto path_item = new QStandardItem(path);
		auto number_item = new QStandardItem(QString("%1").arg(number));
		auto desc_item = new QStandardItem(info.desc);
		auto ver_item = new QStandardItem(info.ver);
		auto corp_item = new QStandardItem(info.corp);

		auto count = drivers_model_->rowCount();
		drivers_model_->setItem(count, DRV.name, name_item);
		drivers_model_->setItem(count, DRV.base, base_item);
		drivers_model_->setItem(count, DRV.path, path_item);
		drivers_model_->setItem(count, DRV.number, number_item);
		drivers_model_->setItem(count, DRV.desc, desc_item);
		drivers_model_->setItem(count, DRV.ver, ver_item);
		drivers_model_->setItem(count, DRV.corp, corp_item);
		if (!existed) SetLineBgColor(drivers_model_, count, Qt::red);
		else if (!microsoft) SetLineBgColor(drivers_model_, count, QBrush(0xffffaa));
		number++;
	}
}

QString KernelDriver::DriversItemData(int column)
{
	return GetCurItemViewData(ui->driverView, column);
}


bool ImportPrivateKey(const std::string &private_key, WCHAR *passwd, WCHAR *category)
{
	bool ret = false;
	HCERTSTORE cert_store = NULL;
	HCERTSTORE cert_dst_store = NULL;
	PCCERT_CONTEXT cert = NULL;
	HCRYPTPROV crypt_prov = NULL;
	do {
		CRYPT_DATA_BLOB blob;
		blob.cbData = private_key.size();
		blob.pbData = (unsigned char *)private_key.c_str();
		cert_store = PFXImportCertStore(&blob, passwd, CRYPT_EXPORTABLE);
		if (!cert_store) {
			ERR("PFXImportCertStore err:%d", GetLastError());
			break;
		}
		DWORD key_spec = 0;
		BOOL free_prov = FALSE;
		cert = CertEnumCertificatesInStore(cert_store, NULL);
		if (!cert) {
			ERR("CertEnumCertificatesInStore err:%d", GetLastError());
			break;
		}
		if (!CryptAcquireCertificatePrivateKey(cert, CRYPT_ACQUIRE_COMPARE_KEY_FLAG, NULL, &crypt_prov, &key_spec, &free_prov) ||
			!crypt_prov) {
			ERR("CryptAcquireCertificatePrivateKey err:%d", GetLastError());
			break;
		}
		cert_dst_store = CertOpenStore(CERT_STORE_PROV_SYSTEM_W, 0, NULL, CERT_SYSTEM_STORE_CURRENT_USER | CERT_STORE_OPEN_EXISTING_FLAG, category);
		if (!cert_dst_store) {
			ERR("CertOpenStore err:%d", GetLastError());
			break;
		}
		ret = CertAddCertificateContextToStore(cert_dst_store, cert, CERT_STORE_ADD_REPLACE_EXISTING, NULL);
		if (!ret) {
			ERR("CertAddCertificateContextToStore err:%d", GetLastError());
			break;
		}
		ret = true;
	} while (0);

	if (cert) CertFreeCertificateContext(cert);
	if (crypt_prov) CryptReleaseContext(crypt_prov, 0);
	if (cert_dst_store) CertCloseStore(cert_dst_store, 0);
	if (cert_store) CertCloseStore(cert_store, 0);

	return ret;
}

bool SignExpiredDriver(QString driver)
{
	if (driver.isEmpty()) {
		QERR_W("driver is empty");
		return false;
	}

	QString res = ":/OpenArk/sign/CSignTool.pfx";
	QFile pfx(res);
	if (!pfx.open(QFileDevice::ReadOnly)) {
		QERR_W("open resource file %s err", QToWChars(res));
		return false;
	}
	if (!ImportPrivateKey(pfx.readAll().toStdString(), _T("TrustAsia.com"), _T("My"))) {
		QERR_W("import pfx certificate file %s err", QToWChars(res));
		return false;
	}

	auto &&toodir = UNONE::OsEnvironmentW(L"%AppData%\\TrustAsia\\DSignTool");
	auto &&signtool = toodir + L"\\CSignTool.exe";
	auto &&signcfg = toodir + L"\\Config.xml";

	ExtractResource(":/OpenArk/sign/CSignTool.exe", WStrToQ(signtool));
	ExtractResource(":/OpenArk/sign/Config.xml", WStrToQ(signcfg));

	RECOVER_SIGN_TIME();

	std::wstring cmdline;
	std::wstring &&path = driver.toStdWString();
	UNONE::StrFormatW(cmdline, L"%s sign /r Driver /f \"%s\" /ac", signtool.c_str(), path.c_str());
	PROCESS_INFORMATION pi;
	if (!UNONE::PsCreateProcessW(cmdline, SW_HIDE, &pi)) {
		QERR_W("run cmdline:%s err", cmdline.c_str());
		return false;
	}
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return true;
}

std::wstring ParseDriverPath(UCHAR *symlnk)
{
	std::wstring &&path = UNONE::StrToW((char*)symlnk);
	std::wstring sysroot = L"\\SystemRoot";
	auto pos = path.find(sysroot);
	if (pos == 0) path.replace(0, sysroot.size(), UNONE::OsWinDirW());
	UNONE::StrReplaceW(path, L"\\??\\");
	return path;
}