// src\OsiApplication\Imaps\Thread.h - IMAP over ssl, in separate thread
#pragma once // Copyright 2023 Alex0vSky (https://github.com/Alex0vSky)
namespace prj_sysw::SmallImapsNotifier::OsiApplication::Imaps {
class Thread {
	static constexpr std::chrono::seconds c_ReconnectTry_sec{ 60 };
	static constexpr std::chrono::seconds c_ConnectTimeout_sec{ 30 };
	static constexpr std::chrono::minutes c_RecheckInterval_minutes{ 5 };

	Credentials m_credentials;
	StateMachine::setActiveTip_t m_clbSetActiveTip;
	StateMachine::showBalloon_t m_clbShowBalloon;

	std::condition_variable m_cv;
	std::mutex m_cvMutex;
	std::atomic_bool m_bStop;
	std::thread m_thread;

	bool breakableWaitReconnect_() {
		std::lock_guard lock(m_cvMutex);
		std::cv_status waiting = m_cv.wait_for( lock, c_ReconnectTry_sec, []{
				return m_bStop;
			});
		return std::cv_status::timeout == waiting;
	}

	// thread
	void run() {
#ifdef _DEBUG
		::SetThreadDescription( ::GetCurrentThread( ), L"NetworkLoop" );
#endif // _DEBUG

		namespace sml = boost::sml;
		using namespace StateMachine;
		do {
			dependencyImaps imaps = { m_credentials, c_ConnectTimeout_sec };
			dependencyEmailCount counter = { m_clbSetActiveTip, m_clbShowBalloon };
			dependencyBreakableSleep sleep = { 
					c_RecheckInterval_minutes
					//std::chrono::seconds{ 10 } // tmp
					, &m_cv
					, &m_cvMutex
				};
			sml::sm< ImapsSm > sm{ imaps, counter, sleep };
			//Helper::my_logger logger; sml::sm< ImapsSm, sml::logger< Helper::my_logger > > sm{ logger, imaps, counter, notifier, sleep };
			//std::stringstream ss; Helper::dump( sm, ss ); Tool::ErrorHandler::log( ss.str( ).c_str( ) );
			sm.process_event( CheckAccess{ } );
			sm.process_event( Inavailable{ } );
			// tmp
			if ( m_bWaitingCheck ) {
				m_bWaitingCheck = false;
				m_bCheckSucc = !sm.is( sml::X );
			}
			sm.process_event( StartLoop{ } );
			while ( !sm.is( sml::X ) ) {
				sm.process_event( HasBreak{ } );
				sm.process_event( RenewEmail{ } );
				sm.process_event( Inavailable{ } );
				sm.process_event( ContinueLoop{ } );
			}
		} while ( breakableWaitReconnect_( ) );
	}
//
 public:
	Thread(
		const Credentials &credentials
		, StateMachine::setActiveTip_t clbSetActiveTip
		, StateMachine::showBalloon_t clbShowBalloon
	)
		: m_credentials( credentials )
		, m_clbSetActiveTip( clbSetActiveTip )
		, m_clbShowBalloon( clbShowBalloon )
		, m_bStop( false )
	 {
		// will be init in last order
		m_thread = { &Thread::run, this };
	}

	// tmp
	std::atomic_bool m_bWaitingCheck = true;
	std::atomic_bool m_bCheckSucc = false;

	bool ñredentialsEqual(const Credentials &credentials) {
		return ( credentials == m_credentials );
	}

	void stop() {
		{
			std::lock_guard lock( m_cvMutex );
			m_bStop = true;
		}
		m_cv.notify_all( );
		m_thread.join( );
	}

	// Noncopyable
	Thread(const Thread&) = delete;
	Thread& operator=(const Thread&) = delete;
};
} // namespace prj_sysw::SmallImapsNotifier::OsiApplication::Imaps _
