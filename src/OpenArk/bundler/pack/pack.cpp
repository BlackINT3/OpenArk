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
#include "pack.h"
#include "../lz4/lz4.h"
#include "../crc32/crc32.h"
#include <unone.h>
#include <time.h>

/*******************************/
//BundleHeader
//...[Crypted]
//......[Compressed]
//.........BundleScript
//.........BundleRecord
//.........BundleRecord
//.....................
//......[Compressed]
//...[Crypted]
/*******************************/
#define BUNDLE_MAGIC	'BUDE'
#define BUNDLE_XORKEY	 0x99

#pragma pack(push,1)
struct BundleHeader {
	uint32_t magic;			//'BUDE'
	uint32_t timestamp;	//timestamp
	uint64_t orisize;		//original size
	uint64_t cpsize;		//compressed size
	uint32_t crc32;			//crc32
	uint32_t recordcnt;	//record count
	uint16_t dirname[MAX_PATH]; //root dir name
};

struct BundleScript {
	uint32_t len;
	uint16_t buf[1];
};

struct BundleRecord {
	uint64_t size;
	uint32_t crc32;
	uint16_t name[MAX_PATH];
	uint8_t data[1];
};
#pragma pack(pop)

BOOL BundleWriteFile(HANDLE fd, LPCVOID buf, DWORD buflen, LPDWORD writelen, LPOVERLAPPED overlapped)
{
	DWORD i = buflen;
	while (i--)	{
		((PBYTE)buf)[i] ^= BUNDLE_XORKEY;
	}
	BOOL result = WriteFile(fd, buf, buflen, writelen, overlapped);
	i = buflen;
	while (i--)	{
		((PBYTE)buf)[i] ^= BUNDLE_XORKEY;
	}
	return result;
}

BOOL BundleReadFile(HANDLE fd, LPVOID buf, DWORD buflen, LPDWORD readlen, LPOVERLAPPED overlapped)
{
	if (ReadFile(fd, buf, buflen, readlen, overlapped)) {
		DWORD i = *readlen;
		while (i--) {
			((PBYTE)buf)[i] ^= BUNDLE_XORKEY;
		}
	}
	return FALSE;
}

BOOL BundleReadMemory(const char* data, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, BOOL reset = FALSE)
{
	static DWORD offset = 0;
	if (reset)
		offset = 0;
	DWORD i = nNumberOfBytesToRead;
	while (i--) {
		((PBYTE)lpBuffer)[i] = data[offset + i] ^ BUNDLE_XORKEY;
	}
	offset += nNumberOfBytesToRead;
	return TRUE;
}

BOOL BundleCrypt(std::string &data)
{
	for (auto& c : data) {
		c ^= BUNDLE_XORKEY;
	}
	return TRUE;
}

BundleError BundleVerify(BundleHeader *bhdr)
{
	if (bhdr->magic != BUNDLE_MAGIC) {
		return BERR_INVALID_FILEMAGIC;
	}
	if (bhdr->cpsize < 0 || bhdr->orisize < 0) {
		return BERR_INVALID_HEADER;
	}
	if (bhdr->recordcnt < 0) {
		return BERR_INVALID_RECORDCOUNT;
	}
	return BERR_OK;
}

BundleError BundlePack(const std::wstring &dirname, std::vector<std::wstring> &files, 
	std::vector<std::wstring> &names, std::wstring script, std::string &bdata)
{
	BundleError err = BERR_OK;
	int recordcount = 0;
	std::string buf;
	BundleScript s = {0};
	s.len = script.size();
	buf.append((char*)&s, sizeof(BundleScript) - 2);
	buf.append((char*)script.c_str(), script.size() * 2);
	for (size_t i = 0; i < files.size(); i++) {
		std::string data;
		std::wstring path = files[i];
		std::wstring name = names[i];
		if (!UNONE::FsReadFileDataW(path, data)) {
			err = BERR_FAIL_READ;
			break;
		}
		BundleRecord record = { 0 };
		if (data.size() > 0) {
			record.crc32 = crc32(data.data(), data.size());
		}
		wcsncpy_s((wchar_t*)record.name, MAX_PATH, name.c_str(), MAX_PATH - 1);
		record.size = data.size();
		buf.append((char*)&record, sizeof(BundleRecord) - 1);
		buf.append(data);
	}
	if (err != BERR_OK) {
		return err;
	}
	int maxsize = LZ4_compressBound(buf.size());
	std::string cpdata;
	cpdata.resize(maxsize);
	int cpsize = LZ4_compress(buf.data(), (char *)cpdata.data(), buf.size());
	cpdata.resize(cpsize);
	BundleCrypt(cpdata);

	BundleHeader bhdr = { 0 };
	bhdr.magic = BUNDLE_MAGIC;
	bhdr.crc32 = crc32((unsigned char*)buf.data(), buf.size());
	bhdr.cpsize = cpsize;
	bhdr.orisize = buf.size();
	bhdr.recordcnt = files.size();
	bhdr.timestamp = (uint32_t)time(0);
	wcsncpy_s((wchar_t*)bhdr.dirname, MAX_PATH, dirname.c_str(), MAX_PATH - 1);
	std::string bstr((char*)&bhdr, sizeof(BundleHeader));
	BundleCrypt(bstr);
	bdata.append(bstr);
	bdata.append(cpdata);

	return BERR_OK;
}

BundleError BundleUnpack(const std::wstring &outdir, const char *bdata, std::wstring &script, std::wstring &dirname)
{
	BundleError err = BERR_OK;
	BundleHeader bhdr = { 0 };
	if (!BundleReadMemory(bdata, &bhdr, sizeof(bhdr), TRUE)) {
		return BERR_FAIL_READ;
	}
	err = BundleVerify(&bhdr);
	if (err != BERR_OK) {
		return err;
	}
	dirname = (wchar_t*)bhdr.dirname;
	uint64_t oribuflen = bhdr.orisize + 64;
	uint64_t cpbuflen = bhdr.cpsize;
	std::string cpdata, oridata;
	cpdata.resize((unsigned int)cpbuflen);
	oridata.resize((unsigned int)oribuflen);
	BundleReadMemory(bdata, (LPVOID)cpdata.data(), (DWORD)bhdr.cpsize);
	int size = LZ4_decompress_safe(cpdata.data(), (char*)oridata.data(), (int)cpbuflen, (int)oribuflen);
	if (size == 0) {
		return BERR_FAIL_DECOMPRESSED;
	}
	uint32_t hash = crc32(oridata.data(), size);
	if (hash != bhdr.crc32) {
		return BERR_INVALID_HEADER;
	}
	BundleScript *sptr = (BundleScript*)oridata.data();
	script.assign((wchar_t*)sptr->buf, sptr->len);

	BundleRecord *rptr = (BundleRecord*)((char*)sptr + sizeof(BundleScript) - 2 + sptr->len*2);
	for (int i = 0; i < (int)bhdr.recordcnt; i++) {
		if (rptr->size != 0) {
			if (crc32(rptr->data, (uint32_t)rptr->size) != rptr->crc32) {
				err = BERR_INVALID_RECORDCRC32;
				break;
			}
		}
		std::wstring&& path = UNONE::FsPathComposeW(outdir, (wchar_t*)rptr->name);
		std::wstring&& dir = UNONE::FsPathToDirW(path);
		if (!UNONE::FsCreateDirW(dir)) {
			err = BERR_FAIL_CREATEDIR;
			break;
		}
		HANDLE fd = CreateFileW(path.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (fd == INVALID_HANDLE_VALUE) {
			err = BERR_FAIL_CREATEFILE;
			break;
		}
		if (rptr->size != 0) {
			DWORD wlen;
			if (!WriteFile(fd, rptr->data, (DWORD)rptr->size, &wlen, NULL)) {
				err = BERR_FAIL_WRITE;
				CloseHandle(fd);
				break;
			}
		}
		rptr = (BundleRecord*)(((uint8_t*)rptr) + rptr->size - 1 + sizeof(BundleRecord));
		CloseHandle(fd);
	}
	return err;
}

BundleError BundleGetDirName(const char *bdata, std::wstring &dirname)
{
	BundleError err = BERR_OK;
	BundleHeader bhdr = { 0 };
	DWORD readlen = 0;
	if (!BundleReadMemory(bdata, &bhdr, sizeof(bhdr), TRUE)) {
		return BERR_FAIL_READ;
	}
	err = BundleVerify(&bhdr);
	if (err != BERR_OK) {
		return err;
	}
	dirname = (wchar_t*)bhdr.dirname;
	return err;
}