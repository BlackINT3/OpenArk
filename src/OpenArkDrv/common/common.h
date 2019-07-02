#pragma once
#include <knone.h>
#include <ntifs.h>

typedef struct _ARK_DRIVER {
	PDRIVER_OBJECT drvobj;
	PDEVICE_OBJECT devobj;
	ULONG ver;
	ULONG major;
	ULONG minor;
	ULONG build;
	PVOID ps_notify;


} ARK_DRIVER, *PARK_DRIVER;

extern ARK_DRIVER ArkDrv;

BOOLEAN InitArkDriver(PDRIVER_OBJECT drvobj, PDEVICE_OBJECT devobj);
