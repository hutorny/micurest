#pragma once
#include "micurest/micurest.ccs"
#include "micurest/miculog.ccs"
#include "micurest/access_log.ccs"
#include "micurest/network_spark_socket.ccs"
#include "micurest/miculog_spark_usart_hal.ccs"

namespace configuration {
namespace target { struct Photon; }
struct Current : Is<target::Photon, build::Default> {};
template<> struct Selector<micurest::network_spark_socket::tcp::server> : Current {};
template<> struct Selector<micurest::network::access_log> : Current {};
template<> struct Selector<miculog::details::default_appender> : Current {};
}

namespace miculog {
template<> struct ClassLogLevels<
	micurest::network_spark_socket::tcp::server,
	configuration::build::Default> :
	From<level::warn> {};
}
