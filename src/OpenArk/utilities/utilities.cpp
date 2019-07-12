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
#include "utilities.h"
#include "../common/common.h"
#include "../openark/openark.h"

Utilities::Utilities(QWidget *parent) :
	parent_((OpenArk*)parent)
{
	ui.setupUi(this);
	ui.tabWidget->setTabPosition(QTabWidget::West);
	ui.tabWidget->tabBar()->setStyle(new OpenArkTabStyle);

	InitSystemToolsView();
}

Utilities::~Utilities()
{
}

void Utilities::InitSystemToolsView()
{
	connect(ui.cmdBtn, &QPushButton::clicked, [] {ShellRun("cmd.exe", "/k cd /d %userprofile%"); });
	connect(ui.wslBtn, &QPushButton::clicked, [] {ShellRun("wsl.exe", ""); });
	connect(ui.powershellBtn, &QPushButton::clicked, [] {ShellRun("powershell.exe", ""); });
	connect(ui.calcBtn, &QPushButton::clicked, [] {ShellRun("calc.exe", ""); });
	connect(ui.regeditBtn, &QPushButton::clicked, [] {ShellRun("regedit.exe", ""); });
	connect(ui.servicesBtn, &QPushButton::clicked, [] {ShellRun("services.msc", ""); });
	connect(ui.taskmgrBtn, &QPushButton::clicked, [] {ShellRun("taskmgr.exe", ""); });
	connect(ui.programsBtn, &QPushButton::clicked, [] {ShellRun("control.exe", "appwiz.cpl"); });
	connect(ui.envBtn, &QPushButton::clicked, [] {ShellRun("SystemPropertiesAdvanced.exe", ""); });
	connect(ui.pcnameBtn, &QPushButton::clicked, [] {ShellRun("SystemPropertiesComputerName.exe", ""); });

	connect(ui.sysinfoBtn, &QPushButton::clicked, [] {ShellRun("cmd.exe", "/c systeminfo |more & pause"); });
	connect(ui.datetimeBtn, &QPushButton::clicked, [] {ShellRun("control.exe", "date/time"); });
	connect(ui.tasksBtn, &QPushButton::clicked, [] {ShellRun("taskschd.msc", "/s"); });
	connect(ui.versionBtn, &QPushButton::clicked, [] {ShellRun("winver.exe", ""); });
	connect(ui.deskiconsBtn, &QPushButton::clicked, [] {ShellRun("rundll32.exe", "shell32.dll,Control_RunDLL desk.cpl,,0"); });
	connect(ui.wallpaperBtn, &QPushButton::clicked, [] {
		if (UNONE::OsMajorVer() <= 5) ShellRun("rundll32.exe", "shell32.dll,Control_RunDLL desk.cpl,,0");
		else ShellRun("control.exe", "/name Microsoft.Personalization /page pageWallpaper"); 
	});
	connect(ui.devmgrBtn, &QPushButton::clicked, [] {ShellRun("devmgmt.msc", ""); });
	connect(ui.diskmgrBtn, &QPushButton::clicked, [] {ShellRun("diskmgmt.msc", ""); });

	connect(ui.resmonBtn, &QPushButton::clicked, [] {ShellRun("resmon.exe", ""); });
	connect(ui.perfBtn, &QPushButton::clicked, [] {ShellRun("perfmon.exe", "SystemPropertiesComputerName.exe"); });
	connect(ui.perfsetBtn, &QPushButton::clicked, [] {ShellRun("SystemPropertiesPerformance.exe", ""); });
	connect(ui.powerBtn, &QPushButton::clicked, [] {ShellRun("control.exe", "powercfg.cpl,,3"); });
	connect(ui.usersBtn, &QPushButton::clicked, [] {ShellRun("lusrmgr.msc", ""); });
	connect(ui.uacBtn, &QPushButton::clicked, [] {ShellRun("UserAccountControlSettings.exe", ""); });
	connect(ui.evtBtn, &QPushButton::clicked, [] {ShellRun("eventvwr.msc", ""); });
	connect(ui.gpoBtn, &QPushButton::clicked, [] {ShellRun("gpedit.msc", ""); });
	connect(ui.secpolBtn, &QPushButton::clicked, [] {ShellRun("secpol.msc", ""); });
	connect(ui.certBtn, &QPushButton::clicked, [] {ShellRun("certmgr.msc", ""); });
	connect(ui.credBtn, &QPushButton::clicked, [] {ShellRun("control.exe", "/name Microsoft.CredentialManager"); });


	connect(ui.firewallBtn, &QPushButton::clicked, [] {ShellRun("control.exe", "firewall.cpl"); });
	connect(ui.proxyBtn, &QPushButton::clicked, [] {ShellRun("rundll32.exe", "shell32.dll,Control_RunDLL inetcpl.cpl,,4"); });
	connect(ui.netconnBtn, &QPushButton::clicked, [] {ShellRun("control.exe", "ncpa.cpl"); });
	connect(ui.hostsBtn, &QPushButton::clicked, [&] {ShellRun("notepad.exe", WStrToQ(UNONE::OsSystem32DirW() + L"\\drivers\\etc\\hosts")); });
	connect(ui.ipv4Btn, &QPushButton::clicked, [] {ShellRun("cmd.exe", "/c ipconfig|findstr /i ipv4 & pause"); });
	connect(ui.ipv6Btn, &QPushButton::clicked, [] {ShellRun("cmd.exe", "/c ipconfig|findstr /i ipv6 & pause"); });
	connect(ui.routeBtn, &QPushButton::clicked, [] {ShellRun("cmd.exe", "/c route print & pause"); });
	connect(ui.sharedBtn, &QPushButton::clicked, [] {ShellRun("fsmgmt.msc", ""); });
}