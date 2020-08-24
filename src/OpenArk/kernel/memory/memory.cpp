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
#include "memory.h"
#include <common/common.h>
#include <common/utils/disassembly/disassembly.h>
#include <common/qt-wrapper/qt-wrapper.h>
#include <openark/openark.h>
#include <kernel/driver/driver.h>
#include <arkdrv-api/arkdrv-api.h>
#include <QtUiTools/QtUiTools>
#define DEFINE_WIDGET(type, value) auto value = memui_->findChild<type>(#value)

KernelMemory::KernelMemory()
{

}

KernelMemory::~KernelMemory()
{

}

void KernelMemory::onTabChanged(int index)
{
	CommonTabObject::onTabChanged(index);
}

bool KernelMemory::EventFilter()
{
	return true;
}

void KernelMemory::ModuleInit(Ui::Kernel *mainui, Kernel *kernel)
{
	this->ui = mainui;

	Init(ui->tabMemory, TAB_KERNEL, TAB_KERNEL_MEMORY);

	auto memrw_ = new KernelMemoryRW();
	auto memwidget = memrw_->GetWidget();
	memwidget->setParent(ui->tabMemoryView);
	memwidget->layout()->removeWidget(memwidget);
	ui->verticalLayout_7->addWidget(memwidget);
	memwidget->show();
}

KernelMemoryRW::KernelMemoryRW()
{
	QUiLoader loader;
	QFile file(":/UI/ui/memory-rw.ui");
	file.open(QFile::ReadOnly);
	memui_ = loader.load(&file);
	memui_->setAttribute(Qt::WA_DeleteOnClose);
	file.close();
	setAttribute(Qt::WA_DeleteOnClose);

	connect(memui_, &QWidget::destroyed, this, &QWidget::close);

	DEFINE_WIDGET(QPushButton*, readMemBtn);
	connect(readMemBtn, &QPushButton::clicked, this, [&] {
		DEFINE_WIDGET(QLineEdit*, pidEdit);
		DEFINE_WIDGET(QLineEdit*, readAddrEdit);
		DEFINE_WIDGET(QLineEdit*, readSizeEdit);
		ULONG64 addr = VariantInt64(readAddrEdit->text().toStdString());
		ULONG size = VariantInt(readSizeEdit->text().toStdString());
		ULONG pid = VariantInt(pidEdit->text().toStdString(), 10);
		ViewMemory(pid, addr, size);
	});

	DEFINE_WIDGET(QPushButton*, dumpToFileBtn);
	connect(dumpToFileBtn, &QPushButton::clicked, this, [&] {
		DEFINE_WIDGET(QLineEdit*, pidEdit);
		DEFINE_WIDGET(QLineEdit*, readAddrEdit);
		DEFINE_WIDGET(QLineEdit*, readSizeEdit);
		DEFINE_WIDGET(QLabel*, statusLabel);
		ULONG64 addr = VariantInt64(readAddrEdit->text().toStdString());
		ULONG size = VariantInt(readSizeEdit->text().toStdString());
		ULONG pid = VariantInt(pidEdit->text().toStdString(), 10);

		std::string buf;
		if (!ArkDrvApi::Memory::MemoryRead(pid, addr, size, buf)) {
			LabelError(statusLabel, tr("Read Memory error, addr:%1 size:%2").arg(QString::number(addr, 16).toUpper()).arg(size));
			return;
		}
		
		QString filename = WStrToQ(UNONE::StrFormatW(L"%s_%X_%X", QToWChars(CacheGetProcInfo(pid).name), addr, size));
		QString dumpmem = QFileDialog::getSaveFileName(this, tr("Save to"), filename, tr("DumpMemory(*)"));
		if (!dumpmem.isEmpty()) {
			UNONE::FsWriteFileDataW(dumpmem.toStdWString(), buf) ?
				LabelSuccess(statusLabel, tr("Dump memory to file ok")):
				LabelError(statusLabel, tr("Dump memory to file error"));
		}
	});

	DEFINE_WIDGET(QPushButton*, writeMemBtn);
	connect(writeMemBtn, &QPushButton::clicked, this, [&] {
		DEFINE_WIDGET(QLineEdit*, writeDataEdit);
		auto data = writeDataEdit->text().toStdString();
		UNONE::StrReplaceA(data, " ");
		data = UNONE::StrHexStrToStreamA(data);
		WriteMemory(data);
	});

	DEFINE_WIDGET(QPushButton*, writeStringBtn);
	connect(writeStringBtn, &QPushButton::clicked, this, [&] {
		DEFINE_WIDGET(QLineEdit*, writeDataEdit);
		auto data = writeDataEdit->text().toStdString();
		WriteMemory(data);
	});

	DEFINE_WIDGET(QLineEdit*, pidEdit);
	connect(pidEdit, &QLineEdit::textChanged, [&](const QString&) {
		DEFINE_WIDGET(QLineEdit*, pidEdit);
		DEFINE_WIDGET(QLabel*, pnameLabel);
		ULONG pid = VariantInt(pidEdit->text().toStdString(), 10);
		pnameLabel->setText(CacheGetProcInfo(pid).name);
	});
}

KernelMemoryRW::~KernelMemoryRW()
{
	free_callback_(free_vars_);
}

void KernelMemoryRW::ViewMemory(ULONG pid, ULONG64 addr, ULONG size)
{
	bool readok = false;
	char *mem = nullptr;
	ULONG memsize = 0;
	std::string buf;
	DEFINE_WIDGET(QLineEdit*, pidEdit);
	pidEdit->setText(QString::number(pid));

	if (ArkDrvApi::Memory::MemoryRead(pid, addr, size, buf)) {
		mem = (char*)buf.c_str();
		memsize = buf.size();
		readok = true;
	}

	auto hexdump = HexDumpMemory(addr, mem, size);
	auto disasm = DisasmMemory(addr, mem, size);

	DEFINE_WIDGET(QTextEdit*, hexEdit);
	DEFINE_WIDGET(QTextEdit*, disasmEdit);
	DEFINE_WIDGET(QLabel*, regionLabel);
	DEFINE_WIDGET(QLabel*, statusLabel);

	hexEdit->setText(StrToQ(hexdump));
	disasmEdit->setText(StrToQ(disasm));
	std::vector<DRIVER_ITEM> infos;
	ArkDrvApi::Driver::DriverEnumInfo(infos);
	QString path;
	for (auto info : infos) {
		if (IN_RANGE(addr, info.base, info.size)) {
			path = WStrToQ(ParseDriverPath(info.path));
			break;
		}
	}
	regionLabel->setText(path);
	readok ? LabelSuccess(statusLabel, tr("Read Memory successfully, addr:%1 size:%2").arg(QString::number(addr, 16).toUpper()).arg(size)) :
		LabelError(statusLabel, tr("Read Memory error, addr:%1 size:%2").arg(QString::number(addr, 16).toUpper()).arg(size));
}

void KernelMemoryRW::ViewMemory(ULONG pid, std::string data)
{
	return ViewMemory(pid, (ULONG64)data.c_str(), data.size());
}

void KernelMemoryRW::WriteMemory(std::string data)
{
	DEFINE_WIDGET(QLineEdit*, pidEdit);
	DEFINE_WIDGET(QLineEdit*, writeAddrEdit);
	DEFINE_WIDGET(QLabel*, statusLabel);

	ULONG64 addr = VariantInt64(writeAddrEdit->text().toStdString());

	if (ArkDrvApi::Memory::IsKernelAddress(addr)) {
		if (QMessageBox::warning(this, tr("Warning"), tr("Write kernel memory maybe cause BSOD, are you sure to write?"),
			QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
			return;
		}
	}
	ULONG pid = VariantInt(pidEdit->text().toStdString(), 10);
	ArkDrvApi::Memory::MemoryWrite(pid, addr, data) ?
		LabelSuccess(statusLabel, tr("Write Memory successfully, addr:0x%1").arg(QString::number(addr, 16).toUpper())) :
		LabelError(statusLabel, tr("Write Memory error, addr:0x%1").arg(QString::number(addr, 16).toUpper()));
}
