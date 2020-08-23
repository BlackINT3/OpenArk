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
#include "../common/common.h"
#include "../common/utils/disassembly/disassembly.h"
#include "../openark/openark.h"
#include "../../../OpenArkDrv/arkdrv-api/arkdrv-api.h"
#include "../common/qt-wrapper/qt-wrapper.h"
#include "../driver/driver.h"
#include <QtUiTools/QtUiTools>

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
#define DEFINE_WIDGET(type, value) auto value = memui_->findChild<type>(#value)
	QUiLoader loader;
	QFile file(":/UI/ui/memory-rw.ui");
	file.open(QFile::ReadOnly);
	memui_ = loader.load(&file);
	file.close();
	DEFINE_WIDGET(QPushButton*, readMemBtn);
	connect(readMemBtn, &QPushButton::clicked, this, [&] {
		DEFINE_WIDGET(QLineEdit*, pidEdit);
		DEFINE_WIDGET(QLineEdit*, readAddrEdit);
		DEFINE_WIDGET(QLineEdit*, readSizeEdit);
		ULONG64 addr = VariantInt64(readAddrEdit->text().toStdString());
		ULONG size = VariantInt(readSizeEdit->text().toStdString());
		ULONG pid = VariantInt(pidEdit->text().toStdString());
		ViewMemory(pid, addr, size);
	});

	DEFINE_WIDGET(QPushButton*, writeMemBtn);
	connect(writeMemBtn, &QPushButton::clicked, this, [&] {
		DEFINE_WIDGET(QLineEdit*, writeDataEdit);
		auto data = writeDataEdit->text().toStdString();
		UNONE::StrReplaceA(data, " ");
		data = UNONE::StrStreamToHexStrA(data);
		if (ArkDrvApi::Memory::MemoryWrite(GetCurrentProcessId(), data)) {
			MsgBoxInfo(tr("Write Memory ok"));
		} else {
			MsgBoxError(tr("Write Memory error"));
		}
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
}


void KernelMemoryRW::ViewMemory(ULONG pid, ULONG64 addr, ULONG size)
{
	std::vector<DRIVER_ITEM> infos;
	ArkDrvApi::Driver::DriverEnumInfo(infos);
	QString path;
	for (auto info : infos) {
		if (IN_RANGE(addr, info.base, info.size)) {
			path = WStrToQ(ParseDriverPath(info.path));
			break;
		}
	}
	char *mem = nullptr;
	ULONG memsize = 0;
	std::string buf;
	if (ArkDrvApi::Memory::MemoryRead(GetCurrentProcessId(), addr, size, buf)) {
		mem = (char*)buf.c_str();
		memsize = buf.size();
	}
	auto hexdump = HexDumpMemory(addr, mem, size);
	auto disasm = DisasmMemory(addr, mem, size);

	auto hexEdit = memui_->findChild<QTextEdit*>("hexEdit");
	auto disasmEdit = memui_->findChild<QTextEdit*>("disasmEdit");
	auto regionLabel = memui_->findChild<QLabel*>("regionLabel");

	hexEdit->setText(StrToQ(hexdump));
	disasmEdit->setText(StrToQ(disasm));
	regionLabel->setText(path);
}
