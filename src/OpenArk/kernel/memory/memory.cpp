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

KernelMemory::KernelMemory()
{

}

KernelMemory::~KernelMemory()
{

}

void KernelMemory::onTabChanged(int index)
{
}

bool KernelMemory::EventFilter()
{
	return true;
}

void KernelMemory::ModuleInit(Ui::Kernel *mainui, Kernel *kernel)
{
	this->ui = mainui;

	Init(ui->tabMemory, TAB_KERNEL, KernelTabMemory);

	connect(ui->dumpmemBtn, &QPushButton::clicked, this, [&] {
		ULONG64 addr = VariantInt64(ui->addrEdit->text().toStdString());
		ULONG size = VariantInt(ui->sizeEdit->text().toStdString());
		ShowDumpMemory(addr, size);
	});
}

void KernelMemory::ShowDumpMemory(ULONG64 addr, ULONG size)
{
	std::vector<DRIVER_ITEM> infos;
	ArkDrvApi::DriverEnumInfo(infos);
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
	if (ArkDrvApi::MemoryRead(addr, size, buf)) {
		mem = (char*)buf.c_str();
		memsize = buf.size();
	}
	auto hexdump = HexDumpMemory(addr, mem, size);
	auto disasm = DisasmMemory(addr, mem, size);
	ui->hexEdit->setText(StrToQ(hexdump));
	ui->disasmEdit->setText(StrToQ(disasm));
	ui->regionLabel->setText(path);
}

void KernelMemory::ShowUnlockFiles()
{
}