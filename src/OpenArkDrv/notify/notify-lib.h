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
#include <ntifs.h>

BOOLEAN GetProcessNotifyInfo(ULONG &count, PULONG64 &items);
BOOLEAN RemoveProcessNotify(ULONG64 routine);

BOOLEAN GetThreadNotifyInfo(ULONG &count, PULONG64 &items);
BOOLEAN RemoveThreadNotify(ULONG64 routine);

BOOLEAN GetImageNotifyInfo(ULONG &count, PULONG64 &items);
BOOLEAN RemoveImageNotify(ULONG64 routine);

BOOLEAN GetRegistryNotifyInfo(ULONG &count, PULONG64 &items);
BOOLEAN RemoveRegistryNotify(ULONG64 routine);