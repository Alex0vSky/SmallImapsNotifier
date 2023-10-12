// src\WndProc.h - base messages behavior, and forward window message to others class
#pragma once // Copyright 2023 Alex0vSky (https://github.com/Alex0vSky)
namespace prj_sysw { namespace SmallImapsNotifier { 
namespace detail_ { 
template<class T2 = Tray< >, class T3 = Menu< >>
class WndProc_ : public ForwardToInstance::Base {
	static const LRESULT c_dwHandled = 0; // warning, all return code depends of message kind
	LRESULT messagesHandler_(UINT uMsg, WPARAM wParam, LPARAM lParam) {
		if ( false 
//			|| m_oRestoreTouchPad.handleWindowMessage( m_hWnd, uMsg, wParam, lParam )
			|| m_poTray ->handleWindowMessage( m_hWnd, uMsg, wParam, lParam )
			|| m_poMenu ->handleWindowMessage( m_hWnd, uMsg, wParam, lParam )
		)
			return c_dwHandled;
		switch( uMsg ) {
			default: 
				return ::DefWindowProcA( m_hWnd, uMsg, wParam, lParam );
			case WM_CLOSE:
				::DestroyWindow( m_hWnd );
			break;
			case WM_DESTROY:
				::PostQuitMessage( 0 );
			break;
		}
		return c_dwHandled;
	}
	static constexpr auto cast_v = &CREATESTRUCTA::lpCreateParams;
	
 protected:
//	T m_oRestoreTouchPad;
	T2 *m_poTray;
	T3 *m_poMenu;
	
 public: 
//	WndProc_(WrapperTouchPad<> *poTouchPad, T2 *poTray, T3 *poMenu) 
//		: m_oRestoreTouchPad( poTouchPad )
	WndProc_(T2 *poTray, T3 *poMenu) 
		: m_poTray( poTray )
		, m_poMenu( poMenu )
    {}

	using parrent_t = ForwardToInstance::OfWindow< 
			decltype( &WndProc_::messagesHandler_ )
			, &WndProc_::messagesHandler_
			, decltype( &CREATESTRUCTA::lpCreateParams )
			, &CREATESTRUCTA::lpCreateParams 
		>;
};
} // namespace detail_ _
class WndProc : public detail_::WndProc_< >, public detail_::WndProc_< >::parrent_t {
	using detail_::WndProc_< >::WndProc_;
	WndProc() = delete;
};
}} // namespace prj_sysw::SmallImapsNotifier _
