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
	RUN_EXE_BY_CMD,
	RUN_DIR,
	RUN_CMD_DIR,
	RUN_OPEN_URL,
};

WINTOOL_ITEM WinAllTools[] = {
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
	
	//2020.09.02 add
	{ RUN_EXE, "pchunternew32", "PCHunterNew/PCHunter32.exe", "PCHunterNew32.zip" },
	{ RUN_EXE, "pchunternew64", "PCHunterNew/PCHunter64.exe", "PCHunterNew64.zip" },
	{ RUN_EXE, "wke32", "WKE32.exe", "WKE32.exe" },
	{ RUN_EXE, "wke64", "WKE64.exe", "WKE64.exe" },
	{ RUN_EXE, "ghidra", "Ghidra/ghidraRun.bat", "Ghidra.zip" },
	{ RUN_EXE, "keygener", "Keygener.exe", "Keygener.exe" },
	{ RUN_EXE, "pygtools", "PYGTools/PYG_TOOLS_VER5.exe", "PYGTools.zip" },
	{ RUN_EXE, "poolmonx", "PoolMonX/PoolMonX.exe", "PoolMonX.zip" },
	{ RUN_EXE_BY_CMD, "diskgenius", "DiskGenius/DiskGenius.exe", "DiskGenius.zip" },	//cannot shell open diskgenius directly, WTF
	{ RUN_EXE, "window", "Window.exe", "Window.exe" },
	{ RUN_CMD_DIR, "curl", "network/curl/", "network/curl.zip" },
	{ RUN_EXE, "nmap", "network/nmap-setup.exe", "network/nmap-setup.exe" },
	{ RUN_EXE, "charles", "charles-win64.msi", "charles-win64.msi" },
	{ RUN_CMD_DIR, "tcpdump", "network/tcpdump/", "network/tcpdump.zip" },
	{ RUN_EXE, "x7z", "compressor/7z.exe", "compressor/7z.exe" },
	{ RUN_EXE, "winrar", "compressor/winrar.exe", "compressor/winrar.exe" },
	{ RUN_EXE, "chrome49", "browser/ChromeStandalone49.exe", "browser/ChromeStandalone49.exe" },
	{ RUN_EXE, "chrome85", "browser/ChromeStandalone85.exe", "browser/ChromeStandalone85.exe" },
	{ RUN_EXE, "firefox", "browser/FirefoxSetup.exe", "browser/FirefoxSetup.exe" },
	{ RUN_EXE, "cpuz", "cpuz/cpuz_x32.exe", "cpuz.zip" },
	{ RUN_EXE, "aida64", "aida64/aida64.exe", "aida64.zip" },

	//WinDevKits
	{ RUN_OPEN_URL, "jdk", "https://mirrors.huaweicloud.com/java/jdk/", "" },
	{ RUN_OPEN_URL, "python", "https://www.python.org/downloads/", "" },
	{ RUN_OPEN_URL, "golang", "https://studygolang.com/dl", "" },
	{ RUN_EXE, "git32", "dev/Git-32bit.exe", "dev/Git-32bit.exe" },
	{ RUN_EXE, "torgit32", "dev/TortoiseGit-32bit.msi", "dev/TortoiseGit-32bit.msi" },
	{ RUN_EXE, "torsvn32", "dev/TortoiseSVN-32bit.msi", "dev/TortoiseSVN-32bit.msi" },
	{ RUN_EXE, "git64", "dev/Git-64bit.exe", "dev/Git-64bit.exe" },
	{ RUN_EXE, "torgit64", "dev/TortoiseGit-64bit.msi", "dev/TortoiseGit-64bit.msi" },
	{ RUN_EXE, "torsvn64", "dev/TortoiseSVN-64bit.msi", "dev/TortoiseSVN-64bit.msi" },
	{ RUN_EXE, "vc2005x64", "dev/vcredist_2005_x64", "" },
	{ RUN_EXE, "vc2005x86", "dev/vcredist/vcredist_2005_x86.exe", "dev/vcredist/vcredist_2005_x86.exe" },
	{ RUN_EXE, "vc2008x64", "dev/vcredist/vcredist_2008_x64.exe", "dev/vcredist/vcredist_2008_x64.exe" },
	{ RUN_EXE, "vc2008x86", "dev/vcredist/vcredist_2008_x86.exe", "dev/vcredist/vcredist_2008_x86.exe" },
	{ RUN_EXE, "vc2010x64", "dev/vcredist/vcredist_2010_x64.exe", "dev/vcredist/vcredist_2010_x64.exe" },
	{ RUN_EXE, "vc2010x86", "dev/vcredist/vcredist_2010_x86.exe", "dev/vcredist/vcredist_2010_x86.exe" },
	{ RUN_EXE, "vc2012x64", "dev/vcredist/vcredist_2012_x64.exe", "dev/vcredist/vcredist_2012_x64.exe" },
	{ RUN_EXE, "vc2012x86", "dev/vcredist/vcredist_2012_x86.exe", "dev/vcredist/vcredist_2012_x86.exe" },
	{ RUN_EXE, "vc2013x64", "dev/vcredist/vcredist_2013_x64.exe", "dev/vcredist/vcredist_2013_x64.exe" },
	{ RUN_EXE, "vc2013x86", "dev/vcredist/vcredist_2013_x86.exe", "dev/vcredist/vcredist_2013_x86.exe" },
	{ RUN_EXE, "vc2015x64", "dev/vcredist/vcredist_2015_x64.exe", "dev/vcredist/vcredist_2015_x64.exe" },
	{ RUN_EXE, "vc2015x86", "dev/vcredist/vcredist_2015_x86.exe", "dev/vcredist/vcredist_2015_x86.exe" },
	{ RUN_EXE, "vc1519x64", "dev/vcredist/vcredist_2015~2019_x64.exe", "dev/vcredist/vcredist_2015~2019_x64.exe" },
	{ RUN_EXE, "vc1519x86", "dev/vcredist/vcredist_2015~2019_x86.exe", "dev/vcredist/vcredist_2015~2019_x86.exe" },
};

Reverse::Reverse(QWidget *parent, int tabid) :
	CommonMainTabObject::CommonMainTabObject((OpenArk*)parent)
{
	ui.setupUi(this);
	ui.progressBar->setValue(0);
	ui.progressBar->show();

	InitWinReverseToolsView();
	InitWinDriverKitsView();

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
	static auto is64 = UNONE::OsIs64();
	auto sender = QObject::sender();
	name = sender->objectName().replace("Btn", "");
	if (sender == ui.pchunterBtn) name = is64 ? "pchunter64" : "pchunter32";
	if (sender == ui.pchunternewBtn) name = is64 ? "pchunternew64" : "pchunternew32";
	if (sender == ui.wkeBtn) name = is64 ? "wke64" : "wke32";
	if (sender == ui.gitBtn) name = is64 ? "git64" : "git32";
	if (sender == ui.torgitBtn) name = is64 ? "torgit64" : "torgit32";
	if (sender == ui.torsvnBtn) name = is64 ? "torsvn64" : "torsvn32";

	WINTOOL_ITEM wintool;
	for (int i = 0; i < _countof(WinAllTools); i++)	{
		if (WinAllTools[i].name == name) {
			wintool = WinAllTools[i];
			break;
		}
	}
	DownloadAndExecuteFile(wintool);
}


void Reverse::DownloadAndExecuteFile(WINTOOL_ITEM wintool)
{
	int type;
	QString uri, exec;

	uri = wintool.uri;
	exec = wintool.exec;
	type = wintool.type;

	if (type == RUN_OPEN_URL) {
		ShellOpenUrl(exec);
		return;
	}

	auto &&filebase = WStrToQ(AppConfigDir() + L"/files/");
	if (!UNONE::FsIsExistedW(filebase.toStdWString())) {
		UNONE::FsCreateDirW(filebase.toStdWString());
	}
	auto &&path = filebase + uri;
	auto &&url = AppFsUrl() + "/" + uri;
	exec = filebase + exec;

	auto Run = [&](int type, QString exe)->bool {
		if (UNONE::FsIsExistedW(QToWStr(exe))) {
			if (type == RUN_EXE)
				ShellRun(exe, "");
			else if (type == RUN_CMD_DIR)
				ShellRunCmdDir(exe);
			else if (type == RUN_DIR)
				ExploreFile(exe);
			else if (type == RUN_EXE_BY_CMD)
				ShellRunCmdExe(exe, SW_HIDE);
			return true;
		}
		return false;
	};

	static bool pending = false;
	
	if (Run(type, exec))
		return;

	if (pending) {
		QMessageBox::critical(NULL, tr("Error"), tr("Download pending, wait for a while..."));
		return;
	}
	pending = true;

	UNONE::FsCreateDirW(UNONE::FsPathToDirW(QToWStr(path)));
	file = new QFile(path);
	file->open(QIODevice::WriteOnly);

	QNetworkAccessManager *accessmgr = new QNetworkAccessManager(this);
	accessmgr->setNetworkAccessible(QNetworkAccessManager::Accessible);
	QUrl qurl(url);

	QNetworkRequest request(qurl);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
	reply = accessmgr->get(request);

	connect((QObject *)reply, SIGNAL(readyRead()), this, SLOT(readContent()));
	connect(accessmgr, &QNetworkAccessManager::finished, [&, Run, type, path, exec](QNetworkReply*) {
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


		auto filepath = path.toStdString();
		if (UNONE::FsPathToExtensionA(filepath) == ".zip") {
			//Unpack
			auto dir = UNONE::FsPathToDirA(filepath);
			ZipUtils::UnpackToDir(filepath, ZipUtils::UNPACK_CURRENT, dir);
			//Clean
			DeleteFileA(filepath.c_str());
		}

		//Run
		Run(type, exec);

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
	QList<QPushButton*> buttons = ui.commonBox->findChildren<QPushButton*>();
	for (auto &btn : buttons) {
		connect(btn, SIGNAL(clicked()), this, SLOT(onExecute()));
	}

	connect(ui.toolsfolderBtn, &QPushButton::clicked, [] {
		auto folder = AppConfigDir() + L"\\files";
		ShellRun(WStrToQ(folder), "");
	});
}

void Reverse::InitWinDriverKitsView()
{
	QList<QPushButton*> buttons = ui.devkitsBox->findChildren<QPushButton*>();
	for (auto &btn : buttons) {
		connect(btn, SIGNAL(clicked()), this, SLOT(onExecute()));
	}
}
