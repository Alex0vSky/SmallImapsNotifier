// src\Main.h - code main class, for unittests code coverage
#pragma once // Copyright 2023 Alex0vSky (https://github.com/Alex0vSky)
#include "resource.h" // NOLINT(build/include_subdir)
#include "Tool\ErrorHandler.h"
#include "Tool\HeapMem.h"
#include "Tool\Registry.h"
#include "Tool\Trait.h"
#include "Tool\Cryptor.h"
#include "Config.h"
typedef const std::string & crstr_t;
#include "OsiApplication\Imaps\Credentials.h"
#include "PersistSettings\Creds.h"
#include "ForwardToInstance.h"
#include "OsiApplication\Imaps\StateMachine\Helper.h"
#include "OsiApplication\Imaps\StateMachine\Impl.h"
#include "OsiApplication\Imaps\Thread.h"
#include "DialogPopupSettings.h"
#include "Menu.h"
#include "Tray.h"
#include "WndProc.h"
#include "Version.h"

namespace prj_sysw::SmallImapsNotifier {
struct Main {
	static UINT run(HINSTANCE hInst) {
		// Objects chain
		PersistSettings::Creds<> oSettingsCreds;
		if ( !oSettingsCreds.readConfigFromRegistry( ) ) 
			return Tool::ErrorHandler::showMsg( "Read or initialize config from registry" );
		DialogPopupSettings< > oDialog( &oSettingsCreds );
		Menu< > oMenu( &oDialog );
		Tray< > oTray( hInst, oDialog );
		WndProc oWndProc( &oTray, &oMenu );
		if ( !oTray.init( WndProc::systemCallback, &oWndProc, oMenu ) )
			return Tool::ErrorHandler::showMsg( "SistemTray initialization" );
		::EmptyWorkingSet( ::GetCurrentProcess( ) ); // K32EmptyWorkingSet()

		// After launch need to show dialog
		oDialog.create( oTray.getHwnd( ) );
		
		//::SendMessageA( oDialog.getHandle( ), WM_COMMAND, MAKEWPARAM( IDOK, 0 ), 0 ); // tmp

		// TODO(alex): take cert from OS or hardcode

		MSG stMsg = { };
		while ( ::GetMessageA( &stMsg, NULL, 0, 0 ) ) {
			// Automatic translate and dispatch message to dialog by single call 
			if ( !::IsDialogMessageA( oDialog.getHandle( ), &stMsg ) ) {
				(void)::TranslateMessage( &stMsg ), ::DispatchMessageA( &stMsg );
			}
		}
		return static_cast< UINT >( stMsg.wParam );
	}
};
} // namespace prj_sysw::SmallImapsNotifier
