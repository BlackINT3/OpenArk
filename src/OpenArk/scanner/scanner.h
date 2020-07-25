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
#include <mutex>
#include "ui_scanner.h"
#include "../common/ui-wrapper/ui-wrapper.h"

namespace Ui {
	class Scanner;
	class OpenArkWindow;
}


class OpenArk;

class Scanner : public CommonMainTabObject {
	Q_OBJECT
public:
	Scanner(QWidget *parent, int tabid);
	~Scanner();

protected:
	bool eventFilter(QObject *obj, QEvent *e);

public slots:
	void onTabChanged(int index);
	void onOpenFile(const QString& file);
	void onRefresh();

private slots:
	void dragEnterEvent(QDragEnterEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);
	void dropEvent(QDropEvent *event);
	void onImportChanged(const QModelIndex &current, const QModelIndex &previous);
	void onRelocChanged(const QModelIndex &current, const QModelIndex &previous);
	void onExpandAll();
	void onTextChanged(const QString& text);

private:
	std::wstring GuessFileType();
	bool CheckIsPe();
	void RefreshSummary(const std::wstring& path);
	void RefreshHeaders();
	void RefreshSections();
	void RefreshImport();
	void RefreshExport();
	void RefreshResource();
	void RefreshRelocation();
	void RefreshDebug();
	void RefreshRva();
	bool MapPe(const std::wstring& path);
	bool UnmapPe();

private:
	Ui::Scanner ui;
	QString pe_file_;
	CHAR* pe_image_;
	bool pe_x64_;
	bool pe_valid_;
	OpenArk* parent_;
	QStandardItemModel* sumup_model_;
	QStandardItemModel* sumdown_model_;
	QStandardItemModel* headers_model_;
	QStandardItemModel* sections_model_;
	QStandardItemModel* imp_model_;
	QStandardItemModel* imp_func_model_;
	QStandardItemModel* exp_model_;
	QStandardItemModel* exp_func_model_;
	QStandardItemModel* reloc_model_;
	QStandardItemModel* reloc_item_model_;
	QStandardItemModel* dbg_model_;
	QMenu* headers_menu_;
	int64_t base_prev_;
	std::mutex upt_mutex_;
};