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
#include <QString>
#include <QSettings>

enum ConfOp {
	CONF_GET,
	CONF_SET,
};

static int def_lang_ = -1;

class OpenArkConfig {
public:
	void Init();
	static OpenArkConfig* Instance();
	int GetLang(ConfOp op, int &lang = def_lang_);
	QStringList GetJunkDirs();
	QString GetConsole(const QString &name);
	void GetMainGeometry(int &x, int &y, int &w, int &h);
	void SetMainGeometry(int x, int y, int w, int h);
	void GetPrefMainTab(int &idx);
	void SetPrefMainTab(int idx);
	void GetPrefLevel2Tab(int &idx);
	void SetPrefLevel2Tab(int idx);

	QVariant GetValue(const QString &key, const QVariant &defaultValue = QVariant()) const { return appconf_->value(key, defaultValue); };
	void SetValue(const QString &key, const QVariant &value) { return appconf_->setValue(key, value); };
	bool Contains(const QString &key) const { return appconf_->contains(key); }
	void Sync() { return appconf_->sync(); };

private:
	QSettings *appconf_;

public:
	static OpenArkConfig *confobj_;
	OpenArkConfig();
	~OpenArkConfig();
};
