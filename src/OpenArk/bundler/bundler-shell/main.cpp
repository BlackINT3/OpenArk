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
bool ParseCmdline(const std::wstring& cmdline, std::vector<std::wstring>& vec)
{
	try {
		int argc = 0;
		LPWSTR* argv = CommandLineToArgvW(cmdline.c_str(), &argc);
		for (int Index = 0; Index < argc; ++Index) {
			vec.push_back(argv[Index]);
		}
		HeapFree(GetProcessHeap(), 0, argv);
		return !vec.empty();
	}
	catch (...) {
		vec.clear();
		return false;
	}
}
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

void BundleExecCall(std::wstring param, std::wstring root, int show = SW_SHOW, bool wait = true)
{
	UNONE::StrReplaceIW(param, L"%root%", root);
	std::vector<std::wstring> vec;
	ParseCmdline(param, vec);
	if (vec.empty()) return;
	param.clear();
	for (auto i = 1; i < vec.size(); i++) {
		param.append(vec[i]);
		param.append(L" ");
	}
	SHELLEXECUTEINFOW sh = { 0 };
	sh.cbSize = sizeof(SHELLEXECUTEINFOW);
	sh.fMask = SEE_MASK_NOCLOSEPROCESS;
	sh.hwnd = NULL;
	sh.lpVerb = NULL;
	sh.lpFile = vec[0].c_str();
	sh.lpParameters = param.c_str();
	sh.lpDirectory = NULL;
	sh.nShow = show;
	sh.hInstApp = NULL;
	ShellExecuteExW(&sh);
	if (wait) WaitForSingleObject(sh.hProcess, INFINITE);
	CloseHandle(sh.hProcess);
}

void BundleExecCmd(std::wstring param, std::wstring root)
{
	UNONE::StrReplaceIW(param, L"%root%", root);
	BundleExecCall(L"cmd.exe /c " + param, root, false);
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
			async_tasks.push_back(std::thread(BundleExecCall, param, outdir, SW_SHOW, true));
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