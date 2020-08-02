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
#include <QMessageBox>

void Settings::InitConsoleView()
{
	console_model_ = new QStandardItemModel;
	console_model_->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Value"));
	SetDefaultTableViewStyle(ui.consoleView, console_model_);

	InitTableItem(console_model_);

	QString name;
	name = "History.MaxRecords"; AppendTableRowNameVaule(console_model_, name, OpenArkConfig::Instance()->GetConsole(name));
	name = "History.FilePath"; AppendTableRowNameVaule(console_model_, name, OpenArkConfig::Instance()->GetConsole(name));
}

void Settings::InitCleanView()
{
	QString clean_file_suffix = ui.edit_file_suffix->text();
	ui.edit_file_suffix->setText(OpenArkConfig::Instance()->GetValue("clean_file_suffix").toString());
	QStringList path_list = OpenArkConfig::Instance()->GetValue("clean_path_list").toStringList();
	for (int i = 0; i < path_list.size(); i++)
		ui.listWidget_path->addItem(path_list[i]);

	connect(ui.add_path_btn, &QPushButton::clicked, [this]() {
		QString folder = QFileDialog::getExistingDirectory(this, tr("Open Folder"), "");
		if (folder.isEmpty()) return;
		QString path = folder.replace("/", "\\");
		if (!path.isEmpty())
			ui.listWidget_path->addItem(path);
	});

	connect(ui.del_path_btn, &QPushButton::clicked, [this]() {
		QListWidgetItem *item = ui.listWidget_path->takeItem(ui.listWidget_path->currentRow());
		delete item;
	});
	connect(ui.save_btn, &QPushButton::clicked, [this]() {
		QStringList path_list;
		for (int i = 0; i < ui.listWidget_path->count(); i++) {
			QListWidgetItem  * item = ui.listWidget_path->item(i);
			path_list << item->text();
		}
		OpenArkConfig::Instance()->SetValue("clean_file_suffix", ui.edit_file_suffix->text());
		OpenArkConfig::Instance()->SetValue("clean_path_list", path_list);
		QMessageBox::information(NULL, "", tr("Save Success"));
	});
}

void Settings::InitGeneralView()
{
	QString section = "/Setting.General/";
	QString ctxkey = section + "context_menu";
	auto ctx = OpenArkConfig::Instance()->GetValue(ctxkey, 1).toInt();
	connect(ui.ctxmenuBox, &QCheckBox::toggled, [=](bool checked) {
		const char *subkey = R"(*\shell\OpenArk)";
		if (checked) {
			UNONE::RegistryKey reg;
			reg.Create(HKEY_CLASSES_ROOT, subkey, KEY_SET_VALUE);
			reg.WriteValue("CommandFlags", 0x20);
			reg.WriteValue("ExtendedSubCommandsKey", "OpenArk");
			reg.WriteValue("Icon", QToChars(AppFilePath()));
			reg.WriteValue("MUIVerb", "OpenArk");
			reg.Close();

			reg.Create(HKEY_CLASSES_ROOT, R"(OpenArk\shell\scan)", KEY_SET_VALUE);
			reg.WriteValue("CommandFlags", 0x20);
			reg.WriteValue("", tr("Scan").toLocal8Bit().toStdString());
			reg.CreateKey("command", KEY_SET_VALUE);
			reg.WriteValue("", QToChars(AppFilePath()));
			reg.Close();

			reg.Create(HKEY_CLASSES_ROOT, R"(OpenArk\shell\unlockfile)", KEY_SET_VALUE);
			reg.WriteValue("CommandFlags", 0x20);
			reg.WriteValue("", tr("UnlockFile").toLocal8Bit().toStdString());
			reg.CreateKey("command", KEY_SET_VALUE);
			reg.WriteValue("", QToChars(AppFilePath()));
			reg.Close();

			reg.Create(HKEY_CLASSES_ROOT, R"(OpenArk\shell\settings)", KEY_SET_VALUE);
			reg.WriteValue("CommandFlags", 0x20);
			reg.WriteValue("", tr("Settings").toLocal8Bit().toStdString());
			reg.CreateKey("command", KEY_SET_VALUE);
			reg.WriteValue("", QToChars(AppFilePath()));
			reg.Close();

		} else {
			UNONE::RegistryKey::DeleteKey(HKEY_CLASSES_ROOT, subkey);
			UNONE::RegistryKey::DeleteKey(HKEY_CLASSES_ROOT, "OpenArk");
		}
		OpenArkConfig::Instance()->SetValue(ctxkey, (int)checked);
	});
	ui.ctxmenuBox->setChecked(ctx);
}

Settings::Settings(QWidget *parent)
{
	ui.setupUi(this);
	connect(OpenArkLanguage::Instance(), &OpenArkLanguage::languageChaned, this, [this]() {ui.retranslateUi(this); });

	setAttribute(Qt::WA_ShowModal, true);
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowFlags(windowFlags()& ~(Qt::WindowMaximizeButtonHint| Qt::WindowMinimizeButtonHint)| Qt::MSWindowsFixedSizeDialogHint);

	InitConsoleView();
	InitCleanView();
	InitGeneralView();

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
		OpenArkConfig::Instance()->SetValue(key, value);
	}
	OpenArkConfig::Instance()->Sync();
}

