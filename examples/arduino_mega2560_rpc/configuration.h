#pragma once
#include "miculog.ccs"
#include "access_log.ccs"
#include "network_arduino.ccs"

namespace configuration {
struct Current : Is<target::All, build::Default> {};
template<> struct Selector<micurest::network_arduino::tcp::server> : Current {};
template<> struct Selector<micurest::network::access_log> : Current {};
}

namespace miculog {

template<> struct ClassLogLevels<
	micurest::network_arduino::tcp::server,
	configuration::build::Default> :
	From<level::warn> {};

template<> struct ClassLogLevels<
	micurest::network_arduino::tcp::server,
	configuration::build::Debug> :
	Levels<level::debug, level::info, level::error> {};

template<> struct ClassLogLevels<
	micurest::network::access_log,
	configuration::build::Default> :
	Levels<> {};

template<> struct ClassLogLevels<
	micurest::network::access_log,
	configuration::build::Debug> :
	Levels<level::info> {};
}
