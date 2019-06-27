#include "../../common/common.h"
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
	QString res = ":/OpenArk/CSignTool.pfx";
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

	ExtractResource(":/OpenArk/CSignTool.exe", WStrToQ(signtool));
	ExtractResource(":/OpenArk/Config.xml", WStrToQ(signcfg));

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