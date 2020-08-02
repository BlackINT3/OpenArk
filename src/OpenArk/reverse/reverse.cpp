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

enum {
	RUN_EXE,
	RUN_DIR,
	RUN_CMD_DIR
};

struct {
	int type;
	QString name;
	QString exec;
	QString uri;
} WinReverseTools[] = {
	{ RUN_EXE, "procexp", "procexp/procexp.exe", "procexp.zip" },
	{ RUN_EXE, "procmon", "Procmon/Procmon.exe", "Procmon.zip" },
	{ RUN_EXE, "pchunter32", "PCHunter/PCHunter32.exe", "PCHunter32.zip" },
	{ RUN_EXE, "pchunter64", "PCHunter/PCHunter64.exe", "PCHunter64.zip" },
	{ RUN_EXE, "winobj", "Winobj/Winobj.exe", "Winobj.zip" },
	{ RUN_EXE, "dbgview", "Dbgview/Dbgview.exe", "Dbgview.zip" },
	{ RUN_EXE, "apimonitor32", "API Monitor/apimonitor-x86.exe", "API Monitor.zip" },
	{ RUN_EXE, "apimonitor64", "API Monitor/apimonitor-x64.exe", "API Monitor.zip" },
	{ RUN_CMD_DIR, "sysinternals", "SysinternalsSuite/", "SysinternalsSuite.zip" },
	{ RUN_EXE, "nirsoft", "nirsoft_package/NirLauncher.exe", "nirsoft_package.zip" },

	{ RUN_EXE, "windbg32", "Windbg/x86/windbg.exe", "Windbg32.zip" },
	{ RUN_EXE, "windbg64", "Windbg/x64/windbg.exe", "Windbg64.zip" },
	{ RUN_EXE, "x64dbg32", "x64dbg/x32/x32dbg.exe", "x64dbg.zip" },
	{ RUN_EXE, "x64dbg64", "x64dbg/x64/x64dbg.exe", "x64dbg.zip" },
	{ RUN_EXE, "ida32", "IDA/ida.exe", "IDA.zip" },
	{ RUN_EXE, "ida64", "IDA/ida64.exe", "IDA.zip" },
	{ RUN_EXE, "ollydbg", "OllyDBG/OllyDBG.exe", "OllyDBG.zip" },
	{ RUN_EXE, "ollyice", "OllyICE/OllyICE.exe", "OllyICE.zip" },
	{ RUN_EXE, "od52pj", "OD 52pj/OD.exe", "OD 52pj.zip" },

	{ RUN_EXE, "exeinfope", "ExeinfoPe/exeinfope.exe", "ExeinfoPe.zip" },
	{ RUN_EXE, "reshacker", "ResourceHacker/ResourceHacker.exe", "ResourceHacker.zip" },
	{ RUN_EXE, "cffexplorer", "CFF Explorer/CFF Explorer.exe", "CFF Explorer.zip" },
	{ RUN_EXE, "cheatengine", "Cheat Engine/Cheat Engine.exe", "Cheat Engine.zip" },
	{ RUN_EXE, "peid", "PEID/PEID.exe", "PEID.zip" },
	{ RUN_EXE, "hcd", "HCD/HCD.exe", "HCD.zip" },
	{ RUN_CMD_DIR, "radare", "radare2/bin/", "radare2.zip" },


	{ RUN_EXE, "notepadxx", "Notepad++/notepad++.exe", "Notepadxx.zip" },
	{ RUN_EXE, "editor010", "010Editor/010Editor.exe", "010Editor.zip" },
	{ RUN_EXE, "winhex", "Winhex/winhex.exe", "Winhex.zip" },
	{ RUN_EXE, "hxd", "HxD/HxD.exe", "HxD.zip" },

	{ RUN_EXE, "winspy", "WinSpy/WinSpy.exe", "WinSpy.zip" },
	{ RUN_EXE, "spyxx32", "Spy++/spyxx.exe", "Spyxx32.zip" },
	{ RUN_EXE, "spyxx64", "Spy++/spyxx_amd64.exe", "Spyxx64.zip" },
	{ RUN_EXE, "fiddler2", "Fiddler2/Fiddler.exe", "Fiddler2.zip" },
	{ RUN_EXE, "fiddler4", "Fiddler4/Fiddler.exe", "Fiddler4.zip" },
	{ RUN_EXE, "wiresharkv1", "Wireshark/Wireshark-win32-1.10.14.exe", "Wireshark-v1.zip" },
	{ RUN_EXE, "wiresharkv3", "Wireshark/Wireshark-win32-3.2.3.exe", "Wireshark-v3.zip" },
	{ RUN_EXE, "everything", "Everything/Everything.exe", "Everything.zip" },
	{ RUN_EXE, "teamviewer", "Teamviewer/Teamviewer.exe", "Teamviewer.zip" },
	{ RUN_EXE, "anydesk", "AnyDesk/AnyDesk.exe", "AnyDesk.zip" },
	{ RUN_EXE, "sunlogin", "Sunlogin/SunloginClient_10.3.0.27372.exe", "Sunlogin.zip" },
};

Reverse::Reverse(QWidget *parent, int tabid) :
	parent_((OpenArk*)parent)
{
	ui.setupUi(this);
	ui.progressBar->setValue(0);
	ui.progressBar->show();

	InitWinReverseToolsView();

	CommonMainTabObject::Init(ui.tabWidget, tabid);
}

Reverse::~Reverse()
{
}

void Reverse::onTabChanged(int index)
{
	CommonMainTabObject::onTabChanged(index);
}

void Reverse::onExecute()
{
	QString name;

	auto sender = QObject::sender();
	name = sender->objectName().replace("Btn", "");

	if (sender == ui.pchunterBtn) {
		name = UNONE::OsIs64() ? "pchunter64" : "pchunter32";
	}

	int type;
	QString uri, exec;
	for (int i = 0; i < _countof(WinReverseTools); i++)	{
		if (WinReverseTools[i].name == name) {
			uri = WinReverseTools[i].uri;
			exec = WinReverseTools[i].exec;
			type = WinReverseTools[i].type;
			break;
		}
	}
	auto &&filebase = WStrToQ(AppConfigDir() + L"/files/");
	if (!UNONE::FsIsExistedW(filebase.toStdWString())) {
		UNONE::FsCreateDirW(filebase.toStdWString());
	}
	auto &&file = filebase + uri;
	auto &&url = AppFsUrl() + "/" + uri;
	exec = filebase + exec;
	DownloadAndExecuteFile(type, file, exec, url);
}

void ShellRunCmdDir(QString dir)
{
	auto cmdline = "cmd /k cd /D" + dir;
	UNONE::PsCreateProcessW(cmdline.toStdWString());
}

void Reverse::DownloadAndExecuteFile(int type, QString path, QString exe, QString url)
{
	auto Run = [&](int type, QString exe)->bool {
		if (UNONE::FsIsExistedW(QToWStr(exe))) {
			if (type == RUN_EXE)
				ShellRun(exe, "");
			else if (type == RUN_CMD_DIR)
				ShellRunCmdDir(exe);
			else if (type == RUN_DIR)
				ExploreFile(exe);
			return true;
		}
		return false;
	};

	static bool pending = false;
	
	if (Run(type, exe))
		return;

	if (pending) {
		QMessageBox::critical(NULL, tr("Error"), tr("Download pending, wait for a while..."));
		return;
	}
	pending = true;

	file = new QFile(path);
	file->open(QIODevice::WriteOnly);

	QNetworkAccessManager *accessmgr = new QNetworkAccessManager(this);
	accessmgr->setNetworkAccessible(QNetworkAccessManager::Accessible);
	QUrl qurl(url);

	QNetworkRequest request(qurl);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
	reply = accessmgr->get(request);

	connect((QObject *)reply, SIGNAL(readyRead()), this, SLOT(readContent()));
	connect(accessmgr, &QNetworkAccessManager::finished, [&, Run, type, path, exe](QNetworkReply*) {
		if (reply->error() != QNetworkReply::NoError) {
			QMessageBox::critical(NULL, tr("Error"), tr("Download failed, err:%1").arg(reply->error()));
			ui.progressBar->setValue(0);
			ui.progressBar->setMaximum(100);
			file->close();
			DeleteFileW(QToWStr(path).c_str());
			pending = false;
			return;
		}
		reply->deleteLater();
		file->flush();
		file->close();

		//Unpack
		auto filepath = path.toStdString();
		auto dir = UNONE::FsPathToDirA(filepath);
		ZipUtils::UnpackToDir(filepath, ZipUtils::UNPACK_CURRENT, dir);
	
		//Run
		Run(type, exe);

		//Clean
		DeleteFileA(filepath.c_str());

		pending = false;
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
	QList<QPushButton*> buttons = ui.groupBox->findChildren<QPushButton*>();
	for (auto &btn : buttons) {
		connect(btn, SIGNAL(clicked()), this, SLOT(onExecute()));
	}

	connect(ui.toolsfolderBtn, &QPushButton::clicked, [] {
		auto folder = AppConfigDir() + L"\\files";
		ShellRun(WStrToQ(folder), "");
	});
}
