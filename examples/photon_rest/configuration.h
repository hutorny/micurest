#pragma once
#include "micurest/micurest.ccs"
#include "micurest/miculog.ccs"
#include "micurest/network_log.ccs"
#include "micurest/network_spark_socket.ccs"
#include "micurest/miculog_spark_usart_hal.ccs"

namespace configuration {
namespace target { struct Photon; }
struct Current : Is<target::Photon, build::Default> {};
template<> struct Selector<micurest::network_spark_socket::tcp::tcp> : Current {};
template<> struct Selector<micurest::network::access_log> : Current {};
template<> struct Selector<miculog::details::default_appender> : Current {};

template<typename Build>
struct Configuration<miculog::details::default_appender, target::Photon, Build> {
	static constexpr bool  blocking = true;
	static constexpr short serial = 1;
};
}

namespace miculog {
template<> struct ClassLogLevels<
	micurest::network_spark_socket::tcp::tcp,
	configuration::build::Default> :
	From<level::warn> {};
}
