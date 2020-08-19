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
#include "kobject.h"
#include "../common/common.h"
#include <knone.h>
#include <ntifs.h>
#include <ntimage.h>
#include <ntintsafe.h>
#include <windef.h>

NTSTATUS ObjectDispatcher(IN ULONG op, IN PDEVICE_OBJECT devobj, PVOID inbuf, ULONG inlen, PVOID outbuf, ULONG outlen, IN PIRP irp)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	switch (op) {
	case ARK_OBJECT_TYPE_ENUM:
		break;
	case STORAGE_UNLOCK_CLOSE:
		break;
	default:
		break;
	}

	return status;
}