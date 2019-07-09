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
#include "constants.h"
#include "../../common/common.h"

//Win7
std::map<int, std::string> ObjectTypeTable;// = { { 2,"Type" },{ 3,"Directory" },{ 4,"SymbolicLink" },{ 5,"Token" },{ 6,"Job" },{ 7,"Process" },{ 8,"Thread" },{ 9,"UserApcReserve" },{ 10,"IoCompletionReserve" },{ 11,"DebugObject" },{ 12,"Event" },{ 13,"EventPair" },{ 14,"Mutant" },{ 15,"Callback" },{ 16,"Semaphore" },{ 17,"Timer" },{ 18,"Profile" },{ 19,"KeyedEvent" },{ 20,"WindowStation" },{ 21,"Desktop" },{ 22,"TpWorkerFactory" },{ 23,"Adapter" },{ 24,"Controller" },{ 25,"Device" },{ 26,"Driver" },{ 27,"IoCompletion" },{ 28,"File" },{ 29,"TmTm" },{ 30,"TmTx" },{ 31,"TmRm" },{ 32,"TmEn" },{ 33,"Section" },{ 34,"Session" },{ 35,"Key" },{ 36,"ALPC Port" },{ 37,"PowerRequest" },{ 38,"WmiGuid" },{ 39,"EtwRegistration" },{ 40,"EtwConsumer" },{ 41,"FilterConnectionPort" },{ 42,"FilterCommunicationPort" },{ 43,"PcwObject" } };
char *MessageRawString = "WM_NULL=0x0000,WM_CREATE=0x0001,WM_DESTROY=0x0002,WM_MOVE=0x0003,WM_SIZE=0x0005,WM_ACTIVATE=0x0006,WM_SETFOCUS=0x0007,WM_KILLFOCUS=0x0008,WM_ENABLE=0x000A,WM_SETREDRAW=0x000B,WM_SETTEXT=0x000C,WM_GETTEXT=0x000D,WM_GETTEXTLENGTH=0x000E,WM_PAINT=0x000F,WM_CLOSE=0x0010,WM_QUERYENDSESSION=0x0011,WM_QUIT=0x0012,WM_QUERYOPEN=0x0013,WM_ERASEBKGND=0x0014,WM_SYSCOLORCHANGE=0x0015,WM_ENDSESSION=0x0016,WM_SHOWWINDOW=0x0018,WM_WININICHANGE=0x001A,WM_SETTINGCHANGE=0x001A,WM_DEVMODECHANGE=0x001B,WM_ACTIVATEAPP=0x001C,WM_FONTCHANGE=0x001D,WM_TIMECHANGE=0x001E,WM_CANCELMODE=0x001F,WM_SETCURSOR=0x0020,WM_MOUSEACTIVATE=0x0021,WM_CHILDACTIVATE=0x0022,WM_QUEUESYNC=0x0023,WM_GETMINMAXINFO=0x0024,WM_PAINTICON=0x0026,WM_ICONERASEBKGND=0x0027,WM_NEXTDLGCTL=0x0028,WM_SPOOLERSTATUS=0x002A,WM_DRAWITEM=0x002B,WM_MEASUREITEM=0x002C,WM_DELETEITEM=0x002D,WM_VKEYTOITEM=0x002E,WM_CHARTOITEM=0x002F,WM_SETFONT=0x0030,WM_GETFONT=0x0031,WM_SETHOTKEY=0x0032,WM_GETHOTKEY=0x0033,WM_QUERYDRAGICON=0x0037,WM_COMPAREITEM=0x0039,WM_GETOBJECT=0x003D,WM_COMPACTING=0x0041,WM_COMMNOTIFY=0x0044,WM_WINDOWPOSCHANGING=0x0046,WM_WINDOWPOSCHANGED=0x0047,WM_POWER=0x0048,WM_COPYDATA=0x004A,WM_CANCELJOURNAL=0x004B,WM_NOTIFY=0x004E,WM_INPUTLANGCHANGEREQUEST=0x0050,WM_INPUTLANGCHANGE=0x0051,WM_TCARD=0x0052,WM_HELP=0x0053,WM_USERCHANGED=0x0054,WM_NOTIFYFORMAT=0x0055,WM_CONTEXTMENU=0x007B,WM_STYLECHANGING=0x007C,WM_STYLECHANGED=0x007D,WM_DISPLAYCHANGE=0x007E,WM_GETICON=0x007F,WM_SETICON=0x0080,WM_NCCREATE=0x0081,WM_NCDESTROY=0x0082,WM_NCCALCSIZE=0x0083,WM_NCHITTEST=0x0084,WM_NCPAINT=0x0085,WM_NCACTIVATE=0x0086,WM_GETDLGCODE=0x0087,WM_SYNCPAINT=0x0088,WM_NCMOUSEMOVE=0x00A0,WM_NCLBUTTONDOWN=0x00A1,WM_NCLBUTTONUP=0x00A2,WM_NCLBUTTONDBLCLK=0x00A3,WM_NCRBUTTONDOWN=0x00A4,WM_NCRBUTTONUP=0x00A5,WM_NCRBUTTONDBLCLK=0x00A6,WM_NCMBUTTONDOWN=0x00A7,WM_NCMBUTTONUP=0x00A8,WM_NCMBUTTONDBLCLK=0x00A9,WM_KEYDOWN=0x0100,WM_KEYUP=0x0101,WM_CHAR=0x0102,WM_DEADCHAR=0x0103,WM_SYSKEYDOWN=0x0104,WM_SYSKEYUP=0x0105,WM_SYSCHAR=0x0106,WM_SYSDEADCHAR=0x0107,WM_KEYLAST=0x0108,WM_IME_STARTCOMPOSITION=0x010D,WM_IME_ENDCOMPOSITION=0x010E,WM_IME_COMPOSITION=0x010F,WM_IME_KEYLAST=0x010F,WM_INITDIALOG=0x0110,WM_COMMAND=0x0111,WM_SYSCOMMAND=0x0112,WM_TIMER=0x0113,WM_HSCROLL=0x0114,WM_VSCROLL=0x0115,WM_INITMENU=0x0116,WM_INITMENUPOPUP=0x0117,WM_MENUSELECT=0x011F,WM_MENUCHAR=0x0120,WM_ENTERIDLE=0x0121,WM_MENURBUTTONUP=0x0122,WM_MENUDRAG=0x0123,WM_MENUGETOBJECT=0x0124,WM_UNINITMENUPOPUP=0x0125,WM_MENUCOMMAND=0x0126,WM_CTLCOLORMSGBOX=0x0132,WM_CTLCOLOREDIT=0x0133,WM_CTLCOLORLISTBOX=0x0134,WM_CTLCOLORBTN=0x0135,WM_CTLCOLORDLG=0x0136,WM_CTLCOLORSCROLLBAR=0x0137,WM_CTLCOLORSTATIC=0x0138,WM_MOUSEMOVE=0x0200,WM_LBUTTONDOWN=0x0201,WM_LBUTTONUP=0x0202,WM_LBUTTONDBLCLK=0x0203,WM_RBUTTONDOWN=0x0204,WM_RBUTTONUP=0x0205,WM_RBUTTONDBLCLK=0x0206,WM_MBUTTONDOWN=0x0207,WM_MBUTTONUP=0x0208,WM_MBUTTONDBLCLK=0x0209,WM_MOUSEWHEEL=0x020A,WM_PARENTNOTIFY=0x0210,WM_ENTERMENULOOP=0x0211,WM_EXITMENULOOP=0x0212,WM_NEXTMENU=0x0213,WM_SIZING=0x0214,WM_CAPTURECHANGED=0x0215,WM_MOVING=0x0216,WM_DEVICECHANGE=0x0219,WM_MDICREATE=0x0220,WM_MDIDESTROY=0x0221,WM_MDIACTIVATE=0x0222,WM_MDIRESTORE=0x0223,WM_MDINEXT=0x0224,WM_MDIMAXIMIZE=0x0225,WM_MDITILE=0x0226,WM_MDICASCADE=0x0227,WM_MDIICONARRANGE=0x0228,WM_MDIGETACTIVE=0x0229,WM_MDISETMENU=0x0230,WM_ENTERSIZEMOVE=0x0231,WM_EXITSIZEMOVE=0x0232,WM_DROPFILES=0x0233,WM_MDIREFRESHMENU=0x0234,WM_IME_SETCONTEXT=0x0281,WM_IME_NOTIFY=0x0282,WM_IME_CONTROL=0x0283,WM_IME_COMPOSITIONFULL=0x0284,WM_IME_SELECT=0x0285,WM_IME_CHAR=0x0286,WM_IME_REQUEST=0x0288,WM_IME_KEYDOWN=0x0290,WM_IME_KEYUP=0x0291,WM_MOUSEHOVER=0x02A1,WM_MOUSELEAVE=0x02A3,WM_CUT=0x0300,WM_COPY=0x0301,WM_PASTE=0x0302,WM_CLEAR=0x0303,WM_UNDO=0x0304,WM_RENDERFORMAT=0x0305,WM_RENDERALLFORMATS=0x0306,WM_DESTROYCLIPBOARD=0x0307,WM_DRAWCLIPBOARD=0x0308,WM_PAINTCLIPBOARD=0x0309,WM_VSCROLLCLIPBOARD=0x030A,WM_SIZECLIPBOARD=0x030B,WM_ASKCBFORMATNAME=0x030C,WM_CHANGECBCHAIN=0x030D,WM_HSCROLLCLIPBOARD=0x030E,WM_QUERYNEWPALETTE=0x030F,WM_PALETTEISCHANGING=0x0310,WM_PALETTECHANGED=0x0311,WM_HOTKEY=0x0312,WM_PRINT=0x0317,WM_PRINTCLIENT=0x0318,WM_HANDHELDFIRST=0x0358,WM_HANDHELDLAST=0x035F,WM_AFXFIRST=0x0360,WM_AFXLAST=0x037F,WM_PENWINFIRST=0x0380,WM_PENWINLAST=0x038F,WM_APP=0x8000,WM_USER=0x0400";
std::map<int, std::string> MessageMapTable;

// Dynamic retrieve ObjectIndex ID
void InitObjectTypeTable()
{
	if (!ObjectTypeTable.empty()) return;
	ObjectTypeTable[2] = "Type";
	ObjectTypeTable[3] = "Directory";
	ObjectTypeTable[4] = "SymbolicLink";
	ObjectTypeTable[5] = "Token";
	ObjectTypeTable[6] = "Job";
	ObjectTypeTable[7] = "Process";
	ObjectTypeTable[8] = "Thread";

	auto file = CreateFileW(UNONE::PsGetProcessPathW().c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	auto iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	auto desktop = GetThreadDesktop(GetCurrentThreadId());
	auto event = CreateEventW(NULL, FALSE, FALSE, NULL);
	auto mutant = CreateMutexW(NULL, FALSE, NULL);
	auto semaphore = CreateSemaphoreW(NULL, 1, 1, NULL);
	auto section = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READONLY, 0, 0x1000, NULL);
	auto station = GetProcessWindowStation();
	HKEY key;
	RegOpenKeyW(HKEY_LOCAL_MACHINE, L"SYSTEM", &key);

	typedef	NTSTATUS (NTAPI *__NtCreateKeyedEvent)(
		OUT PHANDLE             KeyedEventHandle,
		IN ACCESS_MASK          DesiredAccess,
		IN POBJECT_ATTRIBUTES   ObjectAttributes OPTIONAL,
		IN ULONG                Reserved);
	auto pNtCreateKeyedEvent = (__NtCreateKeyedEvent)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtCreateKeyedEvent");
	HANDLE keyed_event;
	if (pNtCreateKeyedEvent)	pNtCreateKeyedEvent(&keyed_event, EVENT_ALL_ACCESS, NULL, 0);

	typedef NTSTATUS(NTAPI *__NtCreatePort)(
		OUT PHANDLE             PortHandle,
		IN POBJECT_ATTRIBUTES   ObjectAttributes,
		IN ULONG                MaxConnectInfoLength,
		IN ULONG                MaxDataLength,
		IN OUT PULONG           Reserved OPTIONAL);
	auto pNtCreatePort = (__NtCreatePort)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtCreatePort");
	HANDLE lpc;
	if (pNtCreatePort) {
		OBJECT_ATTRIBUTES oa;
		UNICODE_STRING port_name;
		port_name.Buffer = L"\\0xbaddbadd";
		port_name.Length = sizeof(L"\\0xbaddbadd");
		port_name.MaximumLength = port_name.Length;
		InitializeObjectAttributes(&oa, &port_name, 0, NULL, NULL);
		pNtCreatePort(&lpc, &oa, 0, 0, NULL);
	}

	typedef NTSTATUS(NTAPI *__NtCreateTimer)(
		OUT PHANDLE             TimerHandle,
		IN ACCESS_MASK          DesiredAccess,
		IN POBJECT_ATTRIBUTES   ObjectAttributes,
		IN int           TimerType);
	auto pNtCreateTimer = (__NtCreateTimer)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtCreateTimer");
	HANDLE timer;
	if (pNtCreateTimer)
		pNtCreateTimer(&timer, TIMER_ALL_ACCESS, NULL, 0);

	UNONE::PsEnumHandle(GetCurrentProcessId(), [&](SYSTEM_HANDLE_TABLE_ENTRY_INFO &info)->bool {
		HANDLE hd = (HANDLE)info.HandleValue;
		if (hd == file) {
			ObjectTypeTable[info.ObjectTypeIndex] = "File";
		} else if (hd == iocp) {
			ObjectTypeTable[info.ObjectTypeIndex] = "IoCompletion";
		} else if (hd == desktop) {
			ObjectTypeTable[info.ObjectTypeIndex] = "Desktop";
		} else if (hd == event) {
			ObjectTypeTable[info.ObjectTypeIndex] = "Event";
		} else if (hd == mutant) {
			ObjectTypeTable[info.ObjectTypeIndex] = "Mutant";
		} else if (hd == semaphore) {
			ObjectTypeTable[info.ObjectTypeIndex] = "Semaphore";
		} else if (hd == section) {
			ObjectTypeTable[info.ObjectTypeIndex] = "Section";
		} else if (hd == station) {
			ObjectTypeTable[info.ObjectTypeIndex] = "WindowStation";
		} else if (hd == key) {
			ObjectTypeTable[info.ObjectTypeIndex] = "Key";
		} else if (hd == keyed_event) {
			ObjectTypeTable[info.ObjectTypeIndex] = "KeyedEvent";
		} else if (hd == lpc) {
			if (UNONE::OsMajorVer() >= 6)
				ObjectTypeTable[info.ObjectTypeIndex] = "ALPC Port";
			else
				ObjectTypeTable[info.ObjectTypeIndex] = "LPC Port";
		} else if (hd == timer) {
			ObjectTypeTable[info.ObjectTypeIndex] = "Timer";
		} 
		return true;
	});

	CloseHandle(file);
	CloseHandle(iocp);
	CloseHandle(event);
	CloseHandle(mutant);
	CloseHandle(semaphore);
	CloseHandle(section);
	RegCloseKey(key);
	CloseHandle(lpc);
	CloseHandle(timer);
}

int GetObjectTypeIndex(char* name)
{
	for (auto kv : ObjectTypeTable) {
		if (kv.second == "File") return kv.first;
	}
	return 0;
}

std::string MbiTypeToString(int type)
{
	std::string str;
	if (type & MEM_PRIVATE) str += "MEM_PRIVATE | ";
	if (type & MEM_IMAGE) str += "MEM_IMAGE | ";
	if (type & MEM_MAPPED) str += "MEM_MAPPED | ";
	return str.empty() ? "" : str.substr(0, str.size() - 3);
}
std::string MbiStateToString(int type)
{
	std::string str;
	if (type & MEM_COMMIT) str += "MEM_COMMIT | ";
	if (type & MEM_FREE) str += "MEM_FREE | ";
	if (type & MEM_RESERVE) str += "MEM_RESERVE | ";
	return str.empty() ? "" : str.substr(0, str.size() - 3);
}
std::string MbiPageProtectToString(int type)
{
	std::string str;
	if (type & PAGE_EXECUTE) str += "PAGE_EXECUTE | ";
	if (type & PAGE_EXECUTE_READ) str += "PAGE_EXECUTE_READ | ";
	if (type & PAGE_EXECUTE_READWRITE) str += "PAGE_EXECUTE_READWRITE | ";
	if (type & PAGE_EXECUTE_WRITECOPY) str += "PAGE_EXECUTE_WRITECOPY | ";
	if (type & PAGE_NOACCESS) str += "PAGE_NOACCESS | ";
	if (type & PAGE_READONLY) str += "PAGE_READONLY | ";
	if (type & PAGE_READWRITE) str += "PAGE_READWRITE | ";
	if (type & PAGE_GUARD) str += "PAGE_GUARD | ";
	if (type & PAGE_NOCACHE) str += "PAGE_NOCACHE | ";
	if (type & PAGE_WRITECOMBINE) str += "PAGE_WRITECOMBINE | ";
	return str.empty() ? "" : str.substr(0, str.size() - 3);
}
