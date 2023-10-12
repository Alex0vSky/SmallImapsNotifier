// src\ForwardToInstance.h - convert static call to class instance
// @insp https://stackoverflow.com/questions/35178779/wndproc-as-class-method @insp https://devblogs.microsoft.com/oldnewthing/20191014-00/?p=102992 @insp https://learn.microsoft.com/en-us/windows/win32/winmsg/using-window-procedures
#pragma once // Copyright 2023 Alex0vSky (https://github.com/Alex0vSky)
namespace prj_sysw { namespace SmallImapsNotifier { namespace ForwardToInstance { 
class Base;
namespace detail_ { 
// Select cast of PointerToStructField
namespace PtsfCastSwitcher { 
namespace detail_ { 
	struct Unused_ { bool bUnused_; };
} // namespace detail_ _
	static constexpr auto c_directLParam_v = &detail_::Unused_::bUnused_;
	using directLParam_t = std::remove_const_t< decltype( c_directLParam_v ) >;
	template <class T>
	static constexpr bool isDirectLParam() { return sizeof( T ) == sizeof( detail_::Unused_::bUnused_ ); }
	static_assert( 
			isDirectLParam< decltype( detail_::Unused_::bUnused_ ) >( ) || !isDirectLParam< void* >( )
			, "sizeof( void* ) cannot be equal sizeof( bool )" 
		);
} // namespace PtsfCastSwitcher _
template <typename TPTMF, typename TPTSF, typename LAMBDA>
static LRESULT viaWindowLongPtr(
		HWND hWndMsg, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT uInitMsg, TPTMF VPTMF, TPTSF VPTSF, LAMBDA fnDefault
) {
	using T = Tool::Trait::ptmf::class_t< TPTMF >;
	static_assert( 
			std::is_base_of_v< ForwardToInstance::Base, T >
			, "class T must be inherited from 'ForwardToInstance::Base'" 
		); 
	T *self = nullptr;
	using TOFFSET = Tool::Trait::ptsf::field_t< TPTSF >;
	using TSTRUCT = Tool::Trait::ptsf::struct_t< TPTSF >;
	TSTRUCT *pStruct = reinterpret_cast< TSTRUCT * >( lParam );
	if ( uInitMsg == uMsg ) { 
		self = ( PtsfCastSwitcher::isDirectLParam< TOFFSET >( ) ) 
				?reinterpret_cast<T *>( pStruct )
				:reinterpret_cast<T *>( pStruct ->*VPTSF );
		self ->setHwnd( hWndMsg );
		::SetLastError( 0 );
		LONG_PTR previousOrError = ::SetWindowLongPtrA( hWndMsg, GWLP_USERDATA, reinterpret_cast< LONG_PTR >( self ) );
		if ( !previousOrError && 0 != ::GetLastError( ) ) 
			return FALSE;
	} else {
		self = reinterpret_cast< T * >( ::GetWindowLongPtrA( hWndMsg, GWLP_USERDATA ) );
	}
	if ( self ) 
		return (self ->*VPTMF)( uMsg, wParam, lParam );
	return fnDefault( hWndMsg, uMsg, wParam, lParam );
}
} // namespace detail_ _

class Base {
 protected:
	void setHwnd(HWND hWnd) {
		m_hWnd = hWnd;
	}
	HWND m_hWnd;
	Base()
		: m_hWnd( nullptr ) 
    {}
	template <typename TPTMF, typename TPTSF, typename LAMBDA>
	friend LRESULT detail_::viaWindowLongPtr(
			HWND hWndMsg, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT uInitMsg, TPTMF VPTMF, TPTSF VPTSF, LAMBDA fnDefault
		);

	Base(const Base &) = delete;
	Base& operator = (const Base&) = delete;
};

template <typename TPTMF, TPTMF VPTMF, typename TPTSF, TPTSF VPTSF> 
struct OfWindow {
	static Tool::Trait::ptmf::return_type_t< TPTMF > CALLBACK systemCallback(
		HWND hWndMsg, UINT uMsg, WPARAM wParam, LPARAM lParam
	) {
		return detail_::viaWindowLongPtr< TPTMF, std::remove_const_t< TPTSF > >( 
				hWndMsg, uMsg, wParam, lParam, WM_NCCREATE, VPTMF, VPTSF
				, [](HWND hWndMsg_, UINT uMsg_, WPARAM wParam_, LPARAM lParam_) ->LRESULT {
						return ::DefWindowProcA( hWndMsg_, uMsg_, wParam_, lParam_ );
					}
			);
	}
};
template <typename TPTMF, TPTMF VPTMF> 
struct OfDialog {
	static Tool::Trait::ptmf::return_type_t< TPTMF > CALLBACK systemCallback(
		HWND hWndMsg, UINT uMsg, WPARAM wParam, LPARAM lParam
	) {
		return detail_::viaWindowLongPtr< TPTMF, detail_::PtsfCastSwitcher::directLParam_t >( 
				hWndMsg, uMsg, wParam, lParam, WM_INITDIALOG, VPTMF, detail_::PtsfCastSwitcher::c_directLParam_v
				, [](HWND, UINT, WPARAM, LPARAM) ->BOOL {
						return FALSE;
					}
			);
	}
};

}}} // namespace prj_sysw::SmallImapsNotifier::ForwardToInstance _
