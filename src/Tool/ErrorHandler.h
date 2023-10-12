// src\Tool\ErrorHandler.h - little error handling and logging
#pragma once // Copyright 2023 Alex0vSky (https://github.com/Alex0vSky)
namespace prj_sysw { namespace SmallImapsNotifier { namespace Tool { 
class ErrorHandler {
 public:
	static UINT showMsg(const char *szText) {
		::MessageBoxA( NULL, szText, "SmallImapsNotifier error!", MB_ICONEXCLAMATION | MB_OK );
		return 0;
	}
	static void log(const char *szText) {
		::OutputDebugStringA( szText );
	}
};
}}} // namespace prj_sysw::SmallImapsNotifier::Tool _
