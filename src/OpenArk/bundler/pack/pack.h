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
#include <Windows.h>
#include <stdint.h>
#include <string>
#include <vector>

#define IDX_BUDE 108
#define IDS_BUDE L"IDR_BUDE"

enum BundleError {
	BERR_OK = 0,
	BERR_INVALID_FILEMAGIC,
	BERR_INVALID_HEADER,
	BERR_INVALID_COMPRESS,
	BERR_INVALID_SCRIPTCOUNT,
	BERR_INVALID_RECORDCOUNT,
	BERR_INVALID_RECORDSIZE,
	BERR_INVALID_RECORDCRC32,
	BERR_FAIL_COMPRESSED,
	BERR_FAIL_DECOMPRESSED,
	BERR_FAIL_OPEN,
	BERR_FAIL_READ,
	BERR_FAIL_WRITE,
	BERR_FAIL_CREATEDIR,
	BERR_FAIL_CREATEFILE,
};

BundleError BundlePack(const std::wstring &dirname, std::vector<std::wstring> &files,
	std::vector<std::wstring> &names, std::wstring script, std::string &bdata);

BundleError BundleUnpack(const std::wstring &outdir, const char *bdata, std::wstring &script, std::wstring &dirname);

BundleError BundleGetDirName(const char *bdata, std::wstring &dirname);