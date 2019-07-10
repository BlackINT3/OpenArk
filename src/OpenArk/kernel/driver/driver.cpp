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
#include "../../common/common.h"
#include "../../../OpenArkDrv/arkdrv-api/arkdrv-api.h"
#include <Windows.h>
#include <tchar.h>
#include "driver.h"
#include "Wincrypt.h"  
#pragma comment(lib, "Crypt32.lib")

bool ImportPrivateKey(const std::string &private_key, WCHAR *passwd, WCHAR *category)
{
	bool ret = false;
	HCERTSTORE cert_store = NULL;
	HCERTSTORE cert_dst_store = NULL;
	PCCERT_CONTEXT cert = NULL;
	HCRYPTPROV crypt_prov = NULL;
	do {
		CRYPT_DATA_BLOB blob;
		blob.cbData = private_key.size();
		blob.pbData = (unsigned char *)private_key.c_str();
		cert_store = PFXImportCertStore(&blob, passwd, CRYPT_EXPORTABLE);
		if (!cert_store) {
			ERR("PFXImportCertStore err:%d", GetLastError());
			break;
		}
		DWORD key_spec = 0;
		BOOL free_prov = FALSE;
		cert = CertEnumCertificatesInStore(cert_store, NULL);
		if (!cert) {
			ERR("CertEnumCertificatesInStore err:%d", GetLastError());
			break;
		}
		if (!CryptAcquireCertificatePrivateKey(cert, CRYPT_ACQUIRE_COMPARE_KEY_FLAG, NULL, &crypt_prov, &key_spec, &free_prov) ||
			!crypt_prov) {
			ERR("CryptAcquireCertificatePrivateKey err:%d", GetLastError());
			break;
		}
		cert_dst_store = CertOpenStore(CERT_STORE_PROV_SYSTEM_W, 0, NULL, CERT_SYSTEM_STORE_CURRENT_USER | CERT_STORE_OPEN_EXISTING_FLAG, category);
		if (!cert_dst_store) {
			ERR("CertOpenStore err:%d", GetLastError());
			break;
		}
		ret = CertAddCertificateContextToStore(cert_dst_store, cert, CERT_STORE_ADD_REPLACE_EXISTING, NULL);
		if (!ret) {
			ERR("CertAddCertificateContextToStore err:%d", GetLastError());
			break;
		}
		ret = true;
	} while (0);

	if (cert) CertFreeCertificateContext(cert);
	if (crypt_prov) CryptReleaseContext(crypt_prov, 0);
	if (cert_dst_store) CertCloseStore(cert_dst_store, 0);
	if (cert_store) CertCloseStore(cert_store, 0);

	return ret;
}

bool SignExpiredDriver(QString driver)
{
	if (driver.isEmpty()) {
		QERR_W("driver is empty");
		return false;
	}

	QString res = ":/OpenArk/sign/CSignTool.pfx";
	QFile pfx(res);
	if (!pfx.open(QFileDevice::ReadOnly)) {
		QERR_W("open resource file %s err", QToWChars(res));
		return false;
	}
	if (!ImportPrivateKey(pfx.readAll().toStdString(), _T("TrustAsia.com"), _T("My"))) {
		QERR_W("import pfx certificate file %s err", QToWChars(res));
		return false;
	}

	auto &&toodir = UNONE::OsEnvironmentW(L"%AppData%\\TrustAsia\\DSignTool");
	auto &&signtool = toodir + L"\\CSignTool.exe";
	auto &&signcfg = toodir + L"\\Config.xml";

	ExtractResource(":/OpenArk/sign/CSignTool.exe", WStrToQ(signtool));
	ExtractResource(":/OpenArk/sign/Config.xml", WStrToQ(signcfg));

	RECOVER_SIGN_TIME();

	std::wstring cmdline;
	std::wstring &&path = driver.toStdWString();
	UNONE::StrFormatW(cmdline, L"%s sign /r Driver /f \"%s\" /ac", signtool.c_str(), path.c_str());
	PROCESS_INFORMATION pi;
	if (!UNONE::PsCreateProcessW(cmdline, SW_HIDE, &pi)) {
		QERR_W("run cmdline:%s err", cmdline.c_str());
		return false;
	}
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return true;
}

std::wstring ParseDriverPath(UCHAR *symlnk)
{
	std::wstring &&path = UNONE::StrToW((char*)symlnk);
	std::wstring sysroot = L"\\SystemRoot";
	auto pos = path.find(sysroot);
	if (pos == 0) path.replace(0, sysroot.size(), UNONE::OsWinDirW());
	UNONE::StrReplaceW(path, L"\\??\\");
	return path;
}