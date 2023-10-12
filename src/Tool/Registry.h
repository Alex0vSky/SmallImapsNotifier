// src\Tool\Registry.h - minor registry operations
#pragma once // Copyright 2023 Alex0vSky (https://github.com/Alex0vSky)
namespace prj_sysw { namespace SmallImapsNotifier { namespace Tool { 
class Registry {
	const HKEY m_hRootKey;
	HKEY m_hSubKey;
	const REGSAM m_samDesired;
	Registry(HKEY hRootKey, HKEY hSubKey, REGSAM samDesired) 
		: m_hRootKey( hRootKey ), m_hSubKey( hSubKey ), m_samDesired( samDesired )
    {}
	
 public:
	static const size_t s_hInitialSubKey = MAXSIZE_T;
	~Registry() {
		::RegCloseKey( m_hRootKey );
		if ( reinterpret_cast< HKEY >( s_hInitialSubKey ) != m_hSubKey )
			::RegCloseKey( m_hSubKey );
	}
	enum class EnuRootKey {
		// HKEY_CURRENT_USER
		HKCU, 
		// HKEY_LOCAL_MACHINE
		HKLM, 
	};
	enum class EnuDesiredAccessRights {
		// KEY_ALL_ACCESS 
		ALL_ACCESS, 
		// KEY_READ 
		READ, 
	};

	bool readString(const char *szValueName, Tool::HeapMem *poBuf, size_t *psiLen = nullptr) const { 
		DWORD cbData = (DWORD)poBuf ->getSize( );
		LSTATUS lst = ::RegGetValueA( 
			m_hSubKey, nullptr, szValueName, RRF_RT_REG_SZ, nullptr, poBuf ->getMemPtr( ), &cbData );
		if ( ERROR_SUCCESS != lst )
			return false;
		if ( psiLen )
			*psiLen = ( ( cbData / sizeof( szValueName[ 0 ] ) ) - 1 );
		return true;
	}
	bool readDword(const char *szValueName, DWORD *pdwReadValue) const { 
		DWORD dwType = REG_DWORD, cbData = sizeof( DWORD );
		return ERROR_SUCCESS == ::RegQueryValueExA( 
			m_hSubKey, szValueName, nullptr, &dwType, reinterpret_cast<LPBYTE>( pdwReadValue ), &cbData );
	}
	bool createAndSetValueDword(const char *szValueName, DWORD dwValue) const { 
		return ERROR_SUCCESS == ::RegSetValueExA( 
			m_hSubKey, szValueName, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof( DWORD ) );
	}
	bool createAndSetValueString(const char *szValueName, const char *szValue) const { 
		size_t cchLength = 0; 
		const char *sz = szValue; 
		while ( *(sz++) ) {}
		--sz; 
		cchLength = static_cast<size_t>( sz - szValue ); // aka strlen
		LSTATUS lst = ::RegSetValueExA( 
			m_hSubKey, szValueName, 0, REG_SZ, reinterpret_cast<const BYTE *>( szValue )
			, DWORD( cchLength *sizeof( char ) ) );
		return ERROR_SUCCESS == lst;
	}
	bool deleteValue(const wchar_t *wcsValueName) const { 
		return ERROR_SUCCESS == ::RegDeleteValueW( m_hSubKey, wcsValueName );
	}

	bool createAndSetValueBinary(const char *szValueName, const std::vector<unsigned char> &value) const { 
		size_t cchLength = value.size( ); 
		LSTATUS lst = ::RegSetValueExA( 
			m_hSubKey, szValueName, 0, REG_BINARY, reinterpret_cast<const BYTE *>( value.data( ) )
			, DWORD( value.size( ) ) );
		return ERROR_SUCCESS == lst;
	}
	bool readBinary(const char *szValueName, std::vector<unsigned char> *pvalue) const { 
		DWORD dwType = REG_BINARY, cbData = 0;
		auto lst = ::RegQueryValueExA( 
			m_hSubKey, szValueName, nullptr, &dwType, nullptr, &cbData );
		if ( ERROR_SUCCESS != lst )
			return false;
		pvalue ->resize( cbData );
		return ERROR_SUCCESS == ::RegQueryValueExA( 
				m_hSubKey, szValueName, nullptr, &dwType 
				, reinterpret_cast<LPBYTE>( pvalue ->data( ) ), &cbData 
			);
	}

	// RVO
	static Registry openAlways(
		EnuRootKey enuRootKey
		, const wchar_t *wcsSubKey
		, EnuDesiredAccessRights enuRights = EnuDesiredAccessRights::READ
	) {
		HKEY hRootKey = (HKEY)-1;
		if ( false ) {}
		else if ( EnuRootKey::HKCU == enuRootKey )
			hRootKey = HKEY_CURRENT_USER;
		else if ( EnuRootKey::HKLM == enuRootKey )
			hRootKey = HKEY_LOCAL_MACHINE;
		REGSAM samDesired = 0;
		if ( false ) {}
		else if ( EnuDesiredAccessRights::ALL_ACCESS == enuRights )
			samDesired = KEY_ALL_ACCESS;
		else if ( EnuDesiredAccessRights::READ == enuRights )
			samDesired = KEY_READ;
		HKEY hSubKey = reinterpret_cast< HKEY >( s_hInitialSubKey );
		DWORD ulOptions = REG_OPTION_NON_VOLATILE; // default
		if ( ERROR_SUCCESS != ::RegOpenKeyExW( hRootKey, wcsSubKey, ulOptions, samDesired, &hSubKey ) )
			::RegCreateKeyExW( hRootKey, wcsSubKey, 0, nullptr, ulOptions, samDesired, nullptr, &hSubKey, nullptr );
		return Registry( hRootKey, hSubKey, samDesired );
	}
	Registry(const Registry&& rhs) 
		: m_hRootKey( std::move( rhs.m_hRootKey ) ) // NOLINT(build/include_what_you_use)
		, m_hSubKey( std::move( rhs.m_hSubKey ) ) // NOLINT(build/include_what_you_use)
		, m_samDesired( std::move( rhs.m_samDesired ) ) // NOLINT(build/include_what_you_use)
    {}
	Registry(const Registry&) = delete;

	Registry &operator = (const Registry &) = delete;
};
}}} // namespace prj_sysw::SmallImapsNotifier::Tool _
