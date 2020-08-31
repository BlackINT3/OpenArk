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
	bool eventFilter(QObject *obj, QEvent *e);
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
	bool free_init_;
	std::function<void(QList<QVariant>)> free_callback_;
	QList<QVariant> free_vars_;
	ULONG maxsize_;

public:
	bool eventFilter(QObject *obj, QEvent *e);
	void RegFreeCallback(std::function<void(QList<QVariant>)> callback, QList<QVariant> vars) {
		free_callback_ = callback;
		free_vars_ = vars;
		free_init_ = true;
	};
	void SetMaxSize(ULONG maxsize) { maxsize_ = maxsize; };
	void ViewMemory(ULONG pid, ULONG64 addr, ULONG size);
	void ViewMemory(ULONG pid, std::string data);
	void WriteMemory(std::string data);
	void OpenNewWindow(QWidget *parent, ULONG64 addr, ULONG size)
	{
		auto memwidget = this->GetWidget();
		memwidget->findChild<QLineEdit*>("readAddrEdit")->setText(QString("0x%1").arg(QString::number(addr,16).toUpper()));
		memwidget->findChild<QLineEdit*>("writeAddrEdit")->setText(QString("0x%1").arg(QString::number(addr, 16).toUpper()));
		memwidget->findChild<QLineEdit*>("readSizeEdit")->setText(QString("0x%1").arg(QString::number(size, 16).toUpper()));
		memwidget->setParent(parent);
		memwidget->setWindowTitle(tr("Memory Read-Write"));
		memwidget->setWindowFlags(Qt::Window);
		memwidget->resize(1000, 530);
		memwidget->show();
	}
	QWidget *GetWidget() const { return memui_; };

};