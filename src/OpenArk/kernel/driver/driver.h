#pragma once
#include "../../common/cpp-wrapper/cpp-wrapper.h"
#include <QString>

bool SignExpiredDriver(QString driver);

#define RECOVER_SIGN_TIME() \
		SYSTEMTIME saved = { 0 };\
		GetLocalTime(&saved);\
		auto ticks = GetTickCount();\
		SYSTEMTIME cur = { 0 };\
		cur.wYear = 2012;\
		cur.wMonth = 1;\
		cur.wDay = 1;\
		SetLocalTime(&cur);\
		ON_SCOPE_EXIT([&] {	saved.wSecond += (GetTickCount() - ticks)/1000; SetLocalTime(&saved); });