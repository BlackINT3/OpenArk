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
#include "openark.h"
#include "common/common.h"
#include "process-mgr/process-mgr.h"
#include "scanner/scanner.h"
#include "coderkit/coderkit.h"
#include "bundler/bundler.h"
#include "settings/settings.h"
#include "about/about.h"
#include "cmds/cmds.h"
#include "kernel/kernel.h"
#include "reverse/reverse.h"
#include "utilities/utilities.h"

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

#define APP_CHKUPT_SERVER "http://upt.blackint3.com/openark/version.txt"
#define APP_MESSAGE_PATTERN \
"%{if-debug}<font color=#E0E2E4>%{endif}"\
"%{if-info}<font color=#E0E2E4>%{endif}"\
"%{if-warning}<font color=red>%{endif}"\
"%{if-critical}<font color=red>%{endif}"\
"[%{function}:%{line}]"\
"%{if-debug}[DBG]%{endif}%{if-info}[INFO]%{endif}%{if-warning}[WARN]%{endif}%{if-critical}[ERR]%{endif}"\
"%{message}</font>"
void QtMessageHandlerCallback(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	QString fmt = msg;
	if (!msg.isEmpty()) {
		fmt = qFormatLogMessage(type, context, msg);
	}
	openark->onLogOutput(fmt);
}

OpenArk::OpenArk(QWidget *parent) : 
	QMainWindow(parent)
{
	openark = this;
	qSetMessagePattern(APP_MESSAGE_PATTERN);
	qInstallMessageHandler(QtMessageHandlerCallback);
	UNONE::InterRegisterLogger([&](const std::wstring &log) {
		onLogOutput(WStrToQ(log));
	});
	ui.setupUi(this);
	
	int x, y, w, h;
	OpenArkConfig::Instance()->GetMainGeometry(x, y, w, h);
	move(x, y);
	resize(w, h);
	ui.splitter->setStretchFactor(0, 1);
	ui.splitter->setStretchFactor(1, 5);
	QString title = QString(tr("OpenArk v%1 ").arg(AppVersion()));
	title.append(tr(" [build:%1]  ").arg(AppBuildTime()));

	UNONE::PeX64((CHAR*)GetModuleHandleW(NULL)) ? title.append(tr("64-Bit")) : title.append(tr("32-Bit"));

	setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);
	setWindowTitle(title);

	QWidget *widget = new QWidget();
	QLabel *link = new QLabel(widget);
	link->setText("<a style='color:blue;a{text-decoration: none}' href=\"https://github.com/BlackINT3/OpenArk\">"+ tr("Project on Github")+"</a>&nbsp;");
	link->setOpenExternalLinks(true);

	stool_ = new QToolBar(widget);
	stool_->setObjectName(QStringLiteral("statusToolBar"));
	stool_->setIconSize(QSize(16, 16));
	stool_->addAction(ui.actionConsole);
	stool_->addSeparator();
	stool_->addAction(ui.actionModule);
	stool_->addAction(ui.actionHandle);
	stool_->addAction(ui.actionMemory);
	ui.actionModule->setChecked(false);
	ui.actionHandle->setChecked(false);
	ui.actionMemory->setChecked(false);
	connect(ui.actionModule, SIGNAL(triggered(bool)), this, SLOT(onActionPtool(bool)));
	connect(ui.actionHandle, SIGNAL(triggered(bool)), this, SLOT(onActionPtool(bool)));
	connect(ui.actionMemory, SIGNAL(triggered(bool)), this, SLOT(onActionPtool(bool)));
	//stool_->setStyleSheet("background-color:red");

	QGridLayout *layout = new QGridLayout(widget);
	layout->setMargin(0);
	layout->addWidget(stool_, 0, 0, 1, 1, Qt::AlignVCenter | Qt::AlignLeft);
	layout->addWidget(link, 0, 1, 1, 1, Qt::AlignVCenter | Qt::AlignRight);
	//ui.statusBar->setStyleSheet("QToolButton: {background-color: red; }");
	ui.statusBar->setSizeGripEnabled(false);
	ui.statusBar->setFixedHeight(28);
	ui.statusBar->addWidget(widget, 1);
	connect(ui.actionConsole, SIGNAL(triggered(bool)), this, SLOT(onActionConsole(bool)));

	ui.consoleWidget->hide();
	ui.actionOnTop->setCheckable(true);
	ui.actionExit->setShortcut(QKeySequence(Qt::ALT + Qt::Key_F4));

	// Language 
	QActionGroup *langs = new QActionGroup(this);
	langs->setExclusive(true);
	langs->addAction(ui.actionEnglish);
	langs->addAction(ui.actionZhcn);
	int lang = OpenArkLanguage::Instance()->GetLanguage();
	if (lang == 0) {
		ui.actionEnglish->setChecked(true);
	} else if (lang == 1)  {
		ui.actionZhcn->setChecked(true);
	}
	connect(langs, SIGNAL(triggered(QAction*)), this, SLOT(onActionLanguage(QAction*)));

	connect(ui.actionRun, &QAction::triggered, this, [=]() { UNONE::PsCreateProcessW(L"rundll32.exe shell32.dll,#61"); });
	connect(ui.actionExit, &QAction::triggered, this, [=]() { QApplication::quit(); });
	connect(ui.actionAbout, SIGNAL(triggered(bool)), this, SLOT(onActionAbout(bool)));
	connect(ui.actionSettings, SIGNAL(triggered(bool)), this, SLOT(onActionSettings(bool)));
	connect(ui.actionOpen, SIGNAL(triggered(bool)), this, SLOT(onActionOpen(bool)));
	connect(ui.actionRefresh, SIGNAL(triggered(bool)), this, SLOT(onActionRefresh(bool)));
	connect(ui.actionReset, SIGNAL(triggered(bool)), this, SLOT(onActionReset(bool)));
	connect(ui.actionOnTop, SIGNAL(triggered(bool)), this, SLOT(onActionOnTop(bool)));
	connect(ui.actionGithub, SIGNAL(triggered(bool)), this, SLOT(onActionGithub(bool)));
	connect(ui.actionManuals, SIGNAL(triggered(bool)), this, SLOT(onActionManuals(bool)));
	connect(ui.actionCheckUpdate, SIGNAL(triggered(bool)), this, SLOT(onActionCheckUpdate(bool)));
	connect(ui.actionCoderKit, SIGNAL(triggered(bool)), this, SLOT(onActionCoderKit(bool)));
	connect(ui.actionScanner, SIGNAL(triggered(bool)), this, SLOT(onActionScanner(bool)));
	connect(ui.actionBundler, SIGNAL(triggered(bool)), this, SLOT(onActionBundler(bool)));

	cmds_ = new Cmds(ui.cmdOutWindow);
	cmds_->hide();
	cmds_->setParent(ui.cmdOutWindow);
	ui.cmdInput->installEventFilter(this);
	ui.cmdOutWindow->installEventFilter(this);
	ui.cmdOutWindow->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui.cmdInput, SIGNAL(returnPressed()), this, SLOT(onCmdInput()));
	connect(ui.cmdButton, SIGNAL(clicked()), this, SLOT(onCmdHelp()));
	connect(ui.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onTabChanged(int)));
	connect(ui.cmdOutWindow, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onShowConsoleMenu(const QPoint &)));

	int main_idx = 0;
	int level2_idx = 0;
	OpenArkConfig::Instance()->GetPrefMainTab(main_idx);
	OpenArkConfig::Instance()->GetPrefLevel2Tab(level2_idx);
	auto CreateTabPage = [&](QWidget *widget, QWidget *origin) {
		int idx = ui.tabWidget->indexOf(origin);
		QString text = ui.tabWidget->tabText(idx);
		ui.tabWidget->removeTab(idx);
		ui.tabWidget->insertTab(idx, widget, text);
	};

	auto processmgr = new ProcessMgr(this);
	CreateTabPage(processmgr, ui.tabProcessMgr);

	auto scanner = new Scanner(this);
	CreateTabPage(scanner, ui.tabScanner);
	
	auto coderkit = new CoderKit(this);
	CreateTabPage(coderkit, ui.tabCoderKit);

	auto bundler = new Bundler(this);
	CreateTabPage(bundler, ui.tabBundler);
	
	auto kernel = new Kernel(this);
	CreateTabPage(kernel, ui.tabKernel);

	auto utilities = new Utilities(this);
	CreateTabPage(utilities, ui.tabUtilities);

	auto reverse = new Reverse(this);
	CreateTabPage(reverse, ui.tabReverse);

	switch (main_idx) {
	case TAB_KERNEL: kernel->SetActiveTab(level2_idx); break;
	case TAB_CODERKIT: coderkit->SetActiveTab(level2_idx); break;
	case TAB_SCANNER: scanner->SetActiveTab(level2_idx); break;
	case TAB_UTILITIES: utilities->SetActiveTab(level2_idx); break;
	case TAB_REVERSE: reverse->SetActiveTab(level2_idx); break;
	}

	SetActiveTab(main_idx);

	chkupt_timer_ = new QTimer();
	chkupt_timer_->setInterval(1000);
	chkupt_timer_->start();
	connect(chkupt_timer_, &QTimer::timeout, this, [&]() {
		onActionCheckUpdate(false);
		chkupt_timer_->stop();
	});

	connect(OpenArkLanguage::Instance(), &OpenArkLanguage::languageChaned, this, [this]() {ui.retranslateUi(this); });

	this->installEventFilter(this);
}

bool OpenArk::eventFilter(QObject *obj, QEvent *e)
{
	bool filtered = false;
	if (obj == ui.cmdInput) {
		if (e->type() == QEvent::KeyPress) {
			filtered = true;
			QKeyEvent* keyevt = dynamic_cast<QKeyEvent*>(e);
			if (keyevt->key() == Qt::Key_Up) {
				ui.cmdInput->setText(cmds_->CmdGetLast());
			}	else if (keyevt->key() == Qt::Key_Down) {
				ui.cmdInput->setText(cmds_->CmdGetNext());
			} else if (keyevt->key() == Qt::Key_Tab) {
				ui.cmdOutWindow->setFocus();
			} else {
				filtered = false;
			}
		}
	} else if (obj == ui.cmdOutWindow) {
		if (e->type() == QEvent::KeyPress) {
			filtered = true;
			QKeyEvent* keyevt = dynamic_cast<QKeyEvent*>(e);
			if (keyevt->key() == Qt::Key_Tab) {
				ui.cmdInput->setFocus();
			} else {
				filtered = false;
			}
		}
	} else if (obj == this) {
		if (e->type() == QEvent::Resize) {
			auto evt = dynamic_cast<QResizeEvent*>(e);
			OpenArkConfig::Instance()->SetMainGeometry(-1, -1, evt->size().width(), evt->size().height());
		} else if (e->type() == QEvent::Move) {
			auto evt = dynamic_cast<QMoveEvent*>(e);
			OpenArkConfig::Instance()->SetMainGeometry(evt->pos().x()-8, evt->pos().y()-31, -1, -1);
		}
	}
	

	if (filtered) {
		QKeyEvent* keyevt = dynamic_cast<QKeyEvent*>(e);
		keyevt->ignore();
		return true;
	}
	return QWidget::eventFilter(obj, e);
}

void OpenArk::onActionOpen(bool checked)
{
	QString file = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("All Files (*.*)"));
	if (file.isEmpty()) return;
	QMetaObject::invokeMethod(ui.tabWidget->currentWidget(), "onOpenFile", Qt::QueuedConnection, Q_ARG(QString, file));
}

void OpenArk::onActionRefresh(bool checked)
{
	QMetaObject::invokeMethod(ui.tabWidget->currentWidget(), "onRefresh", Qt::QueuedConnection);
}

void OpenArk::onActionReset(bool checked)
{
	QMetaObject::invokeMethod(ui.tabWidget->currentWidget(), "onReset", Qt::QueuedConnection);
}

void OpenArk::onActionOnTop(bool checked)
{
	HWND wnd = (HWND)winId();
	if (checked) {
		SetWindowOnTop(wnd, true);
	}	else {
		SetWindowOnTop(wnd, false);
	}
}

void OpenArk::onActionAbout(bool checked)
{
	auto about = new About(this);
	about->raise();
	about->show();
}

void OpenArk::onActionSettings(bool checked)
{
	auto about = new Settings(this);
	about->raise();
	about->show();
}

void OpenArk::onActionConsole(bool checked)
{
	QAction* sender = qobject_cast<QAction*>(QObject::sender());
	if (sender->isChecked()) {
		ui.consoleWidget->show();
		ui.cmdInput->setFocus();
	}	else {
		ui.consoleWidget->hide();
	}
}

void OpenArk::onActionPtool(bool checked)
{
	QAction* sender = qobject_cast<QAction*>(QObject::sender());
	if (!sender->isChecked()) {
		emit signalShowPtool(-1);
		return;
	}
	if (sender == ui.actionModule) {
		emit signalShowPtool(0);
		ui.actionHandle->setChecked(false);
		ui.actionMemory->setChecked(false);
		return;
	}

	if (sender == ui.actionHandle) {
		emit signalShowPtool(1);
		ui.actionModule->setChecked(false);
		ui.actionMemory->setChecked(false);
		return;
	}

	if (sender == ui.actionMemory) {
		emit signalShowPtool(2);
		ui.actionModule->setChecked(false);
		ui.actionHandle->setChecked(false);
		return;
	}
}

void OpenArk::onActionManuals(bool checked)
{
	ShellOpenUrl("https://openark.blackint3.com/manuals/");
}

void OpenArk::onActionGithub(bool checked)
{
	ShellOpenUrl("https://github.com/BlackINT3/OpenArk/");
}

void OpenArk::onActionCheckUpdate(bool checked)
{
	QString url = APP_CHKUPT_SERVER;
	QNetworkRequest req;
	req.setUrl(QUrl(url));
	QNetworkAccessManager *mgr = new QNetworkAccessManager(this);
	QNetworkReply *reply = mgr->get(req);
	
	INFO(L"requset server:%s", url.toStdWString().c_str());

	connect(reply, &QNetworkReply::finished, [this, reply, checked]() {
		if (reply->error() != QNetworkReply::NoError) {
			auto err = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
			ERR(L"request http.err:%d, net.err:%d", err.toInt(), (int)reply->error());
			return;
		}
		QByteArray data = reply->readAll();
		QJsonValue val;
		QJsonObject obj;
		
		INFO("server responsed:%s", data.toStdString().c_str());

		if (!JsonParse(data, obj) || !JsonGetValue(obj, "err", val)) {
			ERR(L"request app-err: json invalid");
			return;
		}
		if (val.toInt() != 0) {
			ERR(L"request app-err: %d", val.toInt());
			return;
		}
		QJsonValue appver, appbd, appurl, appfsurl;
		if (!JsonGetValue(obj, "appver", appver) || !JsonGetValue(obj, "appbd", appbd) || !JsonGetValue(obj, "appurl", appurl)) {
			ERR(L"request json err: %d", val.toInt());
			return;
		}

		if (JsonGetValue(obj, "appfsurl", appfsurl)) {
			AppFsUrl(appfsurl.toString());
		}

		auto local_ver = AppVersion();
		auto local_bd = AppBuildTime();
		INFO(L"local appver:%s, build:%s", local_ver.toStdWString().c_str(), local_bd.toStdWString().c_str());
		if (local_ver.isEmpty() || local_bd.isEmpty()) {
			return;
		}
		if (local_ver < appver.toString() || local_bd < appbd.toString()) {
			QString tips = QString(tr("Found new version, app should be updated.\nappver: %1\nappbd: %2\nappurl: %3")
				.arg(appver.toString())
				.arg(appbd.toString())
				.arg(appurl.toString()));
			QMessageBox::StandardButton reply;
			reply = QMessageBox::question(this, tr("App Updates"), tips, QMessageBox::Yes | QMessageBox::No);
			if (reply == QMessageBox::Yes) {
				ShellOpenUrl(appurl.toString());
			}
			return;
		}
		if (checked) MsgBoxInfo(tr("OpenArk is latest."));
		INFO(L"OpenArk is latest.");
		reply->deleteLater();
	});
}

void OpenArk::onActionLanguage(QAction *act)
{
	auto lang = OpenArkLanguage::Instance()->GetLanguage();
	auto text = act->text();
	if (act == ui.actionEnglish) {
		if (lang == 0) return;
		lang = 0;
	} else if (act == ui.actionZhcn) {
		if (lang == 1) return;
		lang = 1;
	}	else {
		return; 
	}
	QString tips = tr("Language changed ok, did you restart application now?");
	OpenArkConfig::Instance()->GetLang(CONF_SET, lang);
	QMessageBox::StandardButton reply;
	reply = QMessageBox::information(this, tr("Information"), tips, QMessageBox::Yes | QMessageBox::No);
	if (reply == QMessageBox::Yes) {
		onExecCmd(L".restart");
	}
}

void OpenArk::onActionCoderKit(bool checked)
{
	SetActiveTab(TAB_CODERKIT);
}

void OpenArk::onActionScanner(bool checked)
{
	SetActiveTab(TAB_SCANNER);
}

void OpenArk::onActionBundler(bool checked)
{
	SetActiveTab(TAB_BUNDLER);
}

void OpenArk::onLogOutput(QString log)
{
	log.replace("\n", "<br/>");
	log.replace("err", "<font color=red>err</font>");
	log.replace("error", "<font color=red>error</font>");
	log.replace("ERR", "<font color=red>ERR</font>");
	log.replace("ERROR", "<font color=red>ERROR</font>");
	ui.cmdOutWindow->append(log);
}

void OpenArk::onExecCmd(const std::wstring &cmdline)
{
	cmds_->CmdDispatcher(cmdline);
}

void OpenArk::onOpen(QString path)
{
	QMetaObject::invokeMethod(ui.tabWidget->currentWidget(), "onOpenFile", Qt::QueuedConnection, Q_ARG(QString, path));
}

void OpenArk::onCmdHelp()
{
	onExecCmd(L".help");
}

void OpenArk::onShowConsoleMenu(const QPoint &pt)
{
	QMenu *menu = ui.cmdOutWindow->createStandardContextMenu();
	menu->addSeparator();
	menu->addAction(tr("History"), this, SLOT(onConsoleHistory()));
	menu->addAction(tr("Helps"), this, SLOT(onConsoleHelps()));
	menu->addAction(tr("Clear"), this, SLOT(onConsoleClear()));
	menu->exec(ui.cmdOutWindow->mapToGlobal(pt));
	delete menu;
}

void OpenArk::onConsoleHistory()
{
	onExecCmd(L".history");
}

void OpenArk::onConsoleClear()
{
	onExecCmd(L".cls");
}

void OpenArk::onConsoleHelps()
{
	onExecCmd(L".help");
}

void OpenArk::onCmdInput()
{
	QLineEdit* sender = qobject_cast<QLineEdit*>(QObject::sender());
	std::wstring input = sender->text().toStdWString();
	if (input.empty()) input = cmds_->CmdGetLast().toStdWString();
	onExecCmd(input);
	auto scroll = ui.cmdOutWindow->verticalScrollBar();
	scroll->setSliderPosition(scroll->maximum());
	sender->clear();
}

void OpenArk::onTabChanged(int idx)
{
	if (idx == TAB_PROCESS) onActionRefresh(true);
	OpenArkConfig::Instance()->SetPrefMainTab(idx);

	switch (idx) {
	case TAB_KERNEL:
	case TAB_CODERKIT:
	case TAB_SCANNER:
	case TAB_UTILITIES:
	case TAB_REVERSE:
		auto obj = ui.tabWidget->currentWidget();
		if (obj->objectName().contains("tab")) break;
		qint32 l2;
		qRegisterMetaType<qint32>("qint32");
		QMetaObject::invokeMethod(obj,
			"GetActiveTab", Qt::DirectConnection, Q_RETURN_ARG(qint32, l2));
		OpenArkConfig::Instance()->SetPrefLevel2Tab(l2);
		break;
	}

}

void OpenArk::StatusBarClear()
{
}

void OpenArk::StatusBarAdd(QWidget *label)
{
	stool_->addSeparator();
	stool_->addWidget(label);
}

void OpenArk::SetActiveTab(int idx)
{
	ui.tabWidget->setCurrentIndex(idx);
}