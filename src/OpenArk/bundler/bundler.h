#pragma once

#include <QtGui>
#include <QtCore>
#include <QtWidgets>
#include <QMutex>
#include <Windows.h>
#include <mutex>
#include "ui_bundler.h"

namespace Ui {
	class Bundler;
	class OpenArkWindow;
}


class OpenArk;

class Bundler : public QWidget {
	Q_OBJECT
public:
	Bundler(QWidget *parent);
	~Bundler();

protected:
	bool eventFilter(QObject *obj, QEvent *e);

private slots:
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);
	void onRefresh();
	void onOpenFolder();
	void onSelectIcon();
	void onUseIcon();
	void onSaveTo();
	
private:
	void OpenFolderImpl(QString folder);
	void SelectIconImpl(QString icon);

private:
	Ui::Bundler ui;
	OpenArk *parent_;
	QString files_folder_;
	QString icon_file_;
	QMenu *files_menu_;
	QStandardItemModel *files_model_;
};