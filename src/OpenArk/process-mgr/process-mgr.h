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
#include <QtGui>
#include <QtCore>
#include <QtWidgets>
#include <QMutex>
#include <Windows.h>
#include "ui_process-mgr.h"
#include "../common/cache/cache.h"

namespace Ui {
	class ProcessMgr;
	class OpenArkWindow;
}

class OpenArk;

class ProcSortFilterProxyModel : public QSortFilterProxyModel {
	Q_OBJECT
public:
	ProcSortFilterProxyModel(QWidget *parent) {};
	~ProcSortFilterProxyModel() {};
protected:
	bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
};
class ModSortFilterProxyModel : public QSortFilterProxyModel {
	Q_OBJECT
public:
	ModSortFilterProxyModel(QWidget *parent) {};
	~ModSortFilterProxyModel() {};
protected:
	bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
};

class ProcessMgr : public QWidget {
	Q_OBJECT
public:
	ProcessMgr(QWidget* parent);
	~ProcessMgr();

signals:
	void signalOpen(QString);

protected:
	bool eventFilter(QObject *obj, QEvent *e);

public slots :
	void onOpenFile(const QString& file);
	void onRefresh();
	void onReset();
	void onProcSelection(QString pid);

private slots:
	void onTimer();
	void onCopyActionTriggerd(QAction* action);
	void onKillProcess();
	void onKillProcessTree();
	void onSuspendProcess();
	void onInjectDll();
	void onSelectPid();
	void onExploreFile();
	void onCreateMiniDump();
	void onCreateFullDump();
	void onEnumThread();
	void onEnumWindow();
	void onSendtoScanner();
	void onShowProperties();
	void onShowModules();
	void onProcSectionClicked(int idx);
	void onProcDoubleClicked(const QModelIndex &idx);
	void onProcChanged(const QModelIndex &current, const QModelIndex &previous);

private:
	void ShowProperties(DWORD pid, int tab);
	void ShowProcess();
	void ShowProcessList();
	void ShowProcessTree();
	void AppendProcessItem(QStandardItem *parent, QStandardItem *name_item, ProcInfo info, int seq);
	void AjustProcessStyle();

	int ProcCurRow();
	int ProcCurCol();
	DWORD ProcCurPid();
	QString ProcCurViewItemData(int column);
	QString ProcViewItemData(int row, int column);
	QString ModCurViewItemData(int column);
	QString ModViewItemData(int row, int column);

private:
	int proc_header_idx_;

private:
	Ui::ProcessMgr ui;
	OpenArk *parent_;
	QLabel *cpu_percent_lable_;
	QLabel *mm_percent_lable_;
	QLabel *cntproc_lable_;
	QLabel *cntthread_lable_;
	QLabel *cnthandle_lable_;
	QMenu *proc_menu_;
	QMenu *mod_menu_;
	QTimer timer_;
	QStandardItemModel *proc_model_;
	QStandardItemModel *mod_model_;
	ProcSortFilterProxyModel *proxy_proc_;
	ModSortFilterProxyModel *proxy_mod_;
};
