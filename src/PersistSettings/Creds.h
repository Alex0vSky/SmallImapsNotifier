// src\PersistSettings\Creds.h - place current settings values to ui, store changed values in registry
#pragma once // Copyright 2023 Alex0vSky (https://github.com/Alex0vSky)
namespace prj_sysw::SmallImapsNotifier::PersistSettings { 
template<class T = Tool::Registry>
class Creds {
	bool readOrCreateNew_(const char *szValueName, std::string *pstr, const std::string strNewValue) const { 
		Tool::HeapMem oBuf;
		if ( m_oReg.readString( szValueName, &oBuf ) ) {
			*pstr = reinterpret_cast<char *>( oBuf.getMemPtr( ) );
			return true;
		}
		*pstr = strNewValue;
		return m_oReg.createAndSetValueString( szValueName, strNewValue.c_str( ) );
	}
	bool readOrCreateNew_(const char *szValueName, DWORD *pdw, const DWORD dw) const { 
		if ( m_oReg.readDword( szValueName, pdw ) ) 
			return true;
		*pdw = dw;
		return m_oReg.createAndSetValueDword( szValueName, dw );
	}

	bool readOrCreateNew_(const char *szValueName, std::vector<unsigned char> *pvec) const { 
		if ( m_oReg.readBinary( szValueName, pvec ) ) 
			return true;
		std::vector<unsigned char> vec;
		*pvec = vec;	
		return m_oReg.createAndSetValueBinary( szValueName, vec );
	}

	bool setStaticDescr_(HWND hWndDialog, DWORD idc, DWORD dwValue) const {
		char szBuf256[ 256 + 1 ] = { };
		if ( !::GetDlgItemTextA( hWndDialog, idc, szBuf256, 256 ) )
			return false;
		std::string str = szBuf256;
		auto pos = str.find( ":" );
		if ( std::string::npos == pos )
			return false;
		str = str.substr( 0, pos );
		std::stringstream ss; ss << ": "; ss << dwValue;
		str += ss.str( );
		if ( !::SetDlgItemTextA( hWndDialog, idc, str.c_str( ) ) )
			return false;
		return true;
	}

	const Config::detail_::RegistryValueName m_stNames;
	// Saved states
	DWORD m_dwInterval_minutes, m_dwReconnectTO_sec;
	OsiApplication::Imaps::Credentials m_credentials;
	Tool::Cryptor m_ñryptor;
	
 protected:
	const T m_oReg;
	
 public:
	Creds(const Creds &) = delete;
	Creds() 
		: m_dwInterval_minutes( 0 ), m_dwReconnectTO_sec( 0 )
		, m_oReg( 
			T::openAlways( 
				Config::Movement::s_enuRootKey
				, Config::Movement::getRegSubkey( )
				, Tool::Registry::EnuDesiredAccessRights::ALL_ACCESS 
				) 
			)
		, m_credentials{ }
    {}
	bool readConfigFromRegistry() { 
		using Default = Config::Movement::DefaultSettings;
		if ( !readOrCreateNew_( m_stNames.c_szHost, &m_credentials.host, Default::c_szHost ) )
			return false;
		DWORD port;
		if ( !readOrCreateNew_( m_stNames.c_szPort, &port, Default::c_dwPort ) )
			return false;
		m_credentials.port = (uint16_t)port;

		std::vector<unsigned char> vec;
		if ( !readOrCreateNew_( m_stNames.c_szLogin, &vec ) )
			return false;
		if ( vec.size( ) )
			m_credentials.login = m_ñryptor.decrypt( vec );

		if ( !readOrCreateNew_( m_stNames.c_szPassword, &vec ) )
			return false;
		if ( vec.size( ) )
			m_credentials.password = m_ñryptor.decrypt( vec );

		if ( !readOrCreateNew_( m_stNames.c_szInterval, &m_dwInterval_minutes, Default::c_dwInterval_minutes ) )
			return false;
		if ( !readOrCreateNew_( m_stNames.c_szReconnectTO, &m_dwReconnectTO_sec, Default::c_dwReconnectTO_sec ) )
			return false;
		return true;
	}
	// @insp https://learn.microsoft.com/en-us/windows/win32/dlgbox/using-dialog-boxes // @insp https://stackoverflow.com/questions/24065214/how-to-make-checkbox-in-win32 // @insp https://qna.habr.com/q/482567
	bool loadToUi(HWND hWndDialog) const { 
		// By default all Ui controlls are empty/zero
		if ( !::SetDlgItemTextA( hWndDialog, IDC_EDIT_HOST, m_credentials.host.c_str( ) ) )
			return false;
		{
			std::stringstream ss; ss << m_credentials.port;
			if ( !::SetDlgItemTextA( hWndDialog, IDC_EDIT_PORT, ss.str( ).c_str( ) ) )
				return false;
		}
		if ( !::SetDlgItemTextA( hWndDialog, IDC_EDIT_LOGIN, m_credentials.login.c_str( ) ) )
			return false;
		if ( !::SetDlgItemTextA( hWndDialog, IDC_EDIT_PASSWORD, m_credentials.password.c_str( ) ) )
			return false;

		const BOOL bRedraw = TRUE;
		HWND hSliderInterval = ::GetDlgItem( hWndDialog, IDC_SLIDER_Interval );
		if ( !hSliderInterval ) 
			return false; //LRESULT lrCurPos1 = 
		::SendMessageA( hSliderInterval, TBM_SETRANGE, bRedraw, MAKELONG( 1, 60 ) ); 
		::SendMessageA( hSliderInterval, TBM_SETPOS, bRedraw, (LPARAM)m_dwInterval_minutes );
		setStaticDescr_( hWndDialog, IDC_STATIC_Interval, m_dwInterval_minutes );

		HWND hSliderReconnectTO = ::GetDlgItem( hWndDialog, IDC_SLIDER_ReconnectTO );
		if ( !hSliderReconnectTO ) 
			return false; //LRESULT lrCurPos1 = 
		::SendMessageA( hSliderReconnectTO, TBM_SETRANGE, bRedraw, MAKELONG( 5, 120 ) );
		::SendMessageA( hSliderReconnectTO, TBM_SETPOS, bRedraw, (LPARAM)m_dwReconnectTO_sec );
		setStaticDescr_( hWndDialog, IDC_STATIC_Reconnect, m_dwReconnectTO_sec );

		return true;
	}

	bool readCreds(HWND hWndDialog, OsiApplication::Imaps::Credentials *credentials) { 
		char szBuf256[ 256 + 1 ] = { };
		*credentials = { };
		UINT rv;
		rv = ::GetDlgItemTextA( hWndDialog, IDC_EDIT_HOST, szBuf256, 256 );
		if ( !rv ) {
			Tool::ErrorHandler::showMsg( "Please enter host value" );
			return false;
		}
		m_credentials.host = szBuf256;
		m_oReg.createAndSetValueString( m_stNames.c_szHost, szBuf256 );

		rv = ::GetDlgItemTextA( hWndDialog, IDC_EDIT_PORT, szBuf256, 256 );
		if ( !rv ) {
			Tool::ErrorHandler::showMsg( "Please enter port value" );
			return false;
		}
		m_credentials.port = std::atoi( szBuf256 );
		if ( !m_credentials.port ) {
			Tool::ErrorHandler::showMsg( "Invalid port value" );
			return false;
		}
		m_oReg.createAndSetValueDword( m_stNames.c_szPort, m_credentials.port );

		rv = ::GetDlgItemTextA( hWndDialog, IDC_EDIT_LOGIN, szBuf256, 256 );
		if ( !rv ) {
			Tool::ErrorHandler::showMsg( "Please enter login value" );
			return false;
		}
		m_credentials.login = szBuf256;
		m_oReg.createAndSetValueBinary( 
			m_stNames.c_szLogin, m_ñryptor.encrypt( m_credentials.login ) );

		rv = ::GetDlgItemTextA( hWndDialog, IDC_EDIT_PASSWORD, szBuf256, 256 );
		if ( !rv ) {
			Tool::ErrorHandler::showMsg( "Please enter password value" );
			return false;
		}
		m_credentials.password = szBuf256;
		m_oReg.createAndSetValueBinary( 
			m_stNames.c_szPassword, m_ñryptor.encrypt( m_credentials.password ) );

		*credentials = m_credentials;
		return true;
	}

	enum class enuSlider {
		Interval,
		Reconnect,
	};
	void setSliderValue(HWND hWndDialog, enuSlider enu, WORD wValue) {
		switch ( enu ) {
			case enuSlider::Interval:
		    {
				m_oReg.createAndSetValueDword( m_stNames.c_szInterval, wValue );
				m_dwInterval_minutes = wValue;
				setStaticDescr_( hWndDialog, IDC_STATIC_Interval, wValue );
			}
			break;
			case enuSlider::Reconnect:
		    {
				m_oReg.createAndSetValueDword( m_stNames.c_szReconnectTO, wValue );
				m_dwReconnectTO_sec = wValue;
				setStaticDescr_( hWndDialog, IDC_STATIC_Reconnect, wValue );
			}
			break;
		}
	}

	Creds &operator = (const Creds &) = delete;
};
} // namespace prj_sysw::SmallImapsNotifier::PersistSettings
