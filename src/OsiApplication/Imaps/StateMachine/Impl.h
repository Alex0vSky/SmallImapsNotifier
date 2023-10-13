// src\OsiApplication\Imaps\StateMachine\Impl.h - IMAP State Machine defininion
#pragma once // Copyright 2023 Alex0vSky (https://github.com/Alex0vSky)
namespace prj_sysw::SmallImapsNotifier::OsiApplication::Imaps::StateMachine {

// events
struct CheckAccess{};
struct Unavailable{};
struct StartLoop{};
struct HasBreak{};
struct RenewEmail{};
struct ContinueLoop{};

typedef std::function< bool(crstr_t) > setActiveTip_t;
typedef std::function< bool(crstr_t, crstr_t) > showBalloon_t;

// dependencies
struct dependencyImaps {
	const Credentials m_credentials;
	std::chrono::seconds c_connectTimeout_sec;
};
struct dependencyEmailCount {
	setActiveTip_t fnSetActiveTip;
	showBalloon_t fnShowBalloon;
	unsigned long m_currentEmailCount;
	unsigned long m_lastEmailCount;

	std::string lastFrom;
	std::string lastSubject;
};
struct dependencyBreakableSleep {
	std::chrono::seconds c_RecheckInterval_sec;
	std::condition_variable *m_pConditionVariable;
	std::mutex *m_pcvMutex;
	bool m_bBreak;
};
struct dependencyAvailable {
	bool m_bAvailable;
};

namespace Action {
struct Imap { void operator()(dependencyImaps &imaps, dependencyEmailCount& counter) {
	std::cout << "Imap()" << std::endl;

	std::cout << "Imap_connect()" << std::endl; 
	std::unique_ptr< mailio::imaps > pmailio;
	pmailio = std::make_unique< mailio::imaps >( 
			imaps.m_credentials.host
			, imaps.m_credentials.port
			, imaps.c_connectTimeout_sec
		);

	std::cout << "Imap_authz()" << std::endl; 
	pmailio ->authenticate(
			imaps.m_credentials.login
			, imaps.m_credentials.password
			, mailio::imaps::auth_method_t::LOGIN
		);

	std::cout << "Imap_fetch()" << std::endl;
	const char mailbox[] = "inbox";
	// @insp https://github.com/jkoomjian/MyEmailResponseRate/issues/2
	const auto stat_ = pmailio ->select( mailbox );

	std::list< mailio::imap::search_condition_t > search_condition;
	// UNSEEN exclude ALL
	search_condition.emplace_back( mailio::imap::search_condition_t::UNSEEN );
	std::list<unsigned long> results;
	pmailio ->search( search_condition, results );
	unsigned long lastSequenceNumber = results.back( );
	std::cout << "lastSequenceNumber = " << lastSequenceNumber << std::endl;

	// To known sender and subject
	mailio::message lastMessage;
	bool header_only;
	pmailio ->fetch( mailbox, lastSequenceNumber, lastMessage, header_only = true );
	pmailio.reset( );

	std::stringstream ss;
	ss << lastMessage.from_to_string( ) << std::endl;
	ss << lastMessage.subject( ) << std::endl;
	std::cout << "lastMessage.from_to_string( ): " << lastMessage.from_to_string( ) << std::endl;
	std::cout << "lastMessage.subject( ): " << lastMessage.subject( ) << std::endl;

	counter.m_currentEmailCount = lastSequenceNumber;
	counter.lastFrom = lastMessage.from_to_string( );
	counter.lastSubject = lastMessage.subject( );

} } Imap;

struct BreakableSleep { void operator()(dependencyBreakableSleep &dependency) {

	std::cout << "Beg wait" << std::endl; 
	std::unique_lock< std::mutex > lock( *dependency.m_pcvMutex );
	std::cv_status finishedWaiting = dependency.m_pConditionVariable ->wait_until( 
			lock
			, std::chrono::system_clock::now( ) + dependency.c_RecheckInterval_sec 
		);
	std::cout << "End wait" << std::endl; 
	if ( std::cv_status::no_timeout == finishedWaiting )
		dependency.m_bBreak = true;

} } BreakableSleep;

struct InitialEmailCount { void operator()(dependencyEmailCount& dependency) {
	std::cout << "m_lastEmailCount = m_currentEmailCount" << std::endl;
	dependency.m_lastEmailCount = dependency.m_currentEmailCount;
} } InitialEmailCount;

struct EmailCounter { void operator()(dependencyEmailCount& dependency) {
	if ( dependency.m_currentEmailCount <= dependency.m_lastEmailCount )
		return;
	dependency.m_lastEmailCount = dependency.m_currentEmailCount;

	dependency.fnSetActiveTip( std::string{ }
			+ "SmallImapsNotifier: last email\n"
			+ "from: '" + dependency.lastFrom + "'\n"
			+ "subject: '" + dependency.lastSubject + "'\n"
		);
	dependency.fnShowBalloon( dependency.lastFrom, dependency.lastSubject );

} } EmailCounter;

struct InboxAvailable { void operator()(dependencyAvailable& dependency) {
	dependency.m_bAvailable = true;
} } InboxAvailable;

} // namespace Action

struct guardAvailable { 
	bool operator()(dependencyAvailable& dependency) const { 
		std::cout << "guardAvailable dependency.m_bAvailable: " << (dependency.m_bAvailable ?"TRUE" :"FALSE" )<< std::endl;
		return !dependency.m_bAvailable;
	}
} guardAvailable;
struct guardBreak { 
	bool operator()(dependencyBreakableSleep &dependency) const { 
		std::cout << "guardBreak dependency.m_bBreak: " << (dependency.m_bBreak ?"TRUE" :"FALSE" )<< std::endl;
		return dependency.m_bBreak;
	}
} guardBreak;

struct ImapsSm {
  auto operator()() const noexcept {
    using namespace boost::sml;
    using namespace Action;
    return make_transition_table(
		// Using `state<>` to avoid anon in MSVC
		*state<struct idle> + event<CheckAccess> / Imap = "TryInbox"_s

		, "TryInbox"_s + event<Unavailable> [guardAvailable] = X
		, "TryInbox"_s + event<StartLoop> / InitialEmailCount = "Waiting"_s
		, "TryInbox"_s + on_entry<_> / InboxAvailable

		, "Waiting"_s + event<HasBreak> [guardBreak] = X
		, "Waiting"_s + event<RenewEmail> / Imap= "RenewedEmail"_s
		, "Waiting"_s + on_entry<_> / BreakableSleep

		, "RenewedEmail"_s + event<Unavailable> [guardAvailable] = X
		, "RenewedEmail"_s + event<ContinueLoop> = "Waiting"_s
		, "RenewedEmail"_s + on_entry<_> / ( InboxAvailable, EmailCounter )

		, *("exceptions handling"_s) + exception<_> / [](dependencyAvailable& dependency) { dependency.m_bAvailable = false; } = X
    );
  }
};

} // namespace prj_sysw::SmallImapsNotifier::OsiApplication::Imaps::StateMachine
