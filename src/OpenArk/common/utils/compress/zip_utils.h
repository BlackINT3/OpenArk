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
#include <string>
#include <unone.h>

namespace Plugin {
namespace Compressor {
class ZipUtils {
public:
	enum WRAPPER_TYPE {
		UNPACK_CURRENT, //uncompress to /
		UNPACK_SUBDIR,	//uncompress to /sub-dir
		PACK_CURRENT, //compress /
		PACK_SUBDIR, //compress /sub-dir
	};

	static bool UnpackToDir(const std::string& zip_path, 
		WRAPPER_TYPE wrapper_type, const std::string& out_dir);

	static bool PackDir(const std::string& dir, 
		WRAPPER_TYPE  wrapper_type, const std::string& zip_path);

	static bool PackFile(const std::string& file,
		const std::string& zip_path);

public:
	ZipUtils(){};
	~ZipUtils(){};
};
}	//namespace Compressor
} //namespace Plugin