// src\OsiApplication\Imaps\Credentials.h - IMAP access credentials
#pragma once // Copyright 2023 Alex0vSky (https://github.com/Alex0vSky)
namespace prj_sysw::SmallImapsNotifier::OsiApplication::Imaps {
struct Credentials {
	std::string host;
	uint16_t port;
	std::string login, password;
};
namespace detail_ {
	auto credentialsToTuple(const Credentials& p) {
		return std::make_tuple( p.host, p.port, p.login, p.password );
	}
} // namespace detail_
bool operator==(const Credentials& lhs, const Credentials& rhs ) {
	return detail_::credentialsToTuple(lhs) == detail_::credentialsToTuple(rhs);
}
} // namespace prj_sysw::SmallImapsNotifier::OsiApplication::Imaps _
