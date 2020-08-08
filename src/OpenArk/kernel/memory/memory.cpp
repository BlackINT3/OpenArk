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
	QUiLoader loader;
	QFile file(":/UI/ui/memory-rw.ui");
	file.open(QFile::ReadOnly);
	memui_ = loader.load(&file);
	file.close();
	auto readmemBtn = memui_->findChild<QPushButton*>("readmemBtn");
	connect(readmemBtn, &QPushButton::clicked, this, [&] {
		auto addrEdit = memui_->findChild<QLineEdit*>("readAddrEdit");
		auto sizeEdit = memui_->findChild<QLineEdit*>("readSizeEdit");
		ULONG64 addr = VariantInt64(addrEdit->text().toStdString());
		ULONG size = VariantInt(sizeEdit->text().toStdString());
		ViewMemory(addr, size);
	});
}

KernelMemoryRW::~KernelMemoryRW()
{
}


void KernelMemoryRW::ViewMemory(ULONG64 addr, ULONG size)
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
	if (ArkDrvApi::Memory::MemoryRead(addr, size, buf)) {
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
