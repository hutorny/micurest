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
#pragma once
#include <configuration.h>
#include "network_arduino.ccs"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <utility/socket.h>
#pragma GCC diagnostic pop
#undef F
#undef bit
#undef abs
#undef max
#undef min
#include "micurest.hpp"

namespace micurest {
namespace network_arduino {
typedef SOCKET handle_t;
using namespace micurest::network;
using cojson::details::ostream;
using cojson::details::istream;

/*****************************************************************************/
namespace tcp {

class server {
public:
	using config = configuration::Configuration<server>;
	static constexpr handle_t badhandle = -1;
	static constexpr auto buffer_static = config::buffer_storage_is_static;
	inline server(const application& ap) noexcept
	  : app(ap), port(0), next(0), accepted(badhandle) {}
	struct session /*: network::connection*/ {
		void close() noexcept;
		void keep() noexcept {}
		inline istream& input() noexcept { return buff; }
		inline ostream& output() noexcept { return buff; }
		session(const server& socket) noexcept;
	private:
		size_t send(const void*, size_t) const noexcept;
		size_t receive(void*, size_t) const noexcept;
		size_t available() const noexcept;
		struct buffer : istream, ostream {
			bool get(char_t& val) noexcept;
			bool put(char_t val) noexcept;
			bool flush() noexcept;
			static constexpr size_t size = config::buffer_size;
			inline buffer(const session& con) noexcept
				: conn(con), getpos(data), endpos(data), putpos(data) { }
			const session& conn;
			const char * getpos;
			const char * endpos;
			char * putpos;
			cojson::details::temporary_s<char,
				config::buffer_size, buffer_static> data;
		};
		const handle_t handle;
		buffer buff;
		byte remote_ip[4];
		friend class buffer;
	};
	bool listen(port_t) noexcept;
	void stop() noexcept;
	void run() noexcept;
protected:
	bool accept() noexcept;
private:
	static bool listen(handle_t, port_t) noexcept;
	const application& app;
	port_t port;
	uint_fast8_t next;
	handle_t accepted;
	friend class server::session;
};
}}}
