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
#include "zip_utils.h"
#include "xzip/XUnzip.h"
#include "xzip/XZip.h"

namespace Plugin {
namespace Compressor {

bool ZipUtils::UnpackToDir(const std::string& zip_path, WRAPPER_TYPE wrapper_type, const std::string& out_dir) {
	bool result = true;
#ifdef _UNICODE
	auto wzip_path = UNONE::StrToW(zip_path);
	HZIP h_zip = OpenZip((void*)wzip_path.c_str(), 0, ZIP_FILENAME);
#else
	HZIP h_zip = OpenZip((void*)zip_path.c_str(), 0, ZIP_FILENAME);
#endif
	if (h_zip == NULL) {
		int x = lasterrorU;
		printf("Open zip:%s err:%d", zip_path.c_str(), lasterrorU);
		return false;
	}

	ZIPENTRY ze;
	unsigned long ret = 0;
	for (int index=0; ret==0; index++) {
		ret = GetZipItemA(h_zip, index, &ze);
		if (ret == 0) {
			std::string name = ze.name;
			if (wrapper_type == UNPACK_SUBDIR) {
				auto pos = name.find("/");
				if (pos != std::string::npos) {
					name = name.substr(pos+1);
				}
			}
			if (name.empty()) {
				continue;
			}
			std::string&& path = UNONE::FsPathComposeA(out_dir, name);
			if (ze.attr & FILE_ATTRIBUTE_DIRECTORY) {
				UNONE::FsCreateDirA(path);
				continue;
			}
			UNONE::FsCreateDirA(UNONE::FsPathToDirA(path));
#ifdef _UNICODE
			auto wpath = UNONE::StrToW(path);
			if (UnzipItem(h_zip, index, (void*)wpath.c_str(), 0, ZIP_FILENAME) != 0) {
#else
			if (UnzipItem(h_zip, index, (void*)path.c_str(), 0, ZIP_FILENAME) != 0) {
#endif
				printf("UnzipItem %s to dir:%s err:%d", ze.name, out_dir.c_str(), lasterrorU);
				result = false;
				break;
			}
			result = true;
		}
	}
	CloseZip(h_zip);
	return result;
}

bool ZipUtils::PackDir(const std::string& dir, WRAPPER_TYPE wrapper_type, const std::string& zip_path)
{
	if (!UNONE::FsIsExistedA(dir)) {
		printf("Dir:%s not existed", dir.c_str());
		return false;
	}
#ifdef _UNICODE
	auto wzip_path = UNONE::StrToW(zip_path);
	HZIP h_zip = CreateZip((void *)wzip_path.c_str(), 0, ZIP_FILENAME);
#else
	HZIP h_zip = CreateZip((void *)zip_path.c_str(), 0, ZIP_FILENAME);
#endif
	if(h_zip == NULL) {
		printf("Create zip_path:%s failed, error %d", zip_path.c_str(), lasterrorU);
		return false;
	}

	bool result = true;
	std::vector<std::string> path_vec;
	std::string std_dir = UNONE::FsPathStandardA(dir);
	UNONE::DirEnumCallbackA fcb = [&](__in char* path, __in char* name, __in void* param)->bool {
		if (UNONE::FsIsDirA(path)) {
			UNONE::FsEnumDirectoryA(path, fcb, param);;
		}
		auto& path_vec = *reinterpret_cast<std::vector<std::string>*>(param);
		path_vec.push_back(path);
		return true;
	};
	UNONE::FsEnumDirectoryA(std_dir, fcb, &path_vec);

	if (wrapper_type == PACK_CURRENT) {
		std_dir = UNONE::FsPathToDirA(std_dir);
	}
	std_dir.append("\\");

	for (size_t i = 0; i < path_vec.size(); i++) {
		ZRESULT zip_ret = 0;
		const std::string& path = path_vec[i];
		std::string rel_name = path;
		UNONE::StrReplaceA(rel_name, std_dir, "");
		auto flag = ZIP_FILENAME;
		if (UNONE::FsIsDirA(path)) {
			flag = ZIP_FOLDER;
		}
#ifdef _UNICODE
		auto wpath = UNONE::StrToW(path);
		zip_ret = ZipAdd(h_zip, rel_name.c_str(), (void*)wpath.c_str(), 0, flag);
#else
		zip_ret = ZipAdd(h_zip, rel_name.c_str(), (void*)path.c_str(), 0, flag);
#endif
		if (zip_ret != ZR_OK) {
			printf("ZipAdd file:%s err:%d", path.c_str(), lasterrorU);
			result = false;
			break;
		}
	}

	CloseZip(h_zip);
	if (!result) {
		DeleteFileA(zip_path.c_str());
	}

	return result;
}

bool ZipUtils::PackFile(const std::string& file, const std::string& zip_path)
{
	bool result = true;

	if (!UNONE::FsIsExistedA(file)) {
		printf("File:%s not exsited", file.c_str());
		return false;
	}
#ifdef _UNICODE
	auto wzip_path = UNONE::StrToW(zip_path);
	HZIP h_zip = CreateZip((void *)wzip_path.c_str(), 0, ZIP_FILENAME);
#else
	HZIP h_zip = CreateZip((void *)zip_path.c_str(), 0, ZIP_FILENAME);
#endif
	if(h_zip == NULL) {
		printf("Create zip_path:%s err:%d", zip_path.c_str(), lasterrorU);
		return false;
	}

	ZRESULT zip_ret = 0;
	std::string&& rel_name = UNONE::FsPathToNameA(file);
#ifdef _UNICODE
	auto wpath = UNONE::StrToW(file);
	zip_ret = ZipAdd(h_zip, rel_name.c_str(), (void*)wpath.c_str(), 0, ZIP_FILENAME);
#else
	zip_ret = ZipAdd(h_zip, rel_name.c_str(), (void*)file.c_str(), 0, ZIP_FILENAME);
#endif
	if (zip_ret != ZR_OK) {
		result = false;
	}

	CloseZip(h_zip);
	if (!result) {
		DeleteFileA(zip_path.c_str());
	}

	return result;
}
}	//namespace Compressor
} //namespace Plugin