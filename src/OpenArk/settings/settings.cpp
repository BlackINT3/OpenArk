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
#include "settings.h"
#include "../common/common.h"

Settings::Settings(QWidget *parent)
{
	ui.setupUi(this);
	connect(OpenArkLanguage::Instance(), &OpenArkLanguage::languageChaned, this, [this]() {ui.retranslateUi(this); });

	setAttribute(Qt::WA_ShowModal, true);
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowFlags(windowFlags()& ~(Qt::WindowMaximizeButtonHint| Qt::WindowMinimizeButtonHint)| Qt::MSWindowsFixedSizeDialogHint);

	console_model_ = new QStandardItemModel;
	console_model_->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Value"));
	SetDefaultTableViewStyle(ui.consoleView, console_model_);

	InitTableItem(console_model_);

	QString name;
	name = "History.MaxRecords"; AppendTableRowNameVaule(console_model_, name, ConfigGetConsole(name));
	name = "History.FilePath"; AppendTableRowNameVaule(console_model_, name, ConfigGetConsole(name));
}

Settings::~Settings()
{
}

void Settings::closeEvent(QCloseEvent *e)
{
	QString section = "/Console/";

	for (int i = 0; i < console_model_->rowCount(); i++) {
		auto name = console_model_->item(i, 0)->data(Qt::DisplayRole).toString();
		auto value = console_model_->item(i, 1)->data(Qt::DisplayRole);
		auto key = section + name;
		appconf->setValue(key, value);
	}
	appconf->sync();
}