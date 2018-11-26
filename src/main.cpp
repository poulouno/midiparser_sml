#include <iostream>
#include <array>
#include <algorithm>
#include "sml.hpp"
#include "midi_event.h"

namespace sml = boost::sml;
namespace{
template <class R, class... Ts>
auto call_impl(R (*f)(Ts...)) {
  return [f](Ts... args) { return f(args...); };
}
template <class T, class R, class... Ts>
auto call_impl(T* self, R (T::*f)(Ts...)) {
  return [self, f](Ts... args) { return (self->*f)(args...); };
}
template <class T, class R, class... Ts>
auto call_impl(const T* self, R (T::*f)(Ts...) const) {
  return [self, f](Ts... args) { return (self->*f)(args...); };
}
template <class T, class R, class... Ts>
auto call_impl(const T* self, R (T::*f)(Ts...)) {
  return [self, f](Ts... args) { return (self->*f)(args...); };
}
/**
 * Simple wrapper to call free/member functions
 * @param args function, [optional] this
 * @return function(args...)
 */
auto call = [](auto... args) { return call_impl(args...); };
 
    struct my_logger {
  template <class SM, class TEvent>
  void log_process_event(const TEvent&) {
    printf("[%s][process_event] %s\n", sml::aux::get_type_name<SM>(), sml::aux::get_type_name<TEvent>());
  }

  template <class SM, class TGuard, class TEvent>
  void log_guard(const TGuard&, const TEvent&, bool result) {
    printf("[%s][guard] %s %s %s\n", sml::aux::get_type_name<SM>(), sml::aux::get_type_name<TGuard>(),
           sml::aux::get_type_name<TEvent>(), (result ? "[OK]" : "[Reject]"));
  }

  template <class SM, class TAction, class TEvent>
  void log_action(const TAction&, const TEvent&) {
    printf("[%s][action] %s %s\n", sml::aux::get_type_name<SM>(), sml::aux::get_type_name<TAction>(),
           sml::aux::get_type_name<TEvent>());
  }

  template <class SM, class TSrcState, class TDstState>
  void log_state_change(const TSrcState& src, const TDstState& dst) {
    printf("[%s][transition] %s -> %s\n", sml::aux::get_type_name<SM>(), src.c_str(), dst.c_str());
  }
};
   
  
	// GUARDS
	const auto is_rt = [](const MidiByte e) { return e.isRT(); };
	const auto is_data = [](const MidiByte e) { return e.isData(); };
	const auto is_status = [](const MidiByte& e) { return e.isStatus(); };
	const auto has_status = [](MidiEvent& m) { return m.mStatus != MidiEvent::kMidiReset; };

	const auto expected = [](const int len, const MidiByte& e, MidiEvent& m) {
		if (e.isStatus())
			return MidiEvent::getLength(e.mByte) == len;
		else if (e.isData())
			return m.getLength() == len;
		else
			return false;
	};

	const auto expected_2 = [](const MidiByte& e, MidiEvent& m) { return expected(3, e, m); };
	const auto expected_1 = [](const MidiByte& e, MidiEvent& m) { return expected(2, e, m); };
	const auto expected_0 = [](const MidiByte& e, MidiEvent& m) { return expected(1, e, m); };
	
	//ACTIONS
	const auto print_midievent = [](MidiEvent& m) {printf("\t: %x %x %x\n", m.mStatus, m.mData1, m.mData2); };
	const auto MidiCB = [](MidiEvent& m) {std::cout << "MidiCB!" << std::endl; print_midievent(m); };
	const auto RTCB = [](const MidiByte& e) {std::cout << "RTCB!" << std::endl; MidiEvent m = { e.mByte }; print_midievent(m); };
	const auto SetStatus = [](const MidiByte& e, MidiEvent& m) {m.mStatus = e.mByte; std::cout << "status!" << std::endl; };
	const auto SetByte1 = [](const MidiByte& e, MidiEvent& m) {m.mData1 = e.mByte; std::cout << "byte1!" << std::endl; };
	const auto SetByte2 = [](const MidiByte& e, MidiEvent& m) {m.mData2 = e.mByte; std::cout << "byte2!" << std::endl; };
    const auto set_status = [] {};

    
    //const auto  tata() {m.mStatus++;};
    struct midi_parser_sm {       
       
        auto operator()() const {   
			using namespace sml;
			return make_transition_table(
				state<class idle>(H)  = state<class status>,
				state<class status>   + event<MidiByte>     [is_status && (expected_2 || expected_1)] / SetStatus	= state<class Byte1>,
				state<class Byte1>    + event<MidiByte>     [is_data && expected_1] / (SetByte1, MidiCB)			= state<class status>,
				state<class Byte1>    +event<MidiByte>		[is_data && expected_2] / SetByte1						= state<class Byte2>,
				state<class Byte2>   + event<MidiByte>     [is_data] / (SetByte2, MidiCB)							= state<class status>,
				//running status
				state<class status>   +event<MidiByte>[is_data && has_status && expected_1] / (SetByte1, MidiCB) = state<class status>,
				state<class status>   +event<MidiByte>[is_data && has_status && expected_2] / (SetByte1) = state<class Byte2>
				);
		}
	};  
	struct Top_sm {

		auto operator()() const {
			using namespace sml;
			return make_transition_table(
				*state<class idle> = state<class midi_parser_sm>,
				state<class midi_parser_sm>   +event<MidiByte>[is_rt] / RTCB = state<class midi_parser_sm>
			);
		}
	};
}

int main()
{
    using namespace sml;
    my_logger logger;
	MidiEvent m;
    //sm<midi_parser_sm, sml::logger<my_logger>> midi_sm{logger, m};
	sm<Top_sm> midi_sm{ m };

	// 2 data msg
    std::array<MidiByte, 3> msg={MidiByte(0x90), MidiByte(0x40), MidiByte(0x7F)};
    for(size_t i=0; i<msg.size(); i++){
        midi_sm.process_event(msg[i]);
    }
	//two data with running status
	midi_sm.process_event(MidiByte(0x10));
	midi_sm.process_event(MidiByte(0x20));
	// 1 data msg
	midi_sm.process_event(MidiByte(0xD0));
	midi_sm.process_event(MidiByte(0x40));

	// 1 data msg with running status
	midi_sm.process_event(MidiByte(0x50));

	// 1 data msg with RT inserted
	midi_sm.process_event(MidiByte(0xD1));
	midi_sm.process_event(MidiByte(0xF8));
	midi_sm.process_event(MidiByte(0x60));
    //std::for_each(msg.begin(), msg.end(), [midi_sm](MidiByte& e){midi_sm.process_event(e)});
}
