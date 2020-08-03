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
#include "../common/common.h"

class CommonMainTabObject : public QTabWidget {
	Q_OBJECT
public:
	CommonMainTabObject() {};
	~CommonMainTabObject() {};

public:
	Q_INVOKABLE void SetActiveTab(QVector<int> idx) {
		tabwidget_->setCurrentIndex(idx[0]);
		auto tabwidgets = tabwidget_->currentWidget()->findChildren<QTabWidget*>();
		if (idx.size() >= 2 && tabwidgets.size() > 0) {
			auto obj = qobject_cast<QTabWidget*>(tabwidgets.at(0));
			obj->setCurrentIndex(idx[1]);
		}
	};
	Q_INVOKABLE int GetActiveTab() { return tabwidget_->currentIndex(); };
	Q_INVOKABLE void SetActiveTab(int idx) { tabwidget_->setCurrentIndex(idx); };

public slots:
	virtual void onTabChanged(int index) {
		auto tabwidgets = tabwidget_->currentWidget()->findChildren<QTabWidget*>();
		if (tabwidgets.size() > 0) {
			auto obj = qobject_cast<QTabWidget*>(tabwidgets.at(0));
			emit obj->currentChanged(obj->currentIndex());
		} else {
			QVector<int> tabs;
			tabs.push_back(index);
			tabs.push_back(0);
			OpenArkConfig::Instance()->SetMainTabMap(maintab_id_, tabs);
		}
	};

protected:
	void Init(QTabWidget *tabwidget, int maintab_id) {
		tabwidget_ = tabwidget;
		maintab_id_ = maintab_id;
		tabwidget_->setTabPosition(QTabWidget::West);
		tabwidget_->tabBar()->setStyle(new OpenArkTabStyle);
		connect(tabwidget_, SIGNAL(currentChanged(int)), this, SLOT(onTabChanged(int)));
	};
	int maintab_id_;

	QTabWidget *tabwidget_;
};

class CommonTabObject : public QWidget {
	Q_OBJECT

public:
	CommonTabObject() {};
	~CommonTabObject() {};

public:
	Q_INVOKABLE int GetActiveTab() { return tabwidget_->currentIndex(); };
	Q_INVOKABLE void SetActiveTab(QVector<int> idx) {
		tabwidget_->setCurrentIndex(idx[0]);
		qobject_cast<QTabWidget *>(tabwidget_->currentWidget())->setCurrentIndex(idx[1]);
	};

public slots:
	virtual void onTabChanged(int index) {
		QVector<int> tabs;
		tabs.push_back(l2tab_id_);
		tabs.push_back(index);
		OpenArkConfig::Instance()->SetMainTabMap(maintab_id_, tabs);
	};

protected:
	void Init(QTabWidget *tabwidget, int maintab_id, int l2tab_id) {
		tabwidget_ = tabwidget; 
		maintab_id_ = maintab_id;
		l2tab_id_ = l2tab_id;
		connect(tabwidget_, SIGNAL(currentChanged(int)), this, SLOT(onTabChanged(int)));
	};
	int maintab_id_;
	int l2tab_id_;
	
	QTabWidget *tabwidget_;
};