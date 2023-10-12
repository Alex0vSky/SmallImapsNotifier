// src\OsiApplication\Imaps\StateMachine\Helper.h - IMAP State Machine dumper and logger
#pragma once // Copyright 2023 Alex0vSky (https://github.com/Alex0vSky)
namespace prj_sysw::SmallImapsNotifier::OsiApplication::Imaps::StateMachine::Helper {
namespace sml = boost::sml;
// trim from start (in place)
static inline void ltrim(std::string &s, char Char = '<') {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [&Char](unsigned char ch) {
        return ( Char != ch );
    }));
}
//	trim from end (in place)
static inline void rtrim(std::string &s, char Char = '>') {
    s.erase(std::find_if(s.rbegin(), s.rend(), [&Char](unsigned char ch) {
        return ( Char != ch );
    }).base(), s.end());
}
// trim from both ends (in place)
static inline void trim(std::string &s) {
    rtrim(s);
    ltrim(s);
}
// @insp https://stackoverflow.com/questions/216823/how-to-trim-an-stdstring
std::string cut_last(const std::string& str) {
	std::string out = str;
	std::size_t found = str.rfind( "::" );
    if ( std::string::npos != found ) {
		found += 2;
		out = str.substr( found, str.length( ) - found );
		trim( out );
	}
	return out;
}

// uses std::tuple_element and std::tuple to access the Nth type in a parameter pack
template <int N, class... Ts>
using NthTypeOf = typename std::tuple_element<N, std::tuple<Ts...>>::type;

// if T is a zero wrapper, ::type will be the inner type. if not, it will be T.
template <class T>
struct strip_zero_wrapper {
  using type = T;
};  // NOLINT(readability-identifier-naming)
template <class T>
struct strip_zero_wrapper<sml::aux::zero_wrapper<T>> {
  using type = T;
};  // NOLINT(readability-identifier-naming)

// allows for checking if the type is sml::aux::zero_wrapper
// sml puts this around types inside of guards and event sequences
template <class... Ts>
struct is_seq_ : sml::aux::false_type {};  // NOLINT(readability-identifier-naming)
template <class... Ts>
struct is_seq_<sml::front::seq_<Ts...>> : sml::aux::true_type {};  // NOLINT(readability-identifier-naming)
template <class... Ts>

// print the types inside a sml::front::seq_
// These types came from a list of actions.
struct print_seq_types {  // NOLINT(readability-identifier-naming)
  template <int I>
  static void func(std::ostream& out) {
    constexpr auto param_pack_empty = (sizeof...(Ts) == I);
    if constexpr (!param_pack_empty) {  // NOLINT(readability-braces-around-statements,bugprone-suspicious-semicolon)
      using current_type = NthTypeOf<I, Ts...>;
      if constexpr (is_seq_<typename current_type::type>::value) {  // NOLINT(readability-braces-around-statements)
        print_seq_types<typename current_type::type>::template func<0>(out);
      } else {  // NOLINT(readability-misleading-indentation)
        std::string str = sml::aux::string<typename strip_zero_wrapper<current_type>::type>{ }.c_str( );
        str = cut_last( str );
        out << str;
      }
      if constexpr (I + 1 < sizeof...(Ts)) {  // NOLINT(readability-braces-around-statements,bugprone-suspicious-semicolon)
        out << ", ";
      }
      print_seq_types<Ts...>::template func<I + 1>(out);
    }
  }
};
template <class... Ts>
struct print_seq_types<sml::front::seq_<Ts...>> {  // NOLINT(readability-identifier-naming)
  template <int I>
  static void func(std::ostream& out) {
    print_seq_types<Ts...>::template func<0>(out);
  }
};

template <class T>
void dump_transition(std::ostream& out) noexcept {
  auto src_state = cut_last( std::string{sml::aux::string<typename T::src_state>{}.c_str()} );
  auto dst_state = cut_last( std::string{sml::aux::string<typename T::dst_state>{}.c_str()} );
  if (dst_state == "X") {
    dst_state = "[*]";
  }

  if (T::initial) {
    out << "[*] --> " << src_state << "\n";
  }

  const auto has_event = !sml::aux::is_same<typename T::event, sml::anonymous>::value;
  const auto has_guard = !sml::aux::is_same<typename T::guard, sml::front::always>::value;
  const auto has_action = !sml::aux::is_same<typename T::action, sml::front::none>::value;

  const auto is_entry = sml::aux::is_same<typename T::event, sml::back::on_entry<sml::_, sml::_>>::value;
  const auto is_exit = sml::aux::is_same<typename T::event, sml::back::on_exit<sml::_, sml::_>>::value;

  if (is_entry || is_exit) {
    out << src_state;
  } else {  // state to state transition
    out << src_state << " --> " << dst_state;
  }

  if (has_event || has_guard || has_action) {
    out << " :";
  }

  if (has_event) {
    auto str = std::string(boost::sml::aux::get_type_name<typename T::event>());
    auto event = cut_last( str );
    if (is_entry) {
      event = "entry";
    } else if (is_exit) {
      event = "exit";
    }
    out << " " << event;
  }

  if (has_guard) {
    out << " [" << cut_last( boost::sml::aux::get_type_name<typename T::guard::type>() ) << "]";
  }

  if (has_action) {
    out << " / ";
    if constexpr (is_seq_<typename T::action::type>::value) {  // NOLINT(readability-braces-around-statements)
      out << "(";
      print_seq_types<typename T::action::type>::template func<0>(out);
      out << ")";
    } else {  // NOLINT(readability-misleading-indentation)
      auto str = std::string( boost::sml::aux::get_type_name<typename T::action::type>() );
      out << cut_last( str );
    }
  }

  out << "\n";
}
template <template <class...> class T, class... Ts>
void dump_transitions(const T<Ts...>&, std::ostream& out) noexcept {
  int _[]{0, (dump_transition<Ts>(out), 0)...};
  (void)_;
}
template <class SM>
void dump(const SM&, std::ostream& out) noexcept {
  out << "@startuml\n\n";
  dump_transitions(typename SM::transitions{}, out);
  out << "\n@enduml\n";
}

struct my_logger {
  template <class SM, class TEvent>
  void log_process_event(const TEvent& event_v) {

	std::string strEvent = sml::aux::get_type_name<TEvent>();
	// skip 
//_1_9::aux::get_type_name<struct boost::ext::sml::v1_1_9::back::on_entry<struct boost::ext::sml::v1_1_9::back::_,struct Xxx>
    if ( std::string::npos != strEvent.rfind( "back::on_entry<" ) ) 
		return;

	strEvent = cut_last( strEvent );
    printf(
			"[%s][process_event] %s\n"
			, cut_last( sml::aux::get_type_name<SM>() ).c_str( )
			, strEvent.c_str( ) 
		);
  }

  template <class SM, class TGuard, class TEvent>
  void log_guard(const TGuard&, const TEvent&, bool result) {
    printf("[%s][guard] %s %s %s\n"
			, cut_last( sml::aux::get_type_name<SM>() ).c_str( )
			, cut_last( sml::aux::get_type_name<TGuard>() ).c_str( )
			, cut_last( sml::aux::get_type_name<TEvent>() ).c_str( )
			, (result ? "[OK]" : "[Reject]")
		);
  }

  template <class SM, class TAction, class TEvent>
  void log_action(const TAction&, const TEvent&) {
    //printf("[%s][action] %s %s\n"
    printf("[%s][action] %s\n"
			, cut_last( sml::aux::get_type_name<SM>() ).c_str( )
			, cut_last( sml::aux::get_type_name<TAction>() ).c_str( )
			////, sml::aux::get_type_name<TAction>()
			//, cut_last( sml::aux::get_type_name<TEvent>() ).c_str( )
		);
  }

  template <class SM, class TSrcState, class TDstState>
  void log_state_change(const TSrcState& src, const TDstState& dst) {
    printf(
			"[%s][transition] %s -> %s\n"
			, cut_last( sml::aux::get_type_name<SM>() ).c_str( )
			, cut_last( src.c_str() ).c_str( )
			, cut_last( dst.c_str() ).c_str( )
		);
  }
};
} // namespace prj_sysw::SmallImapsNotifier::OsiApplication::Imaps::StateMachine::Helper _
