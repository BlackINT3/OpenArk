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
#include "coderkit.h"
#include "../common/common.h"
#include "../openark/openark.h"
#include <locale>
#include <codecvt>
#if (!_DLL) && (_MSC_VER >= 1900 /* VS 2015*/) && (_MSC_VER <= 1911 /* VS 2017 */)
std::locale::id std::codecvt<char16_t, char, _Mbstatet>::id;
#endif

CoderKit::CoderKit(QWidget* parent) :
	parent_((OpenArk*)parent)
{
	ui.setupUi(this);
	ui.tabWidget->setTabPosition(QTabWidget::West);
	ui.tabWidget->tabBar()->setStyle(new OpenArkTabStyle);
	ui.tabWidget->setStyleSheet("background-color: white;");

	//QRegExp re("[0-9]+$");
	//QRegExpValidator *validator = new QRegExpValidator(re, this);
	//ui.defaultEdit->setValidator(validator);

	connect(ui.textEdit, SIGNAL(textChanged()), this, SLOT(onCodeTextChanged()));
	connect(ui.defaultEdit, SIGNAL(textChanged(const QString &)), this, SLOT(onCodeTextChanged(const QString &)));
	connect(ui.asciiEdit, SIGNAL(textChanged(const QString &)), this, SLOT(onCodeTextChanged(const QString &)));
	connect(ui.unicodeEdit, SIGNAL(textChanged(const QString &)), this, SLOT(onCodeTextChanged(const QString &)));
	connect(ui.utf7Edit, SIGNAL(textChanged(const QString &)), this, SLOT(onCodeTextChanged(const QString &)));
	connect(ui.utf8Edit, SIGNAL(textChanged(const QString &)), this, SLOT(onCodeTextChanged(const QString &)));
	connect(ui.utf16Edit, SIGNAL(textChanged(const QString &)), this, SLOT(onCodeTextChanged(const QString &)));
	connect(ui.butf16Edit, SIGNAL(textChanged(const QString &)), this, SLOT(onCodeTextChanged(const QString &)));
	connect(ui.utf32Edit, SIGNAL(textChanged(const QString &)), this, SLOT(onCodeTextChanged(const QString &)));
	connect(ui.butf32Edit, SIGNAL(textChanged(const QString &)), this, SLOT(onCodeTextChanged(const QString &)));
	connect(ui.gbkEdit, SIGNAL(textChanged(const QString &)), this, SLOT(onCodeTextChanged(const QString &)));
	connect(ui.big5Edit, SIGNAL(textChanged(const QString &)), this, SLOT(onCodeTextChanged(const QString &)));
	connect(ui.cp866Edit, SIGNAL(textChanged(const QString &)), this, SLOT(onCodeTextChanged(const QString &)));

	connect(ui.doserrEdit, SIGNAL(textChanged(const QString &)), this, SLOT(onWindowsErrorTextChanged(const QString &)));
	connect(ui.ntstatusEdit, SIGNAL(textChanged(const QString &)), this, SLOT(onWindowsErrorTextChanged(const QString &)));
	connect(ui.hresultEdit, SIGNAL(textChanged(const QString &)), this, SLOT(onWindowsErrorTextChanged(const QString &)));
	connect(ui.msgidBtn, SIGNAL(clicked()), this, SLOT(onMessageId()));
}
 
CoderKit::~CoderKit()
{
}

void CoderKit::onCodeTextChanged()
{
	std::wstring data;
	std::string str;
	QObject* sender = QObject::sender();
	if (sender == ui.textEdit) {
		data = ui.textEdit->toPlainText().toStdWString();
	}

	UpdateEditCodeText(data, sender);
}

typedef std::wstring_convert<std::codecvt_utf8<int16_t>, int16_t> U16Convert;
typedef U16Convert::wide_string U16;
typedef std::wstring_convert<std::codecvt_utf8<int32_t>, int32_t> U32Convert;
typedef U32Convert::wide_string U32;

void CoderKit::onCodeTextChanged(const QString & text)
{
	QLineEdit* sender = qobject_cast<QLineEdit*>(QObject::sender());
	sender->setStyleSheet("background-color:white");
	try {
		std::string str = sender->text().toStdString();
		std::wstring data;
		auto InputFilter = [&](std::string& input) {
			UNONE::StrReplaceA(input, "-");
			UNONE::StrReplaceA(input, " ");
			UNONE::StrReplaceA(input, "0x");
			UNONE::StrReplaceA(input, "h");
			UNONE::StrReplaceA(input, "\\x");
			sender->setText(StrToQ(input));
		};
		InputFilter(str);
		str = UNONE::StrHexStrToStreamA(str);
		if (sender == ui.defaultEdit) {
			data = UNONE::StrACPToWide(str);
		}	else if (sender == ui.asciiEdit) {
			data = UNONE::StrCodeToWide(437, str);
		}	else if (sender == ui.unicodeEdit) {
			data = std::wstring((wchar_t*)str.c_str(), str.size() / 2);
		} else if (sender == ui.utf7Edit) {
			data = UNONE::StrCodeToWide(CP_UTF7, str);
		}	else if (sender == ui.utf8Edit) {
			data = UNONE::StrCodeToWide(CP_UTF8, str);
		}	else if (sender == ui.utf16Edit) {
			U16 utf16((int16_t*)str.c_str(), str.size() / 2);
			data = UNONE::StrUTF8ToWide(U16Convert().to_bytes(utf16));
		}	else if (sender == ui.butf16Edit) {
			str = UNONE::StrReverseA(str, 2);
			U16 utf16((int16_t*)str.c_str(), str.size() / 2);
			data = UNONE::StrUTF8ToWide(U16Convert().to_bytes(utf16));
		}	else if (sender == ui.utf32Edit) {
			U32 utf32((int32_t*)str.c_str(), str.size() / 4);
			data = UNONE::StrUTF8ToWide(U32Convert().to_bytes(utf32));
		} else if (sender == ui.butf32Edit) {
			str = UNONE::StrReverseA(str, 4);
			U32 utf32((int32_t*)str.c_str(), str.size() / 4);
			data = UNONE::StrUTF8ToWide(U32Convert().to_bytes(utf32));
		}	else if (sender == ui.gbkEdit) {
			data = UNONE::StrCodeToWide(936, str);
		}	else if (sender == ui.big5Edit) {
			data = UNONE::StrCodeToWide(950, str);
		}	else if (sender == ui.cp866Edit) {
			data = UNONE::StrCodeToWide(866, str);
		}
		UpdateEditCodeText(data, sender);
	} catch(...) {
		sender->setStyleSheet("background-color:red");
	}
}

void CoderKit::onWindowsErrorTextChanged(const QString & text)
{
	DWORD doserr;
	std::string number = text.toStdString();
	QLineEdit* sender = qobject_cast<QLineEdit*>(QObject::sender());
	if (sender == ui.doserrEdit) {
		ui.msgEdit->setText(WStrToQ(UNONE::OsDosErrorMsgW(UNONE::StrToIntegerA(number))));
	}	else if (sender == ui.ntstatusEdit) {
		doserr = UNONE::OsNtToDosError((UNONE::StrToHexA(number)));
		ui.doserrEdit->setText(QString("%1").arg(doserr));
		ui.msgEdit->setText(WStrToQ(UNONE::OsDosErrorMsgW(doserr)));
	}	else if (sender == ui.hresultEdit) {
		doserr = UNONE::StrToHexA(number) & 0xFFFF;
		ui.doserrEdit->setText(QString("%1").arg(doserr));
		ui.msgEdit->setText(WStrToQ(UNONE::OsDosErrorMsgW(doserr)));
	}
}

void CoderKit::onMessageId()
{
	parent_->onExecCmd(L".msg");
	MsgBoxInfo(tr("Open console to view result"));
}

void CoderKit::UpdateEditCodeText(const std::wstring& data, QObject* ignored_obj)
{
	//prevent multicall simultaneously
	std::unique_lock<std::mutex> guard(upt_mutex_, std::try_to_lock);
	if (!guard.owns_lock()) return;

	auto SetText = [&](QObject* obj, std::string data) {
		if (obj == ignored_obj) return;
		const char* class_name = obj->metaObject()->className();
		if (class_name == QStringLiteral("QTextEdit")) {
			qobject_cast<QTextEdit*>(obj)->setText(StrToQ(data));
		}	else if (class_name == QStringLiteral("QLineEdit")) {
			data = UNONE::StrTrimA(data);
			qobject_cast<QLineEdit*>(obj)->setText(StrToQ(data));
		}
	};

	std::string text;
	text = UNONE::StrWideToUTF8(data);
	SetText(ui.textEdit, text);
	
	text = UNONE::StrStreamToHexStrA(UNONE::StrWideToACP(data));
	SetText(ui.defaultEdit, text);

	text = UNONE::StrStreamToHexStrA(UNONE::StrACPToCode(437, UNONE::StrToA(data)));
	SetText(ui.asciiEdit, text);

	text = UNONE::StrStreamToHexStrA(std::string((char*)data.c_str(), data.size() * 2));
	SetText(ui.unicodeEdit, text);

	text = UNONE::StrStreamToHexStrA(UNONE::StrWideToCode(CP_UTF7, data));
	SetText(ui.utf7Edit, text);

	text = UNONE::StrStreamToHexStrA(UNONE::StrWideToCode(CP_UTF8, data));
	SetText(ui.utf8Edit, text);

	U16Convert cvt16;
	auto utf16 = cvt16.from_bytes(UNONE::StrWideToCode(CP_UTF8, data));
	auto stream = std::string((char*)utf16.c_str(), utf16.size() * 2);
	text = UNONE::StrStreamToHexStrA(stream);
	SetText(ui.utf16Edit, text);

	stream = UNONE::StrReverseA(stream, 2);
	text = UNONE::StrStreamToHexStrA(stream);
	SetText(ui.butf16Edit, text);

	U32Convert cvt32;
	auto utf32 = cvt32.from_bytes(UNONE::StrWideToCode(CP_UTF8, data));
	stream = std::string((char*)utf32.c_str(), utf32.size() * 4);
	text = UNONE::StrStreamToHexStrA(stream);
	SetText(ui.utf32Edit, text);

	stream = UNONE::StrReverseA(stream, 4);
	text = UNONE::StrStreamToHexStrA(stream);
	SetText(ui.butf32Edit, text);

	text = UNONE::StrStreamToHexStrA(UNONE::StrWideToCode(936, data));
	SetText(ui.gbkEdit, text);

	text = UNONE::StrStreamToHexStrA(UNONE::StrWideToCode(950, data));
	SetText(ui.big5Edit, text);

	text = UNONE::StrStreamToHexStrA(UNONE::StrWideToCode(866, data));
	SetText(ui.cp866Edit, text);
}