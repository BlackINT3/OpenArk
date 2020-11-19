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
#include <knone.h>
#include <ntifs.h>

#define ARK_POOLTAG 'OARK'

typedef struct _ARK_DRIVER {
	PDRIVER_OBJECT drvobj;
	PDEVICE_OBJECT devobj;
	ULONG ver;
	ULONG major;
	ULONG minor;
	ULONG build;
	PVOID process_notify;
	PVOID thread_notify;
	PVOID image_notify;
	PVOID registry_notify;
} ARK_DRIVER, *PARK_DRIVER;

extern ARK_DRIVER ArkDrv;

typedef enum {
	_NTOS_UNKNOWN,
	_NTOS_WINXP,
	_NTOS_WINXPSP1,
	_NTOS_WINXPSP2,
	_NTOS_WINXPSP3,
	_NTOS_WIN2003,
	_NTOS_WIN2003SP1,
	_NTOS_WIN2003SP2,
	_NTOS_WINVISTA,
	_NTOS_WINVISTASP1,
	_NTOS_WINVISTASP2,
	_NTOS_WIN7,
	_NTOS_WIN7SP1,
	_NTOS_WIN8,
	_NTOS_WIN81,
	_NTOS_WIN10_1507, //10240
	_NTOS_WIN10_1511, //10586
	_NTOS_WIN10_1607, //14393
	_NTOS_WIN10_1703, //15063
	_NTOS_WIN10_1709, //16299
	_NTOS_WIN10_1803, //17134
	_NTOS_WIN10_1809, //17763
	_NTOS_WIN10_1903, //18362
	_NTOS_WIN10_1909, //18363
	_NTOS_WIN10_2004, //19041
	_NTOS_WIN10_20H2, //19042
} NTOS_VERSION_X, *PNTOS_VERSION_X;

BOOLEAN InitArkDriver(PDRIVER_OBJECT drvobj, PDEVICE_OBJECT devobj);

PVOID GetNtRoutineAddress(IN PCWSTR name);

NTSTATUS DuplicateInputBuffer(IN PIRP irp, PVOID &inbuf);
NTSTATUS ReleaseInputBuffer(IN PIRP irp, PVOID &inbuf);
