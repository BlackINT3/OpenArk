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
#include "cpp-wrapper.h"
#include "../common/common.h"

int VariantInt(std::string val, int radix)
{
	if (val.empty()) {
		return 0;
	}
	if (val.find("0n") == 0) {
		UNONE::StrReplaceA(val, "0n");
		return UNONE::StrToDecimalA(val);
	}
	if (val.find("0x") == 0 || val[val.size() - 1] == 'h') {
		UNONE::StrReplaceA(val, "0x");
		return UNONE::StrToHexA(val);
	}
	if (val.find("0t") == 0) {
		UNONE::StrReplaceA(val, "0t");
		return UNONE::StrToOctalA(val);
	}
	if (val.find("0y") == 0) {
		UNONE::StrReplaceA(val, "0y");
		return UNONE::StrToBinaryA(val);
	}
	switch (radix) {
	case 2: return UNONE::StrToBinaryA(val);
	case 8: return UNONE::StrToOctalA(val);
	case 10: return UNONE::StrToDecimalA(val);
	case 16: return UNONE::StrToHexA(val);
	default: return UNONE::StrToHexA(val);
	}
}

int64_t VariantInt64(std::string val, int radix)
{
	UNONE::StrReplaceA(val, "`");
	if (val.empty()) {
		return 0;
	}
	if (val.find("0n") == 0) {
		UNONE::StrReplaceA(val, "0n");
		return UNONE::StrToDecimal64A(val);
	}
	if (val.find("0x") == 0 || val[val.size() - 1] == 'h') {
		UNONE::StrReplaceA(val, "0x");
		return UNONE::StrToHex64A(val);
	}
	if (val.find("0t") == 0) {
		UNONE::StrReplaceA(val, "0t");
		return UNONE::StrToOctal64A(val);
	}
	if (val.find("0y") == 0) {
		UNONE::StrReplaceA(val, "0y");
		return UNONE::StrToBinary64A(val);
	}
	switch (radix) {
	case 2: return UNONE::StrToBinary64A(val);
	case 8: return UNONE::StrToOctal64A(val);
	case 10: return UNONE::StrToDecimal64A(val);
	case 16: return UNONE::StrToHex64A(val);
	default: return UNONE::StrToHex64A(val);
	}
}

std::wstring VariantFilePath(std::wstring path)
{
	UNONE::StrReplaceIW(path, L"file:///");
	return std::move(path);
}