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