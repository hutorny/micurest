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
#undef F
#undef bit
#undef abs
#undef max
#undef min
#undef getc
#undef putc
#include <configuration.h>
#include "network_spark_socket.ccs"
#include <functional>
#include "micurest.hpp"
#include "socket_hal.h"
#include "timer_hal.h"

namespace micurest {
namespace network_spark_socket {
typedef sock_handle_t handle_t;
using cojson::details::ostream;
using cojson::details::istream;

namespace details {

template<typename DataType, uint_fast8_t Capacity, typename TimeType, TimeType Now()>
class timedqueue {
	typedef uint_fast8_t _size_t;
	struct item {
		DataType data;
		TimeType time;
	};
	item items[Capacity] = {};
	_size_t length = 0;
	static inline constexpr bool expired(
			TimeType from, TimeType now, TimeType timeout) noexcept {
		return now >= from + timeout;
	}
public:
	/* places data at the end of queue 										*/
	inline bool push(DataType data) noexcept {
		if( length >= Capacity ) return false;
		items[length++] = { data, Now() };
		return true;
	}
	/* iterates through all elements, removes dead and expired				*/
	void validate(std::function<bool(DataType&,bool) noexcept> keep,
			TimeType timeout) noexcept {
		_size_t newtail = 0;
		TimeType now = Now();
		for(_size_t i = 0; i < length; ++i) {
			if( keep(items[i].data,
					expired(items[i].time, now, timeout)) ) {
				if( newtail == i ) newtail++;
				else items[newtail++] = items[i];
			};
		}
		length = newtail;
	}
	void replace(DataType value, DataType newval) noexcept {
		for(auto& i : items)
			if( i.data == value )
				i.data = newval;
	}
	size_t len() const noexcept { return length; }
};

}

/*****************************************************************************/
namespace tcp {

class server {
public:
	using config = configuration::Configuration<server>;
	static constexpr uint32_t inactivity_timeout = config::inactivity_timeout;
	static constexpr uint8_t max_connections = config::max_connections;
	static constexpr uint8_t max_sessions = config::max_sessions;
	static constexpr size_t buffer_size = config::buffer_size;
	static constexpr auto buffer_static = config::buffer_static;
	static const handle_t invalid_handle;
	typedef details::timedqueue<handle_t, max_connections, system_tick_t,
			HAL_Timer_Get_Milli_Seconds> timedqueue;
	server(application& _app, network_interface_t _nif = 0) noexcept;
protected:
	struct session_state : micurest::details::httpmessage::state_pdo {
		handle_t handle;
		inline void clear() noexcept {
			micurest::details::httpmessage::state_pdo::clear();
			handle = invalid_handle;
		}
	};
public:
	struct session {
		void close() noexcept;
		inline void keep() noexcept {
			buff.flush();
			if( ! socket.keep(handle) ) close();
		}
		inline istream& input() noexcept { return buff; }
		inline ostream& output() noexcept { return buff; }
		session(server&, session_state*, handle_t, char (&data)[buffer_size],
				size_t) noexcept;
		inline session_state* getstate() const noexcept { return state; }
	private:
		inline bool isgood() const noexcept;
		size_t send(const void*, size_t) const noexcept;
		size_t receive(void*, size_t) const noexcept;
		struct buffer : istream, ostream {
			bool get(char_t& val) noexcept;
			bool put(char_t val) noexcept;
			bool flush() noexcept;
			sock_result_t sync() noexcept;
			inline buffer(const session& con, char (&dat)[buffer_size],
					size_t len) noexcept
			  : conn(con),data(dat),getpos(dat),endpos(dat+len),putpos(dat) {}
			const session& conn;
			char * data;
			const char * getpos;
			const char * endpos;
			static constexpr const size_t size = buffer_size;
			char * putpos;
		};
		server& socket;
		session_state* state;
		handle_t handle;
		buffer buff;
		sock_peer_t peer;
		friend class buffer;
	};
	inline bool listen(port_t port) noexcept {
		return listen(port, nif);
	}
	bool listen(port_t, network_interface_t) noexcept;
	void run() noexcept;
	void stop() noexcept;
protected:
	bool ready() const noexcept;
	session_state* find_session(handle_t handle) noexcept;
	bool keep(handle_t) noexcept;
	void close(handle_t) noexcept;
	handle_t accept(char (&buffer)[buffer_size], size_t& size) noexcept;
	void kill_state_for(handle_t) noexcept;
	void dbg_queuelen() noexcept;
private:
	application& app;
	port_t port;
	network_interface_t nif;
	handle_t socket;
	timedqueue queue;
	session_state states[max_sessions] = {};
	friend class server::session;
};
}}}
