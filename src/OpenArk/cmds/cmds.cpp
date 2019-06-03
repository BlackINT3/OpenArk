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
#include "cmds.h"
#include "constants/constants.h"
#include <time.h>	

struct CommandHelpItem {
	std::wstring cmd;
	std::string func;
	std::wstring doc;
	std::wstring example;
} CmdTable[] = {
{ L".help", "CmdHelp", LR"(show command manuals)", 
LR"(.help [show commands help and examples]
.help ps [show commands help matches *ps*, eg:.ps])"},

{ L".cls", "CmdCls", LR"(clear console screen)", L"" },

{ L".history", "CmdHistory", LR"(show commands history)" , L"" },

{ L".exit", "CmdExit", LR"(exit current process)" , L"" },

{ L".restart", "CmdRestart", LR"(restart current process)" , L"" },

{ L".cmd", "CmdCmd", LR"(sync execute command line, pipe console output)" , LR"(.cmd whoami [PC\Administrator])" },

{ L".start", "CmdStart", LR"(async execute command line)" , LR"(.start taskmgr [open taskmgr])" },

{ L".ts", "CmdTimeStamp", LR"(show unix timestamp)" , LR"(.ts 0n22222/431203BC [show timestamp])" },

{ L".err", "CmdErrorShow", LR"(show LastError or NTSTATUS)" ,
LR"(.err 2 [show LastError]
.err -s c0000034 [show NTSTATUS])" },

{ L".fmt", "CmdFormats", LR"(show value formats, default radix is 16(hex))" ,
LR"(.fmt 0x400/0n1024 [Hex(0x),Dec(0n),Oct(0t),Bin(0y)])" },

{ L".msg", "CmdMsg", LR"(show window message id)", 
LR"(.msg [show message id list]
.msg -name button [show message name matched *button*, eg:WM_LBUTTONDOWN...]
.msg -id 201/0x201/0n513 [implies WM_LBUTTONDOWN])" },

{ L".wnd", "CmdWndInfo", LR"(show window information)",
LR"(.wnd [show system window list]
.wnd -title Worker [show window title name matched *Worker*]
.wnd -class Worker [show window class name matched *Worker*]
.wnd -hwnd 778BC/0x778BC/0n489660 [show HWND=778BC window information]
.wnd -pid 8880 [show process pid=8880 window list]
out: [hwnd] [parent hwnd] [title] [class] [pid.tid] [process name])" },

{ L".ps", "CmdProcessInfo", LR"(show process information)",
LR"(.ps [show processes list]
.ps -name lsass [find process name matched *lsass*]
.ps -pid 1234,2048 [find process pid=1234/2048]
.ps -path \windows [find process path matched *\windows*]
.ps -mods 1234 [show module list that process pid=1234]
.ps -kill -name chrome [kill process name matched *chrome* , be careful!!!]
.ps -kill -pid 1234,2048 [kill process pid=1234 or 2048]
.ps -kill -path \temp\ [kill process path matched *\temp\* , be careful!!!]
.ps -inject 1234 c:\hook.dll [inject hook.dll into pid=1234])" },

{ L".mm", "CmdMemoryInfo", LR"(show memory information)",
LR"(.mm [show os memory]
.mm -pid 1234 [show process pid=1234 memory information])" },
};

Cmds::Cmds(QTextBrowser *parent) :
	cmd_window_(parent),
	cmd_cursor_(0)
{
	for (auto &item : CmdTable) {
		item.doc = LR"(<b>)" + item.doc + LR"(</b>)";
		auto tail = LR"(<br>----------------------------------------------------------------------------------------------------------------------------------------------------------------)";
		if (item.example.empty())
			item.doc.append(tail);
		else	
			item.example.append(tail);
	}
}

Cmds::~Cmds()
{
}

Q_INVOKABLE void Cmds::CmdHelp(QStringList argv)
{
#define SHOW_HELP() \
	if (!item.example.empty()) CmdOutput(L"%s - %s\n%s", item.cmd.c_str(), item.doc.c_str(), item.example.c_str()); \
	else CmdOutput(L"%s - %s", item.cmd.c_str(), item.doc.c_str());

	auto argc = argv.size();
	if (argc == 0) {
		for (auto &item : CmdTable) {
			SHOW_HELP();
		}
	}
	if (argc == 1) {
		auto wstr = argv[0].toStdWString();
		for (auto &item : CmdTable) {
			if (UNONE::StrContainIW(item.cmd, wstr))
				SHOW_HELP();
		}
	}
}

Q_INVOKABLE void Cmds::CmdCls(QStringList argv)
{
	cmd_window_->clear();
}

Q_INVOKABLE void Cmds::CmdCmd(QStringList argv)
{
	std::wstring line;
	for (size_t i = 0; i < argv.size(); i++) {
		line.append(argv[i].toStdWString());
		if (i != (argv.size()-1))
			line.append(L" ");
	}
	auto cmd = L"cmd.exe /c " + line;
	std::string out;
	DWORD code;
	ReadConsoleOutput(UNONE::StrToA(cmd), out, code);
	out = UNONE::StrTrimA(out);
	UNONE::StrReplaceA(out, "\r");
	//UNONE::StrReplaceA(out, "\n", "</br>");
	std::wstring xxx = UNONE::StrToW(out);
	CmdOutput(L"%s", xxx.c_str());
}

Q_INVOKABLE void Cmds::CmdStart(QStringList argv)
{
	std::wstring line;
	for (size_t i = 0; i < argv.size(); i++) {
		line.append(argv[i].toStdWString());
		if (i != (argv.size() - 1))
			line.append(L" ");
	}
	UNONE::PsCreateProcessW(line);
}

Q_INVOKABLE void Cmds::CmdMsg(QStringList argv)
{
	if (MessageMapTable.empty()) {
		std::vector<std::string> msgs;
		UNONE::StrSplitA(MessageRawString, ",", msgs);
		for (auto& m : msgs) {
			std::vector<std::string> kv;
			UNONE::StrSplitA(m, "=", kv);
			std::string name = kv[0];
			int id = UNONE::StrToHex64A(kv[1]);
			MessageMapTable[id] = name;
		}
	}

	auto argc = argv.size();
	if (argc == 0) {
		for (auto kv : MessageMapTable) {
			CmdOutput("0x%04X (%d)  %s", kv.first, kv.first, kv.second.c_str());
		}
		return;
	}

	if (argc == 2) {
		auto str = argv[1].toStdString();
		if (argv[0] == "-name") {
			for (auto kv : MessageMapTable) {
				if (UNONE::StrContainIA(kv.second, str)) {
					CmdOutput("0x%04X (%d)  %s", kv.first, kv.first, kv.second.c_str());
				}
			}
			return;
		}
		if (argv[0] == "-id") {
			for (auto kv : MessageMapTable) {
				if (kv.first == VariantInt(str)) {
					CmdOutput("0x%04X (%d)  %s", kv.first, kv.first, kv.second.c_str());
				}
			}
			return;
		}
	}

	CmdException(ECMD_PARAM_INVALID);
}

Q_INVOKABLE void Cmds::CmdWndInfo(QStringList argv)
{
	auto OutputWndInfo = [&](HWND w) {
		DWORD pid, tid;
		tid = GetWindowThreadProcessId(w, &pid);
		auto pname = UNONE::PsGetProcessNameW(pid);
		auto title = UNONE::PsGetWndTextW(w);
		auto clsname = UNONE::PsGetWndClassNameW(w);
		auto parent = GetParent(w);
		CmdOutput(L"%08X %08X [%s] [%s] [%04X.%04X] %s", w, parent, title.c_str(), clsname.c_str(), pid,tid, pname.c_str());
	};

	auto argc = argv.size();
	if (argc == 0) {
		for (auto w : GetSystemWnds()) {	OutputWndInfo(w);}
		return;
	}
	if (argc == 2) {
		auto wstr = argv[1].toStdWString();
		if (argv[0] == "-title") {
			for (auto w : GetSystemWnds()) {
				auto title = UNONE::PsGetWndTextW(w);
				if (!title.empty() && UNONE::StrContainIW(title, wstr)) {
					OutputWndInfo(w);
				}
			}
			return;
		}
		if (argv[0] == "-class") {
			for (auto w : GetSystemWnds()) {
				auto title = UNONE::PsGetWndClassNameW(w);
				if (!title.empty() && UNONE::StrContainIW(title, wstr)) {
					OutputWndInfo(w);
				}
			}
			return;
		}
		if (argv[0] == "-hwnd") {
			OutputWndInfo((HWND)VariantInt64(UNONE::StrToA(wstr)));
			return;
		}
		if (argv[0] == "-pid") {
			auto wnds = UNONE::PsGetWnds(VariantInt(UNONE::StrToA(wstr)));
			for (auto w : wnds) {
				OutputWndInfo(w);
			}
			return;
		}
	}
	CmdException(ECMD_PARAM_INVALID);
}

Q_INVOKABLE void Cmds::CmdHistory(QStringList argv)
{
	int i = 0;
	for (auto& m : cmd_history_) {
		QString line = QString("%1 %2").arg(i).arg(m);
		if (i == cmd_cursor_) {
			line.insert(0, "* ");
		}
		CmdOutput(line.toStdWString().c_str());
		i++;
	}
}

Q_INVOKABLE void Cmds::CmdTimeStamp(QStringList argv)
{
	auto paramcnt = argv.size();
	if (paramcnt == 0) {
		CmdOutput("%lld", (DWORD64)time(0));
		return;
	}
	if (paramcnt == 1) {
		CmdOutput("%s", UNONE::TmFormatUnixTimeA(VariantInt(argv[0].toStdString()), "Y-M-D H:W:S").c_str());
		return;
	}
	CmdException(ECMD_PARAM_INVALID);
}

Q_INVOKABLE void Cmds::CmdErrorShow(QStringList argv)
{
	auto OutErrorMsg = [&](DWORD err) {
		auto msg = UNONE::OsDosErrorMsgW(err);
		if (msg.empty()) {
			CmdOutput(L"%d is invalid error value", err);
		} else {
			CmdOutput(L"%s", msg.c_str());
		}
	};

	auto argc = argv.size();
	if (argc == 1) {
		int64_t val = VariantInt64(argv[0].toStdString());
		OutErrorMsg(val);
		return;
	}
	if (argc == 2) {
		if (argv[0] == "-s") {
			int64_t val = VariantInt64(argv[1].toStdString());
			OutErrorMsg(UNONE::OsNtToDosError(val));
			return;
		}
	}
	CmdException(ECMD_PARAM_INVALID);
}

Q_INVOKABLE void Cmds::CmdFormats(QStringList argv)
{
	auto DecToBin = [](int64_t val)->std::string {
		char bins[128] = { 0 };
		itoa(val, bins, 2);
		std::string str(bins);
		int v = 8;
		str = UNONE::StrRepeatA("0", (v - str.size() % v)) + str;
		str = UNONE::StrInsertA(str, v, " ");
		return str;
	};
	if (argv.size() == 1) {
		int64_t val = VariantInt64(argv[0].toStdString());
		CmdOutput("HEX: %X", val);
		CmdOutput("DEC: %d", val);
		CmdOutput("OCT: %o", val);
		CmdOutput("BIN: %s", DecToBin(val).c_str());
		auto str = UNONE::StrHexStrToStreamA(UNONE::StrFormatA("%llx", val));
		for (auto &c : str) { if (c>0 && c<0xff && !isprint(c)) c = '.'; }
		CmdOutput("STR: %s", str.c_str());
		return;
	}
	CmdException(ECMD_PARAM_INVALID);
}

Q_INVOKABLE void Cmds::CmdExit(QStringList argv)
{
	ExitProcess(0);
}

Q_INVOKABLE void Cmds::CmdRestart(QStringList argv)
{
	UNONE::PsCreateProcessW(UNONE::PsGetProcessPathW());
	ExitProcess(0);
}

Q_INVOKABLE void Cmds::CmdProcessInfo(QStringList argv)
{
	auto OuputProcessInfo = [&](std::vector<DWORD> pids) {
		std::wstring out;
		out = UNONE::StrFormatW(LR"(<table border="1"><tr><td>PID</td><td>PPID</td><td>ProcessName</td><td>CreateTime</td><td>ProcessPath</td><td>CompanyName</td></tr>)");
		for (auto pid : pids) {
			if (!UNONE::PsIsPidExisted(pid)) continue;
			auto ppid = UNONE::PsGetParentPid(pid);
			auto name = UNONE::PsGetProcessNameW(pid);
			auto path = UNONE::PsGetProcessPathW(pid);
			auto time = ProcessCreateTime(pid);
			std::wstring corp;
			UNONE::FsGetFileInfoW(path, L"CompanyName", corp);
			out.append(UNONE::StrFormatW(L"<tr><td>%d</td><td>%d</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td></tr>", pid, ppid, name.c_str(), time.c_str(), path.c_str(), corp.c_str()));
		}
		out.append(UNONE::StrFormatW(LR"(</table>)"));
		CmdOutput(out.c_str());
	};

	auto OuputModulesInfo = [&](DWORD pid) {
		std::wstring out;
		out = UNONE::StrFormatW(LR"(<table border="1"><tr><td>ModuleName</td><td></td><td>ModuleBase</td><td>ModuleSize</td><td>CompanyName</td></tr>)");
		std::vector<UNONE::MODULE_BASE_INFOW> infos;
		UNONE::PsGetModulesInfoW(pid, infos);
		for (auto info : infos) {
			auto name = info.BaseDllName;
			auto path = info.FullDllName;
			auto base = info.DllBase;
			auto size = info.SizeOfImage;
			std::wstring corp;
			UNONE::FsGetFileInfoW(path, L"CompanyName", corp);
			out.append(UNONE::StrFormatW(L"<tr><td>%s</td><td>0x%X</td><td>0x%X</td><td>%s</td><td>%s</td></tr>", name.c_str(), base, size, path.c_str(), corp.c_str()));
		}
		out.append(UNONE::StrFormatW(LR"(</table>)"));
		CmdOutput(out.c_str());
	};

	auto ParsePids = [&](std::wstring wstr, std::vector<DWORD> &pids) {
		std::vector<std::wstring> wvec;
		UNONE::StrSplitW(wstr, L",", wvec);
		for (auto s : wvec) {
			pids.push_back(VariantInt(UNONE::StrToA(s)));
		}
	};

	auto FindPidsByName = [&](std::wstring& pattern, std::vector<DWORD> &pids) {
		UNONE::PsEnumProcess([&](PROCESSENTRY32W &entry)->bool {
			auto pid = entry.th32ProcessID;
			if (UNONE::StrContainIW(UNONE::PsGetProcessNameW(pid), pattern)) {
				pids.push_back(pid);
			}
			return true;
		});
	};

	auto FindPidsByPath = [&](std::wstring& pattern, std::vector<DWORD> &pids) {
		UNONE::PsEnumProcess([&](PROCESSENTRY32W &entry)->bool {
			auto pid = entry.th32ProcessID;
			if (UNONE::StrContainIW(UNONE::PsGetProcessPathW(pid), pattern)) {
				pids.push_back(pid);
			}
			return true;
		});
	};

	auto KillPids = [&](std::vector<DWORD> &pids) {
		for (auto pid : pids) {
			auto name = UNONE::PsGetProcessNameW(pid);
			auto path = UNONE::PsGetProcessPathW(pid);
			bool killed = UNONE::PsKillProcess(pid);
			if (killed) {
				CmdOutput(L"[+] kill pid:%d name:%s path:%s ok", pid, name.c_str(), path.c_str());
			} else {
				CmdOutput(L"[-] kill pid:%d name:%s path:%s err", pid, name.c_str(), path.c_str());
			}
		}
	};

	auto argc = argv.size();

	if (argc == 0) {
		std::vector<DWORD> pids;
		UNONE::PsGetAllProcess(pids);
		OuputProcessInfo(pids);
		return;
	}

	if (argc == 2) {
		std::wstring wstr = argv[1].toStdWString();
		std::vector<DWORD> pids;
		if (argv[0] == "-name") {
			FindPidsByName(wstr, pids);
			OuputProcessInfo(pids);
			return;
		}
		if (argv[0] == "-pid") {
			ParsePids(wstr, pids);
			OuputProcessInfo(pids);
			return;
		}
		if (argv[0] == "-path") {
			FindPidsByPath(wstr, pids);
			OuputProcessInfo(pids);
			return;
		}
		if (argv[0] == "-mods") {
			OuputModulesInfo(VariantInt(UNONE::StrToA(wstr)));
			return;
		}
	}

	if (argc == 3) {
		if (argv[0] == "-kill") {
			std::wstring wstr = argv[2].toStdWString();
			std::vector<DWORD> pids;
			if (argv[1] == "-name") {
				FindPidsByName(wstr, pids);
				KillPids(pids);
				return;
			}
			if (argv[1] == "-pid") {
				ParsePids(wstr, pids);
				KillPids(pids);
				return;
			}
			if (argv[1] == "-path") {
				FindPidsByPath(wstr, pids);
				KillPids(pids);
				return;
			}
		}
		if (argv[0] == "-inject") {
			DWORD pid = VariantInt(argv[1].toStdString());
			auto path = argv[2].toStdWString();
			auto thd = UNONE::PsInjectByRemoteThreadW(pid, path);
			if (thd) {
				CmdOutput(L"[+] inject pid:%d path:%s ok", pid, path.c_str());
			} else {
				CmdOutput(L"[-] inject pid:%d path:%s err", pid, path.c_str());
			}
			CloseHandle(thd);
			return;
		}
	}

	CmdException(ECMD_PARAM_INVALID);
}

Q_INVOKABLE void Cmds::CmdMemoryInfo(QStringList argv)
{
	SIZE_T PageSize;
	auto OutMemoryInfoStyle1 = [&](wchar_t* name, SIZE_T size) {
		double mb = (double)(size*PageSize) / 1024 / 1024;
		double gb = (double)(size*PageSize) / 1024 / 1024 / 1024;
		CmdOutput(L"%s : %0.2f GB (%0.2f MB)", name, gb, mb);
	};
	auto OutMemoryInfoStyle2 = [&](wchar_t* name, SIZE_T size) {
		double mb = (double) size / 1024 / 1024;
		CmdOutput(L"%s : %0.2f MB", name, mb);
	};
	auto argc = argv.size();
	if (argc == 0) {
		PERFORMANCE_INFORMATION perf = { 0 };
		GetPerformanceInfo(&perf, sizeof(perf));
		PageSize = perf.PageSize;
		double usage = GetSystemUsageOfMemory();
		CmdOutput(L"%s : %d B", L"PageSize", PageSize);
		CmdOutput(L"%s : %0.2f%%", L"Memory Usage", usage);
		CmdOutput(L"-------------------------------------------");
		OutMemoryInfoStyle1(L"CommitTotal", perf.CommitTotal);
		OutMemoryInfoStyle1(L"CommitLimit", perf.CommitLimit);
		OutMemoryInfoStyle1(L"CommitPeak", perf.CommitPeak);
		CmdOutput(L"-------------------------------------------");
		OutMemoryInfoStyle1(L"PhysicalTotal", perf.PhysicalTotal);
		OutMemoryInfoStyle1(L"PhysicalAvailable", perf.PhysicalAvailable);
		CmdOutput(L"-------------------------------------------");
		OutMemoryInfoStyle1(L"KernelTotal", perf.KernelTotal);
		OutMemoryInfoStyle1(L"KernelPaged", perf.KernelPaged);
		OutMemoryInfoStyle1(L"KernelNonpaged", perf.KernelNonpaged);
		OutMemoryInfoStyle1(L"SystemCache", perf.SystemCache);
		return;
	}
	if (argc == 2) {
		if (argv[0] == "-pid") {
			DWORD pid = VariantInt(argv[1].toStdString());
			PROCESS_MEMORY_COUNTERS_EX mm_info;
			if (!UNONE::MmGetProcessMemoryInfo(pid, mm_info))
				return;
			OutMemoryInfoStyle2(L"Working set", mm_info.WorkingSetSize);
			OutMemoryInfoStyle2(L"WS Private", mm_info.WorkingSetSize - GetProcessPrivateWorkingSet(pid));
			OutMemoryInfoStyle2(L"Private", mm_info.PrivateUsage);
			OutMemoryInfoStyle2(L"PeakWorkingSet", mm_info.PeakWorkingSetSize);
			CmdOutput(L"%s : %d", L"PageFaultCount", mm_info.PageFaultCount);
			CmdOutput(L"-------------------------------------------");
			OutMemoryInfoStyle2(L"QuotaPagedPoolUsage", mm_info.QuotaPagedPoolUsage);
			OutMemoryInfoStyle2(L"QuotaNonPagedPoolUsage", mm_info.QuotaNonPagedPoolUsage);
			OutMemoryInfoStyle2(L"QuotaPeakPagedPoolUsage", mm_info.QuotaPeakPagedPoolUsage);
			OutMemoryInfoStyle2(L"QuotaPeakNonPagedPoolUsage", mm_info.QuotaPeakNonPagedPoolUsage);
			CmdOutput(L"-------------------------------------------");
			OutMemoryInfoStyle2(L"PagefileUsage", mm_info.PagefileUsage);
			OutMemoryInfoStyle2(L"PeakPagefileUsage", mm_info.PeakPagefileUsage);
			return;
		}
	}

	CmdException(ECMD_PARAM_INVALID);
}

QString Cmds::CmdGetLast()
{
	QString cmdline;
	if (cmd_cursor_ > 0) {
		cmd_cursor_--;
		cmdline = cmd_history_[cmd_cursor_];
	}
	return cmdline;
}

QString Cmds::CmdGetNext()
{
	QString cmdline;
	if (cmd_cursor_ < cmd_history_.size()) {
		cmdline = cmd_history_[cmd_cursor_];
		cmd_cursor_++;
	}
	return cmdline;
}

void Cmds::CmdException(int type)
{
	switch (type) {
	case ECMD_PARAM_INVALID:
		CmdErrorOutput(L"[-] Command parameters invalid. (.help to show help-doc)");
		break;
	case ECMD_NOSUCH_CMD:
		CmdErrorOutput(L"[-] No such command. (.help to show help-doc)");
		break;
	default:
		CmdErrorOutput(L"[-] Unknown exception.");
		break;
	}
}

void Cmds::CmdErrorOutput(const std::wstring &err)
{
	CmdOutput(L"<font color=red>%s</font>", err.c_str());
}

void Cmds::CmdOutput(const char* format, ...)
{
	va_list lst;
	va_start(lst, format);
	std::wstring &&wstr = UNONE::StrToW(UNONE::StrFormatVaListA(format, lst));
	CmdOutput(L"%s", wstr.c_str());
	va_end(lst);
}

void Cmds::CmdOutput(const wchar_t* format, ...)
{
	QString log;
	va_list lst;
	va_start(lst, format);
	std::wstring &&wstr = UNONE::StrFormatVaListW(format, lst);
	log = QString().fromStdWString(wstr);
	va_end(lst);
	log.append("\n");
	log.replace("\n", "<br/>");
	log.replace("ERROR", "<font color=red>ERROR</font>");
	log.replace("err", "<font color=red>err</font>");
	log.replace("failed", "<font color=red>failed</font>");
	log.replace("fail", "<font color=red>fail</font>");
	log.replace("warning", "<font color=red>warning</font>");
	log.replace("WRAN", "<font color=red>WRAN</font>");
	log.replace("FATAL", "<font color=red>FATAL</font>");
	cmd_window_->append(log);
}

void Cmds::CmdDispatcher(const std::wstring &cmdline)
{
	UNONE::InterUnregisterLogger();

	std::wstring wstr = UNONE::StrTrimW(cmdline);
	
	//if (!cmd_window_->toPlainText().isEmpty()) CmdOutput(LR"(<hr>)");
	CmdOutput(LR"(<b><font color="black">C:\>%s</font></b>)", cmdline.c_str());

	if (cmd_history_.empty() || QString::compare(WStrToQ(wstr), cmd_history_.back(), Qt::CaseInsensitive)!=0) {
		cmd_history_.push_back(WStrToQ(wstr));
	}
	cmd_cursor_ = cmd_history_.size();

	std::wstring cmd;
	std::vector<std::wstring> param;
	std::vector<std::wstring> vec;
	if (UNONE::StrContainW(wstr, L" ")) {
		UNONE::StrSplitW(wstr, L" ", vec);
		for (auto v : vec) {
			if (!v.empty()) param.push_back(v);
		}
		cmd = param[0];
		param.erase(param.begin());
	}	else {
		cmd = wstr;
	}

	QStringList pramlst = WVectorToQList(param);
	
	for (auto& item : CmdTable) {
		if (UNONE::StrCompareIW(item.cmd, cmd)) {
			UNONE::InterRegisterLogger([&](const std::wstring &log) { 
				CmdOutput(log.c_str());
			});
			QMetaObject::invokeMethod(this, item.func.c_str(), Qt::QueuedConnection, Q_ARG(QStringList, pramlst));
			return;
		}
	}
	CmdException(ECMD_NOSUCH_CMD);
}

int Cmds::VariantInt(std::string val)
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
	return UNONE::StrToHexA(val);
}

int64_t Cmds::VariantInt64(std::string val)
{
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
	return UNONE::StrToHex64A(val);
}