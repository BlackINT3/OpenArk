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
#include "common.h"
#include "../driver/driver.h"
#include "../notify/notify.h"

ARK_DRIVER ArkDrv;

BOOLEAN InitArkDriver(PDRIVER_OBJECT drvobj, PDEVICE_OBJECT devobj)
{
	ArkDrv.drvobj = drvobj;
	ArkDrv.devobj = devobj;
	
	ArkDrv.ver = KNONE::OsNtVersion();
	ArkDrv.major = KNONE::OsMajorVersion();
	ArkDrv.minor = KNONE::OsMinorVersion();
	ArkDrv.build = KNONE::OsBuildNumber();

	InitDriverDispatcher();
	InitNotifyDispatcher();
	
	return TRUE;
}

PVOID GetNtRoutineAddress(IN PCWSTR name)
{
	UNICODE_STRING ustr;
	RtlInitUnicodeString(&ustr, name);
	return MmGetSystemRoutineAddress(&ustr);
}