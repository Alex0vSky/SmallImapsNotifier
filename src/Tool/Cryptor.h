// c:\prj\sysw\tinysynapticsscroll\src\Tool\Cryptor.h - cryptor via openssl
#pragma once // Copyright 2023 Alex0vSky (https://github.com/Alex0vSky)
namespace prj_sysw { namespace SmallImapsNotifier { namespace Tool { 
class Cryptor {
	static const int c_blockSize = 16;
	static constexpr auto cypher = EVP_aes_256_cbc;
	unsigned char key[ c_blockSize * 2 ], iv[ c_blockSize ];
	int outlen1, outlen2;
	std::shared_ptr< EVP_CIPHER_CTX > m_ctx;

 public:
	Cryptor()
		: key{ }
		, iv{ }
		, m_ctx{ EVP_CIPHER_CTX_new( ), [](EVP_CIPHER_CTX *p) { EVP_CIPHER_CTX_free( p ); } }
	{ 
		// Maximum user name length, #include <Lmcons.h>
		const auto UNLEN = 256;   
		wchar_t username[ UNLEN + 1 ] = { };
        DWORD cch = UNLEN;
        ::GetUserNameW( username, &cch );
		memcpy_s( key, sizeof( key ), username, cch * sizeof( wchar_t) );
	}
	std::vector<unsigned char> encrypt(std::string str) {
		auto pInput = reinterpret_cast<unsigned char *>( str.data( ) );
		auto inputLen = str.length( ) + 1;
		auto outLenAligned = ( ( inputLen + c_blockSize - 1 ) / c_blockSize ) * c_blockSize;
		std::vector<unsigned char> out( outLenAligned );
		// no error checks
		EVP_CipherInit( m_ctx.get( ), cypher(), key, iv, TRUE );
		EVP_CipherUpdate( m_ctx.get( ), out.data( ), &outlen1, pInput, (int)inputLen );
		EVP_CipherFinal( m_ctx.get( ), out.data( ) + outlen1, &outlen2 );
		return out;
	}
	std::string decrypt(const std::vector<unsigned char> &binary) {
		std::vector<unsigned char> out( binary.size( ) );
		// no error checks
		EVP_CipherInit( m_ctx.get( ), cypher( ), key, iv, FALSE );
		EVP_CipherUpdate( m_ctx.get( ), out.data( ), &outlen1, binary.data( ), (int)binary.size( ) );
		EVP_CipherFinal( m_ctx.get( ), out.data( ) + outlen1, &outlen2 );
		return reinterpret_cast<char *>( out.data( ) );
	}
};
}}} // namespace prj_sysw::SmallImapsNotifier::Tool _
