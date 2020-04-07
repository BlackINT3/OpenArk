#ifndef _CRT_RAND_S
#define _CRT_RAND_S
#endif
#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <unone.h>
#include <thread>
#include <string>
#include <vector>
#include "../pack/pack.h"

typedef std::tuple<std::wstring, std::wstring> DirectiveSeq;

#include <stdlib.h>
std::string TmRandHexString(int count)
{
	char str_temp[2] = { 0 };
	std::string str;
	int i, x;
	unsigned int randnum = 0;
	const char charset[] = "0123456789ABCDEF";
	if (count <= 0)
		return "";
	for (i = 0; i < count; ++i)
	{
		rand_s(&randnum);
		x = randnum % (unsigned)(sizeof(charset) - 1);
		sprintf_s(str_temp, "%c", charset[x]);
		str.append(str_temp);
	}
	return str;
}

void BundleExecStart(std::wstring param, std::wstring root)
{
	UNONE::StrReplaceIW(param, L"%root%", root);
	UNONE::PsCreateProcessW(param);
}

void BundleExecCall(std::wstring param, std::wstring root)
{
	UNONE::StrReplaceIW(param, L"%root%", root);
	PROCESS_INFORMATION pi;
	UNONE::PsCreateProcessW(param, SW_SHOW, &pi);
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}

void BundleExecCmd(std::wstring param, std::wstring root)
{
	UNONE::StrReplaceIW(param, L"%root%", root);
	UNONE::PsCreateProcessW(L"cmd.exe /c " + param, SW_HIDE);
}

void BundleExecClean(std::wstring param, std::wstring root)
{
	UNONE::FsDeleteDirectoryW(root);
}

void BundleOutputError(std::wstring msg)
{
	MessageBoxW(NULL, msg.c_str(), L"error", MB_ICONERROR);
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	bool found = false;
	LPVOID bdata = NULL;
	HRSRC res = FindResourceW(GetModuleHandleW(NULL), MAKEINTRESOURCE(IDX_BUDE), IDS_BUDE);
	if (res) {
		HGLOBAL hg = LoadResource(GetModuleHandleW(NULL), res);
		if (hg) {
			bdata = LockResource(hg);
			if (bdata) {
				DWORD size = SizeofResource(GetModuleHandleW(NULL), res);
				found = true;
			}
		}
	}
	std::string test;
	if (!found) {
#ifndef _DEBUG
		BundleOutputError(L"Resource is corrupted.");
		return 1;
#endif
#ifdef _DEBUG
		UNONE::FsReadFileDataA("c:/test.bd", test);
		if (test.empty())
			return 1;
#endif
		bdata = (LPVOID)test.c_str();
	}
	std::wstring dirname;
	BundleError err;
	err = BundleGetDirName((const char*)bdata, dirname);
	if (err != BERR_OK) {
		BundleOutputError(L"Directory name invalid.");
		return 1;
	}
	std::wstring parentdir, outdir, script;
	parentdir = UNONE::OsEnvironmentW(L"%temp%\\BUDE\\") + UNONE::StrToW(TmRandHexString(12));
	outdir = parentdir + L"\\" + dirname;
	UNONE::FsCreateDirW(outdir);
	err = BundleUnpack(outdir, (const char*)bdata, script, dirname);
	if (err != BERR_OK) {
		BundleOutputError(L"Unpack files error.");
		return 1;
	}
	std::vector<std::wstring> lines;
	UNONE::StrSplitLinesW(script, lines);
	std::vector<DirectiveSeq> directives;
	for (auto& line : lines) {
		if (line.find(';') != 0) {
			size_t pos = line.find(' ');
			std::wstring cmd, param;
			if (pos != std::wstring::npos) {
				cmd = line.substr(0, pos);
				param = line.substr(pos + 1);
			} else {
				cmd = line;
			}
			directives.push_back(std::make_tuple(cmd, param));
		}
	}

	std::vector<std::thread> async_tasks;
	for (auto &d : directives) {
		std::wstring cmd, param;
		std::tie(cmd, param) = d;
		if (UNONE::StrCompareIW(cmd, L"start")) {
			async_tasks.push_back(std::thread(BundleExecCall, param, outdir));
		}	else if (UNONE::StrCompareIW(cmd, L"call")) {
			BundleExecCall(param, outdir);
		}	else if (UNONE::StrCompareIW(cmd, L"cmd")) {
			BundleExecCmd(param, outdir);
		} else if (UNONE::StrCompareIW(cmd, L"clean")) {
			BundleExecClean(param, parentdir);
		}
	}
	for (auto &t : async_tasks) {
		t.join();
	}
	return 0;
}