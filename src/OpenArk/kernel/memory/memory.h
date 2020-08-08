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
#pragma once
#include <windows.h>
#include <vector>
#include "ui_kernel.h"
#include "../kernel.h"
#include "../common/common.h"

class Ui::Kernel;
class Kernel;
class KernelMemoryRW;

class KernelMemory : public CommonTabObject {
	Q_OBJECT

public:
	enum {
		Region,
		View,
	};
	KernelMemory();
	~KernelMemory();
public:
	bool EventFilter();
	void ModuleInit(Ui::Kernel *mainui, Kernel *kernel);

private slots:
	void onTabChanged(int index);

private:
	Ui::Kernel *ui;
	KernelMemoryRW *memrw_;
};

class KernelMemoryRW : public QWidget {
	Q_OBJECT

public:
	KernelMemoryRW();
	~KernelMemoryRW();

private:
	QWidget *memui_;

public:
	void ViewMemory(ULONG64 addr, ULONG size);
	QWidget *GetWidget() const { return memui_; };

};