/*
 * Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * network_esp8266.hpp - session layer implementation for ESP8266
 *
 * This file is part of µcuREST Library. http://hutorny.in.ua/projects/micurest
 *
 * The µcuREST Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License v2
 * as published by the Free Software Foundation;
 *
 * The µcuREST Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License v2
 * along with the COJSON Library; if not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>.
 */
#ifndef NETWORK_ARDUINO_HPP_
#define	NETWORK_ARDUINO_HPP_
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <utility/socket.h>
#pragma GCC diagnostic pop
#undef F
#undef bit
#undef abs
#undef max
#undef min
#ifndef NETWORK_HPP_
#	include "network.hpp"
#endif

namespace micurest {
namespace network_arduino {
typedef SOCKET handle_t;
using namespace micurest::network;
using cojson::details::ostream;
using cojson::details::istream;

/*****************************************************************************/
namespace proto {

class tcp {
public:
	static constexpr handle_t badhandle = -1;
	inline tcp(port_t aport) noexcept
	  : port(aport), next(0), accepted(badhandle) {}
	struct connection /*: network::connection*/ {
		void close() noexcept;
		void keep() noexcept {}
		inline istream& input() noexcept { return buff; }
		inline ostream& output() noexcept { return buff; }
		connection(const tcp& socket) noexcept;
	private:
		size_t send(const void*, size_t) const noexcept;
		size_t receive(void*, size_t) const noexcept;
		size_t available() const noexcept;
		struct buffer : istream, ostream {
			bool get(char_t& val) noexcept;
			bool put(char_t val) noexcept;
			bool flush() noexcept;
			static constexpr size_t size = 256; //TODO configurable
			inline buffer(const connection& con) noexcept
				: conn(con), getpos(data), endpos(data), putpos(data) { }
			const connection& conn;
			const char * getpos;
			const char * endpos;
			char * putpos;
			static char data[size]; //TODO configurable static
		};
		const handle_t handle;
		buffer buff;
		friend class buffer;
	};
	bool accept() noexcept;
	bool listen() noexcept;
	void stop() noexcept;
private:
	static bool listen(handle_t, port_t) noexcept;

	port_t port;
	uint_fast8_t next;
	handle_t accepted;
	friend class tcp::connection;
};
}}}
#endif
