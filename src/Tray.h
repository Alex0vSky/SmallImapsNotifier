// src\Tray.h - systemtray, create and process window messages
#pragma once // Copyright 2023 Alex0vSky (https://github.com/Alex0vSky)
namespace prj_sysw::SmallImapsNotifier { 
class WndProc;
template<class T = WndProc, class T2 = Menu< > >
class Tray {
	// Must be a valid or LoadIcon() == ERROR_RESOURCE_TYPE_NOT_FOUND;
	const HINSTANCE m_hInst;
	const HICON m_hIcon;
	const UINT c_uWM_TASKBAR_CREATE;
	HWND m_hHiddenWnd;
	static const UINT c_uIdTrayIcon = 1;
	NOTIFYICONDATAA m_stNotifyIconData;

	// @insp ...
	template <std::size_t N1, std::size_t N2> static constexpr void str_chars_(const char (&szSrc)[N1], char (&szDst)[N2]) {
		static_assert( N1 <= N2, "source 'szSrc' string is too long" );
		for ( std::size_t i = 0; i < N1 && i < N2; ++i ) 
			szDst[ i ] = szSrc[ i ];
	}
	
 public:
	explicit Tray(HINSTANCE hInst, DialogPopupSettings< > &oDialog)
		: m_hInst( hInst )
		, m_hIcon( ::LoadIconA( hInst, static_cast<LPCTSTR>( MAKEINTRESOURCEA( IDI_ICON1 ) ) ) )
		, c_uWM_TASKBAR_CREATE( ::RegisterWindowMessageA( "TaskbarCreated" ) )
		, m_hHiddenWnd( nullptr )
		, m_stNotifyIconData{ }
    { 
		using namespace std::placeholders;
		oDialog.setClb( 
				std::bind( &Tray< >::setActiveTip, this, _1 ) 
				, std::bind( &Tray< >::showBalloon, this, _1, _2 ) 
			);
	}
	
	bool init(WNDPROC lpfnWndProc, T *poWndProc, const T2 &oMenu) {
		char szClassName[] = "TinySynapticsScrollApplication";
		if ( !m_hIcon ) 
			return false;
		// @insp https://github.com/marek/trayframework/blob/master/trayframework.c
		const INITCOMMONCONTROLSEX stIccex = { sizeof( INITCOMMONCONTROLSEX ), ICC_UPDOWN_CLASS | ICC_LISTVIEW_CLASSES };
		if ( !::InitCommonControlsEx( &stIccex ) ) 
			return false;
		WNDCLASSEXA stWc = { };
		stWc.cbSize = sizeof( stWc ); 
		stWc.style = CS_HREDRAW | CS_VREDRAW; 
		stWc.lpfnWndProc = lpfnWndProc;
		stWc.hInstance = m_hInst;
		stWc.hIcon = m_hIcon; 
		stWc.lpszClassName = szClassName;
		stWc.hIconSm = m_hIcon;
		if ( !::RegisterClassExA( &stWc ) )
			return false;

		// Create the hidden window
		m_hHiddenWnd = ::CreateWindowExA( 
				WS_EX_CLIENTEDGE 
				, szClassName, ""
				, WS_OVERLAPPEDWINDOW
				, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT
				, NULL, NULL, m_hInst
				, poWndProc
			);
		if ( !m_hHiddenWnd ) 
			return false;

		m_stNotifyIconData = { }; 
		m_stNotifyIconData.cbSize = sizeof( m_stNotifyIconData );

		//// @insp https://www.codeproject.com/Articles/4768/Basic-use-of-Shell-NotifyIcon-in-Win32#fromHistory
		//// @insp https://www.codeproject.com/Articles/207/Determining-the-version-number-of-a-DLL-or-Executa
		//typedef LPCSTR (WINAPI *GetDllVersion_t)(void);
		//HMODULE hCabinet = LoadLibraryA( "Cabinet.dll" );
		//GetDllVersion_t pfnGetDllVersion = (GetDllVersion_t)GetProcAddress( hCabinet, "GetDllVersion" );
		//if ( pfnGetDllVersion ) {
		//	// "Shell32.dll"
		//	ULONGLONG ullVersion;
		//	ullVersion = pfnGetDllVersion( );
		//	FreeLibrary( hCabinet );
		//	if(ullVersion >= MAKEDLLVERULL(6,0,0,0))
		//		m_stNotifyIconData.cbSize = sizeof(NOTIFYICONDATA);
		//	else if(ullVersion >= MAKEDLLVERULL(5,0,0,0))
		//		m_stNotifyIconData.cbSize = NOTIFYICONDATA_V2_SIZE;
		//	else 
		//		m_stNotifyIconData.cbSize = NOTIFYICONDATA_V1_SIZE;
		//}

		m_stNotifyIconData.hWnd = m_hHiddenWnd;
		m_stNotifyIconData.uID = c_uIdTrayIcon;
		m_stNotifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		m_stNotifyIconData.uCallbackMessage = oMenu.getWindowMessageId( );
		m_stNotifyIconData.hIcon = m_hIcon;
#ifdef _DEBUG
		str_chars_( "SmallImapsNotifier: waiting for credentials\n[DEBUG]", m_stNotifyIconData.szTip );
#else
		str_chars_( "SmallImapsNotifier: waiting for credentials", m_stNotifyIconData.szTip );
#endif
		// Display tray icon
		if ( !::Shell_NotifyIconA( NIM_ADD, &m_stNotifyIconData ) ) 
			return false;

		// HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced\EnableBalloonTips
		// Set Version 5 behaviour for balloon feature
		m_stNotifyIconData.uVersion = NOTIFYICON_VERSION;
		if ( !::Shell_NotifyIconA( NIM_SETVERSION, &m_stNotifyIconData ) )
			return false;
		return true;
	}
	bool handleWindowMessage(HWND hWnd, UINT uMsg, WPARAM, LPARAM) {
		// Taskbar has been recreated, cause: process explorer.exe has been restarted
		if ( uMsg == c_uWM_TASKBAR_CREATE ) { 
			if ( ::Shell_NotifyIconA( NIM_ADD, &m_stNotifyIconData ) ) 
				return true;
			Tool::ErrorHandler::showMsg( "Systray icon recreation" );
			::DestroyWindow( hWnd );
		}
		// Remove Tray Item
		if ( WM_DESTROY == uMsg ) {
			::Shell_NotifyIconA( NIM_DELETE, &m_stNotifyIconData );
			m_hHiddenWnd = nullptr;
		}
		// yes, false and pass to caller
		return false;
	}
	HWND getHwnd() const {
		return m_hHiddenWnd;
	}

	bool setActiveTip(crstr_t tipPart) {
		std::string tip = tipPart;
		tip += "\n";
		tip += "Settings or exit";
#ifdef _DEBUG
		tip += " [DEBUG]";
#endif
		strcpy_s( m_stNotifyIconData.szTip, tip.c_str( ) );
		if ( !::Shell_NotifyIconA( NIM_MODIFY, &m_stNotifyIconData ) ) 
			return false;
		return true;
	}
	// @insp https://social.msdn.microsoft.com/Forums/en-US/0af102b3-5e2e-409b-a261-7ebdb6438252/displaying-tooltip-on-system-tray-notification?forum=vcgeneral
	bool showBalloon(crstr_t title, crstr_t text) {
		m_stNotifyIconData.uFlags = NIF_INFO;
		strcpy_s( m_stNotifyIconData.szInfoTitle, title.c_str( ) );
		strcpy_s( m_stNotifyIconData.szInfo, text.c_str( ) );
		const UINT uTimeout_milli = 10000;
		m_stNotifyIconData.uTimeout = uTimeout_milli;
		// or NIIF_WARNING or NIIF_ERROR
		m_stNotifyIconData.dwInfoFlags = NIIF_INFO;

		if ( !::Shell_NotifyIconA( NIM_MODIFY, &m_stNotifyIconData ) ) 
			return false;
		return true;
	}

	Tray &operator = (const Tray &) = delete;
};
} // namespace prj_sysw::SmallImapsNotifier
