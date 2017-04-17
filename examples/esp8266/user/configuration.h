#pragma once

#include "cojson.ccs"
#include "miculog.ccs"
#include "micurest.ccs"
#include "access_log.ccs"
#include "network_esp8266.ccs"

/************************************************************************/
/*							configuration selectors						*/
/************************************************************************/
namespace configuration {
namespace target { struct ESP8266; }

/* Current configuration set is target ESP8266, build Default			*/
struct Current : Is<target::ESP8266,build::Debug> {};

/* Configurable classes are on Current configuration set				*/
template<> struct Selector<micurest::network_esp::tcp::server>	: Current {};
template<> struct Selector<micurest::network::access_log>		: Current {};
template<> struct Selector<cojson::config>						: Current {};
template<> struct Selector<cojson::details::lexer>				: Current {};
template<> struct Selector<micurest::config>					: Current {};

/************************************************************************/
/*							cojson configuration						*/
/************************************************************************/
template<typename Build>
struct Configuration<cojson::config, target::ESP8266, Build> :
	Configuration<cojson::config, target::All, Build> {
	static constexpr auto iostate   = cojson::default_config::iostate_is::_virtual;
	static constexpr bool sprintf_buffer_static = true;
};

template<typename Build>
struct Configuration<cojson::details::lexer, target::ESP8266, Build> :
	Configuration<cojson::details::lexer, target::All, Build> {
	static constexpr auto temporary_static = true;
};
}

/************************************************************************/
/*							micurest configuration						*/
/************************************************************************/

/* nothing configured here 												*/

/************************************************************************/
/*							miculog configuration						*/
/************************************************************************/

namespace miculog {

template<> struct ClassLogLevels<
	micurest::network_esp::tcp::server,
	configuration::build::Default> :
	From<level::warn> {};

template<> struct ClassLogLevels<
	micurest::network::access_log,
	configuration::build::Default> :
	Levels<> {};
}

