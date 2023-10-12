// src\DialogPopupSettings.h - create popup dialog from menu of tray icon
#pragma once // Copyright 2023 Alex0vSky (https://github.com/Alex0vSky)
namespace prj_sysw::SmallImapsNotifier { 
namespace detail_ { 
template<class T = PersistSettings::Creds<>>
class DialogPopupSettings_ : public ForwardToInstance::Base {

	OsiApplication::Imaps::Thread::uptr_t m_imapsThread;
	// For Tray
	using setActiveTip_t = OsiApplication::Imaps::StateMachine::setActiveTip_t;
	using showBalloon_t = OsiApplication::Imaps::StateMachine::showBalloon_t;
	setActiveTip_t m_clbSetActiveTip;
	showBalloon_t m_clbShowBalloon;

	bool setCreds_(const OsiApplication::Imaps::Credentials &credentials) {
		if ( m_imapsThread ) {
			if ( m_imapsThread ->ñredentialsEqual( credentials ) )
				return true;
			m_imapsThread ->stop( );
			m_imapsThread.reset( );
		}
		m_imapsThread = std::make_unique< OsiApplication::Imaps::Thread >( 
				credentials
				, m_clbSetActiveTip
				, m_clbShowBalloon
			);

		// TODO(alex): async, via WM_TIMER?
		m_clbSetActiveTip( "SmallImapsNotifier: checking access...\n" + credentials.host );
		while( m_imapsThread ->m_bWaitingCheck ) {
			::Sleep( 300 );
		}
		if ( !m_imapsThread ->m_bCheckSucc ) {
			m_clbSetActiveTip( "SmallImapsNotifier: fail\n" + credentials.host );
			return false;
		}
		m_clbSetActiveTip( "SmallImapsNotifier: on working\n" + credentials.host );

		return true;
	}

	INT_PTR dialogFunc_(UINT uMsg, WPARAM wParam, LPARAM lParam) { 
		switch ( uMsg )  { 
			default:
				return FALSE;
			case WM_SYSCOMMAND: {
				// Close via click SystemMenu on TitleBar, or [ALT+F4]
				if ( SC_CLOSE == ( wParam & 0xfff0 ) ) { 
					::DestroyWindow( m_hWnd );
					m_hWnd = nullptr;
				}
			}
			break;
			case WM_COMMAND: 
				switch ( LOWORD( wParam ) ) { 
					// And too keyboard [ESC] pressed
					case IDCANCEL:
						::EndDialog( m_hWnd, (INT_PTR)wParam );
						m_hWnd = nullptr;
					break;
					// And too keyboard [Enter] pressed
					case IDOK: {
						OsiApplication::Imaps::Credentials credentials;
						if ( !m_poSettingsCreds ->readCreds( m_hWnd, &credentials ) )
							break;
						if ( !setCreds_( credentials ) ) {
							Tool::ErrorHandler::showMsg( "Invalid credentials" );
							break;
						}
						::EndDialog( m_hWnd, (INT_PTR)wParam );
						m_hWnd = nullptr;
					}
					break;
				} 
			break;
			case WM_HSCROLL: 
			    {
					HWND hSlider = reinterpret_cast<HWND>( lParam );
					WORD wRequestKind = LOWORD( wParam );
					if ( SB_ENDSCROLL != wRequestKind || !hSlider ) 
						break;
					WORD wPosition = (WORD)::SendMessageA( hSlider, TBM_GETPOS, 0, 0 );
					if ( false ) {
					} else if ( hSlider == ::GetDlgItem( m_hWnd, IDC_SLIDER_Interval ) ) {
						m_poSettingsCreds ->setSliderValue( m_hWnd, T::enuSlider::Interval, wPosition );
					} else if ( hSlider == ::GetDlgItem( m_hWnd, IDC_SLIDER_ReconnectTO ) ) {
						m_poSettingsCreds ->setSliderValue( m_hWnd, T::enuSlider::Reconnect, wPosition );
					}
				}
			break;
		}
		return TRUE;
	}

 protected:
	T *m_poSettingsCreds;
	bool init_() {
		return m_poSettingsCreds ->loadToUi( m_hWnd );
	}
	
 public:
	explicit DialogPopupSettings_(T *poSettingsScroll) 
		: m_poSettingsCreds( poSettingsScroll ) 
	 { }
	~DialogPopupSettings_() {
		m_imapsThread ->stop( );
	}

	HWND getHandle() const {
		return m_hWnd;
	}
	using parrent_t = ForwardToInstance::OfDialog< 
			decltype( &DialogPopupSettings_::dialogFunc_ )
			, &DialogPopupSettings_::dialogFunc_ 
		>;

	void setClb(setActiveTip_t clb_setActiveTip, showBalloon_t clb_showBalloon) {
		m_clbSetActiveTip = clb_setActiveTip;
		m_clbShowBalloon = clb_showBalloon;
	}
};
} // namespace detail_ _

template<class T = PersistSettings::Creds<>>
class DialogPopupSettings 
	: public detail_::DialogPopupSettings_< T >
	, public detail_::DialogPopupSettings_< T >::parrent_t {
	using detail_::DialogPopupSettings_< T >::DialogPopupSettings_;
 public:
	// @insp https://learn.microsoft.com/en-us/windows/win32/dlgbox/using-dialog-boxes
	bool create(HWND hWndParrent) { 
#pragma warning( push )
#pragma warning( disable: 5039 )
		// TODO: hwnd leak on recreate
		HWND hWnd = ::CreateDialogParamA( 
				(HINSTANCE)::GetModuleHandleA( NULL )
				, static_cast<LPCTSTR>( MAKEINTRESOURCE( IDD_DIALOGBAR ) )
				, hWndParrent
				, detail_::DialogPopupSettings_< T >::parrent_t::systemCallback
				, (LPARAM)this
			);
#pragma warning( pop )
		if ( !hWnd )
			return false;
		if ( !detail_::DialogPopupSettings_< T >::init_( ) )
			return false;

		// @insp SO/questions/7410846/using-an-icon-on-a-dialog-box-window-c-win32-api
		HICON hIcon = ::LoadIconA( 
				(HINSTANCE)::GetModuleHandleA( NULL )
				, static_cast<LPCTSTR>( MAKEINTRESOURCEA( IDI_ICON1 ) ) 
			);
		SendMessageW( hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon );

		::ShowWindow( hWnd, SW_SHOW );
		return true;
	}
	DialogPopupSettings() = delete;
};
} // namespace prj_sysw::SmallImapsNotifier
