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

typedef std::wstring_convert<std::codecvt_utf8<int16_t>, int16_t> U16Convert;
typedef U16Convert::wide_string U16;
typedef std::wstring_convert<std::codecvt_utf8<int32_t>, int32_t> U32Convert;
typedef U32Convert::wide_string U32;

// Algorithm index
struct {
	int s = 0;
	int base64 = s++;
	int crc32 = s++;
	int md5 = s++;
	int sha1 = s++;
	int rc4 = s++;
} IDX;

struct {
	int s = 0;
	int bits64 = s++;
	int bits32 = s++;
	int bits16 = s++;
} BITS_IDX;

CoderKit::CoderKit(QWidget* parent, int tabid) :
	parent_((OpenArk*)parent)
{
	ui.setupUi(this);
	connect(OpenArkLanguage::Instance(), &OpenArkLanguage::languageChaned, this, [this]() {ui.retranslateUi(this); });

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

	alg_idx_ = 0;
	onAlgIndexChanged(alg_idx_);
	ui.typeBox->insertItem(IDX.base64, "Base64");
	ui.typeBox->insertItem(IDX.crc32, "CRC32");
	ui.typeBox->insertItem(IDX.md5, "MD5");
	ui.typeBox->insertItem(IDX.sha1, "SHA1");
	//ui.typeBox->insertItem(IDX.rc4, "RC4");

	connect(ui.typeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onAlgIndexChanged(int)));
	connect(ui.plainEdit, SIGNAL(textChanged()), this, SLOT(onAlgPlainChanged()));
	connect(ui.cipherEdit, SIGNAL(textChanged()), this, SLOT(onAlgPlainChanged()));
	//connect(ui.keyEdit, SIGNAL(textChanged()), this, SLOT(onAlgPlainChanged()));

	InitAsmToolsView();

	CommonMainTabObject::Init(ui.tabWidget, tabid);
}
 
CoderKit::~CoderKit()
{
}

void CoderKit::onTabChanged(int index)
{
	CommonMainTabObject::onTabChanged(index);
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
			data = std::wstring((wchar_t*)str.c_str(), str.size() / 2);
		}	else if (sender == ui.butf16Edit) {
			str = UNONE::StrReverseA(str, 2);
			data = std::wstring((wchar_t*)str.c_str(), str.size() / 2);
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
	std::string number = text.toStdString();
	QLineEdit* sender = qobject_cast<QLineEdit*>(QObject::sender());
	if (sender == ui.doserrEdit) {
		auto err = VariantInt(number, 10);
		auto msg = UNONE::StrFormatW(L"%d: %s", err, UNONE::OsDosErrorMsgW(err).c_str());
		ui.msgEdit->setText(WStrToQ(msg));
	}	else if (sender == ui.ntstatusEdit) {
		auto err = VariantInt(number, 16);
		auto doserr = UNONE::OsNtToDosError((VariantInt(number, 16)));
		auto msg = UNONE::StrFormatW(L"%X: %s", err, UNONE::OsDosErrorMsgW(doserr).c_str());
		ui.doserrEdit->setText(QString("%1").arg(doserr));
		ui.msgEdit->setText(WStrToQ(msg));
	}	else if (sender == ui.hresultEdit) {
		auto hr = VariantInt(number, 16);
		auto doserr = hr & 0xFFFF;
		ui.doserrEdit->setText(QString("%1").arg(doserr));
		auto msg = UNONE::StrFormatW(L"%X: %s", hr, UNONE::OsDosErrorMsgW(doserr).c_str());
		ui.msgEdit->setText(WStrToQ(msg));
	}
}

void CoderKit::onMessageId()
{
	parent_->onExecCmd(L".msg");
	MsgBoxInfo(tr("Open console to view result"));
}

void CoderKit::onAlgIndexChanged(int index)
{
	alg_idx_ = index;

	auto e_key = ui.keyEdit;
	auto e_plain = ui.plainEdit;
	auto l_plain = ui.keyLabel;
	auto e_cipher = ui.cipherEdit;

	e_key->hide();
	l_plain->hide();
	if (index == IDX.rc4) {
		e_key->show();
		l_plain->show();
		UpdateAlgorithmText(false);
		return;
	}

	UpdateAlgorithmText(true);
	return;
}

void CoderKit::onAlgPlainChanged()
{
	auto sender = qobject_cast<QTextEdit*>(QObject::sender());
	if (sender == ui.plainEdit) {
		UpdateAlgorithmText(true);
	} else if (sender == ui.cipherEdit) {
		UpdateAlgorithmText(false);
	} else if (sender == ui.keyEdit) {
		UpdateAlgorithmText(true);
	} 
}

void CoderKit::InitAsmToolsView()
{
	ui.splitter->setStretchFactor(0, 1);
	ui.splitter->setStretchFactor(1, 2);
	ui.nullRadio->setChecked(true);
	connect(ui.asmBtn, &QPushButton::clicked, this, [&]() {
		int bits = 64;
		auto idx = ui.platformBox->currentIndex();
		if (idx == BITS_IDX.bits64) bits = 64;
		else if (idx == BITS_IDX.bits32) bits = 32;
		else if (idx == BITS_IDX.bits16) bits = 16;
		auto &&in = ui.asmEdit->toPlainText().toStdString();

		std::string formats;
		if (ui.nullRadio->isChecked()) formats = "";
		else if (ui.spaceRadio->isChecked()) formats = " ";
		else if (ui.slashxRadio->isChecked()) formats = "\\x";

		auto &&out = NasmAsm(in, bits, formats);
		ui.disasmEdit->setText(out);
	});

	connect(ui.disasmBtn, &QPushButton::clicked, this, [&]() {
		int bits = 64;
		auto idx = ui.platformBox->currentIndex();
		if (idx == BITS_IDX.bits64) bits = 64;
		else if (idx == BITS_IDX.bits32) bits = 32;
		else if (idx == BITS_IDX.bits16) bits = 16;
		auto &&in = ui.asmEdit->toPlainText().toStdString();
		const char *pfx = "file:///";
		auto pos = in.find(pfx);
		if (pos == 0) {
			auto file = UNONE::StrToW(in.substr(pos + strlen(pfx)));
			UNONE::FsReadFileDataW(file, in);
		} else {
			UNONE::StrReplaceA(in, " ");
			UNONE::StrReplaceA(in, "\\x");
			in = UNONE::StrHexStrToStreamA(in);
		}
		if (in.size() >= 10 * KB) {
			auto msbox = QMessageBox::warning(this, tr("Warning"),
				tr("Your input data so much(suggest less 10 KB), it'll be very slowly, continue?"),
				QMessageBox::Yes | QMessageBox::No);
			if (msbox == QMessageBox::No) return;
		}

		auto &&out = NasmDisasm(in, bits);
		ui.disasmEdit->setText(out);
	});
}

void CoderKit::UpdateAlgorithmText(bool crypt)
{
	auto e_key = ui.keyEdit;
	std::string key = e_key->toPlainText().toStdString();

	auto e_plain = ui.plainEdit;
	std::string plain = e_plain->toPlainText().toStdString();

	auto e_cipher = ui.cipherEdit;
	std::string cipher;

	if (alg_idx_ == IDX.base64) {
		if (crypt) {
			cipher = Cryptor::Base64Encode(plain);
		} else {
			cipher = e_cipher->toPlainText().toStdString();
			plain = Cryptor::Base64Decode(cipher);
			e_plain->blockSignals(true);
			e_plain->setText(StrToQ(plain));
			e_plain->blockSignals(false);
			return;
		}
	}	else if (alg_idx_ == IDX.crc32) {
		auto val = Cryptor::GetCRC32ByData(plain);
		cipher = UNONE::StrFormatA("%x", val);
	} else if (alg_idx_ == IDX.md5) {
		cipher = Cryptor::GetMD5ByData(plain);
		cipher = UNONE::StrStreamToHexStrA(cipher);
	}	else if (alg_idx_ == IDX.sha1) {
		cipher = Cryptor::GetSHA1ByData(plain);
		cipher = UNONE::StrStreamToHexStrA(cipher);
	}	else if (alg_idx_ == IDX.rc4) {
		cipher = Cryptor::GetSHA1ByData(plain);
	}

	if (!crypt) return;
	e_cipher->blockSignals(true);
	e_cipher->setText(StrToQ(cipher));
	e_cipher->blockSignals(false);
}

void CoderKit::UpdateEditCodeText(const std::wstring& data, QObject* ignored_obj)
{
	//prevent multi call simultaneously
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

	text = UNONE::StrStreamToHexStrA(std::string((char*)data.c_str(), data.size() * 2));
	SetText(ui.utf16Edit, text);

	auto stream = std::string((char*)data.c_str(), data.size() * 2);
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

QString CoderKit::NasmAsm(std::string data, int bits, const std::string &format)
{
	if (bits == 64) data.insert(0, "[bits 64]\n");
	else if (bits == 32) data.insert(0, "[bits 32]\n");
	else if (bits == 16) data.insert(0, "[bits 16]\n");

	auto &&nasm = AppConfigDir() + L"\\nasm\\nasm.exe";
	if (!UNONE::FsIsExistedW(nasm)) {
		ExtractResource(":/OpenArk/nasm/nasm.exe", WStrToQ(nasm));
	}
	auto &&tmp_out = UNONE::OsEnvironmentW(L"%Temp%\\temp-nasm-code.bin");
	auto &&tmp_in = UNONE::OsEnvironmentW(L"%Temp%\\temp-nasm-code.asm");
	UNONE::FsWriteFileDataW(tmp_in, data);
	auto &&cmdline = UNONE::StrFormatW(L"%s -f bin -o \"%s\" \"%s\"", nasm.c_str(), tmp_out.c_str(), tmp_in.c_str());
	std::wstring out;
	DWORD exitcode;
	QString err_prefix = tr("Compile Error:\n--------------------------------------------------------------\n");
	auto ret = ReadStdout(cmdline, out, exitcode);
	if (!ret) return err_prefix + tr("start nasm error");
	if (exitcode != 0) return err_prefix + WStrToQ(out);
	std::string bin;
	UNONE::FsReadFileDataW(tmp_out, bin);

	bin = UNONE::StrStreamToHexStrA(bin);
	bin = format + UNONE::StrInsertA(bin, 2, format);
	return StrToQ(bin);
}

QString CoderKit::NasmDisasm(const std::string &data, int bits)
{
	auto &&ndisasm = AppConfigDir() + L"\\nasm\\ndisasm.exe";
	if (!UNONE::FsIsExistedW(ndisasm)) {
		ExtractResource(":/OpenArk/nasm/ndisasm.exe", WStrToQ(ndisasm));
	}
	auto &&tmp_in = UNONE::OsEnvironmentW(L"%Temp%\\temp-ndisasm-code.bin");
	UNONE::FsWriteFileDataW(tmp_in, data);
	auto &&cmdline = UNONE::StrFormatW(L"%s -b %d \"%s\"", ndisasm.c_str(), bits, tmp_in.c_str());
	std::wstring out;
	DWORD exitcode;
	auto ret = ReadStdout(cmdline, out, exitcode);
	if (!ret) return tr("start ndisasm error");
	UNONE::StrLowerW(out);
	return WStrToQ(out);
}