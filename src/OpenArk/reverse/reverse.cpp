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
#include "reverse.h"
#include "../common/common.h"
#include "../common/utils/compress/zip_utils.h"
#include "../openark/openark.h"
using namespace Plugin::Compressor;

struct {
	QString name;
	QString exec;
	QString uri;
} WinReverseTools[] = {
	{"procexp", "procexp/procexp.exe", "procexp.zip" },
	{"procmon", "procmon/procmon.exe", "procmon.zip" },
	{"winobj", "winobj/winobj.exe", "winobj.zip"},
	{"pchunter32", "pchunter/pchunter32.exe", "pchunter32.zip" },
	{"pchunter64", "pchunter/pchunter64.exe", "pchunter64.zip" },
	{"dbgview", "dbgview/dbgview.exe", "dbgview.zip" },
	{"winspy", "winspy/winspy.exe", "winspy.zip" },
	{"hxd", "hxd/hxd.exe", "hxd.zip" }
};

Reverse::Reverse(QWidget *parent) :
	parent_((OpenArk*)parent)
{
	ui.setupUi(this);
	ui.tabWidget->setTabPosition(QTabWidget::West);
	ui.tabWidget->tabBar()->setStyle(new OpenArkTabStyle);
	ui.progressBar->setValue(0);
	ui.progressBar->show();
	connect(ui.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onTabChanged(int)));

	InitWinReverseToolsView();
}

Reverse::~Reverse()
{
}

void Reverse::ActivateTab(int idx)
{
	ui.tabWidget->setCurrentIndex(idx);
}

void Reverse::onTabChanged(int index)
{
	OpenArkConfig::Instance()->SetPrefLevel2Tab(index);
}

void Reverse::onExecute()
{
	QString name;
	
	auto sender = QObject::sender();
	if (sender == ui.pchunterBtn)
		name = UNONE::OsIs64() ? "pchunter64" : "pchunter32";
	else if (sender == ui.procexpBtn)
		name = "procexp";
	else if (sender == ui.procmonBtn)
		name = "procmon";
	else if (sender == ui.winobjBtn)
		name = "winobj";
	else if (sender == ui.dbgviewBtn)
		name = "dbgview";
	else if (sender == ui.winspyBtn)
		name = "winspy";
	else if (sender == ui.hxdBtn)
		name = "hxd";

	QString uri, exec;
	for (int i = 0; i < _countof(WinReverseTools); i++)	{
		if (WinReverseTools[i].name == name) {
			uri = WinReverseTools[i].uri;
			exec = WinReverseTools[i].exec;
			break;
		}
	}
	auto &&filebase = WStrToQ(AppConfigDir() + L"/files/");
	if (!UNONE::FsIsExistedW(filebase.toStdWString())) {
		UNONE::FsCreateDirW(filebase.toStdWString());
	}
	auto &&file = filebase + uri;
	auto &&url = AppFsUrl() + "/" + uri;
	INFO(L"fsurl appver:%s", url.toStdWString().c_str());
	exec = filebase + exec;
	DownloadAndExecuteFile(file, exec, url);
}

void Reverse::DownloadAndExecuteFile(QString path, QString exec, QString url)
{
	if (UNONE::FsIsExistedW(QToWStr(exec))) {
		ShellRun(exec, "");
		return;
	}

	file = new QFile(path);
	file->open(QIODevice::WriteOnly);

	QNetworkAccessManager *accessManager = new QNetworkAccessManager(this);
	accessManager->setNetworkAccessible(QNetworkAccessManager::Accessible);
	QUrl qurl(url);

	QNetworkRequest request(qurl);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
	reply = accessManager->get(request);

	connect((QObject *)reply, SIGNAL(readyRead()), this, SLOT(readContent()));
	connect(accessManager, &QNetworkAccessManager::finished, [&, path, exec](QNetworkReply*) {
		if (reply->error() != QNetworkReply::NoError) {
			QMessageBox::critical(NULL, tr("Error"), tr("Download failed, err:%1").arg(reply->error()));
			ui.progressBar->setValue(0);
			ui.progressBar->setMaximum(100);
			file->close();
			DeleteFileW(QToWStr(path).c_str());
			return;
		}
		reply->deleteLater();
		file->flush();
		file->close();

		//Unpack
		auto filepath = path.toStdString();
		auto dir = UNONE::FsPathToDirA(filepath);
		ZipUtils::UnpackToDir(filepath, ZipUtils::UNPACK_CURRENT, dir);
		
		//Execute
		ShellRun(exec, "");

	});
	connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onProgress(qint64, qint64)));
}

void Reverse::readContent()
{
	file->write(reply->readAll());
}

void Reverse::onProgress(qint64 bytesSent, qint64 bytesTotal)
{
	ui.progressBar->setMaximum(bytesTotal);
	ui.progressBar->setValue(bytesSent);
}

void Reverse::InitWinReverseToolsView()
{
	connect(ui.procexpBtn, SIGNAL(clicked()), this, SLOT(onExecute()));
	connect(ui.procmonBtn, SIGNAL(clicked()), this, SLOT(onExecute()));
	connect(ui.winobjBtn, SIGNAL(clicked()), this, SLOT(onExecute()));
	connect(ui.pchunterBtn, SIGNAL(clicked()), this, SLOT(onExecute()));
	connect(ui.winspyBtn, SIGNAL(clicked()), this, SLOT(onExecute()));
	connect(ui.dbgviewBtn, SIGNAL(clicked()), this, SLOT(onExecute()));
	connect(ui.hxdBtn, SIGNAL(clicked()), this, SLOT(onExecute()));
}