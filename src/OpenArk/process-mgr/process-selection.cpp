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
#include "../common/common.h"
#include "process-selection.h"

ProcessSelection::ProcessSelection(QWidget *parent) :
	parent_((ProcessMgr*)parent)
{
	ui.setupUi(this);
	connect(OpenArkLanguage::Instance(), &OpenArkLanguage::languageChaned, this, [this]() {ui.retranslateUi(this); });

	setAttribute(Qt::WA_ShowModal, true);
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowFlags(windowFlags()& ~Qt::WindowMaximizeButtonHint);
	setWindowTitle(tr("Select..."));

	ui.decRadio->setChecked(true);
	ui.pidEdit->setFocus();
	connect(ui.okBtn, SIGNAL(accepted()), this, SLOT(onLocateProcess()));
	connect(this, SIGNAL(procSelection(QString)), parent_, SLOT(onProcSelection(QString)));
	connect(ui.pidEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onPidChanged(const QString&)));
	connect(ui.decRadio, SIGNAL(toggled(bool)), this, SLOT(onToggled(bool)));
	connect(ui.hexRadio, SIGNAL(toggled(bool)), this, SLOT(onToggled(bool)));
}

ProcessSelection::~ProcessSelection()
{
}

void ProcessSelection::onLocateProcess()
{
	QString dec = WStrToQ(UNONE::StrFormatW(L"%d", GetInputPid()));
	emit procSelection(dec);
}

void ProcessSelection::onPidChanged(const QString &text)
{
	DWORD pid = GetInputPid();
	ui.nameEdit->setText(WStrToQ(UNONE::PsGetProcessNameW(pid)));
	ui.pathEdit->setText(WStrToQ(UNONE::PsGetProcessPathW(pid)));
}

void ProcessSelection::onToggled(bool checked)
{
	auto sender = qobject_cast<QRadioButton*>(QObject::sender());
	if (!sender->isChecked()) return;
	DWORD pid = 0;
	if (sender == ui.decRadio) {
		pid = UNONE::StrToHexA(ui.pidEdit->text().toStdString());
		ui.pidEdit->setText(WStrToQ(UNONE::StrFormatW(L"%d", pid)));
		return;
	}
	if (sender == ui.hexRadio) {
		pid = UNONE::StrToIntegerA(ui.pidEdit->text().toStdString());
		ui.pidEdit->setText(WStrToQ(UNONE::StrFormatW(L"%X", pid)));
		return;
	}
}

DWORD ProcessSelection::GetInputPid()
{
	DWORD pid;
	bool hexed = true;
	if (ui.decRadio->isChecked()) hexed = false;
	if (hexed) {
		pid = UNONE::StrToHexA(ui.pidEdit->text().toStdString());
	} else {
		pid = UNONE::StrToIntegerA(ui.pidEdit->text().toStdString());
	}
	return pid;
}