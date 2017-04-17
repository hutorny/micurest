/*
 * Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * network_spark_socket.cpp - session layer implementation for Particle
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
 * along with the µcuREST Library; if not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>.
 */

#include "access_log.hpp"
#include "network_spark_socket.hpp"
#include "system_network.h"
#include <cstdio>
#include <cstdarg>
#include <application.h>

namespace micurest {
namespace network {
	template<>
	void access_log::_log<HAL_IPAddress>(const char* lbl,
			const HAL_IPAddress& addr, char dlm) noexcept {
		if( enabled() ) {
			if( addr.v != 6 )
				appender::log(level(-1), "%-8s %d.%d.%d.%d %c", lbl,
						(int)(addr.ipv4 >> 24) & 0xFF,
						(int)(addr.ipv4 >> 16) & 0xFF,
						(int)(addr.ipv4 >>  8) & 0xFF,
						(int)(addr.ipv4 >>  0) & 0xFF, dlm);
#		if HAL_IPv6
			else
				info("%8:s %x:%x:%x:%x:%x:%x:%x:%x %c", lbl,
						(addr.ipv6[0] >> 16) & 0xFFFF,
						(addr.ipv6[0] >>  0) & 0xFFFF,
						(addr.ipv6[1] >> 16) & 0xFFFF,
						(addr.ipv6[1] >>  0) & 0xFFFF,
						(addr.ipv6[2] >> 16) & 0xFFFF,
						(addr.ipv6[2] >>  0) & 0xFFFF,
						(addr.ipv6[3] >> 16) & 0xFFFF,
						(addr.ipv6[3] >>  0) & 0xFFFF, dlm);
#		endif
		}
}
}

using session = network_spark_socket::tcp::server::session;
const miculog::Log<application> log;

template<>
void application::service<session>(session& con) const noexcept {
	typedef application::result_t result_t;

	istream& in(con.input());
	ostream& out(con.output());
	result_t res = service(in, out, con.getstate());
	if( res == result_t::keep || res == result_t::fragment )
		con.keep();
	else
		con.close();
}

namespace network_spark_socket {
/******************************************************************************/

const network::access_log access;
const miculog::Log<micurest::network_spark_socket::tcp::server> log;

/******************************************************************************/
namespace tcp {

server::server(application& _app, network_interface_t _nif) noexcept
  : app(_app), port(0), nif(_nif), socket(socket_handle_invalid()) {}

inline bool server::ready() const noexcept {
	return network_ready(nif, 0, nullptr);
}

bool server::listen(port_t _port, network_interface_t _nif) noexcept {
	if( _port == port && _nif == nif &&  socket_handle_valid(socket) )
		return true;
	if( socket_handle_valid(socket) ) stop();
	port = _port;
	nif  = _nif;
	if( ready() ) {
		socket = socket_create_tcp_server(port, nif);
		if( socket_handle_valid(socket) ) {
			log.debug("%s uses %lX\n",__func__, socket);
			for(auto& state : states) state.clear();
			return true;
		}
		log.error("listen port %d on nif %d error\n", port, nif);
	} else
		log.info("nif %d not ready\n", nif);
	return false;
}

void server::stop() noexcept {
	log.info("%s\n",__func__);
	queue.validate([](handle_t& handle, bool) noexcept
		-> bool {
		socket_close(handle);
		return false;
	}, 0);
	if( socket_handle_valid(socket) ) socket_close(socket);
	socket = invalid_handle;
}

using cojson::details::countof;

server::session_state* server::find_session(handle_t handle) noexcept {
	for(auto& state: states) {
		if( state.handle == handle ) {
			if( !state.isempty() )
				log.info("%s %lX %p\n", __func__, handle, &state);
			return &state;
		}
	}
	for(auto& state: states) {
		if( state.handle == invalid_handle ) {
			state.handle = handle;
			return &state;
		}
	}
	log.warn("%s %lX %p\n", __func__, handle, nullptr);
	return nullptr;
}

void server::run() noexcept {
	cojson::details::temporary_s<char,buffer_size,buffer_static> buffer;
	handle_t session_handle;
	size_t size = 0;
	if( invalid_handle != (session_handle = accept(buffer, size)) ) {
		session client(*this, find_session(session_handle), session_handle,
				buffer, size);
		app.service(client);
	}
}

bool server::keep(handle_t handle) noexcept {
	return queue.push(handle);
}

inline void server::kill_state_for(handle_t handle) noexcept {
	for(auto& i : states)
		if( i.handle == handle ) i.clear();
}

void server::close(handle_t handle) noexcept {
	if( handle == invalid_handle ) return;
	log.debug("%s %lX\n", __func__, handle);
	queue.replace(handle, invalid_handle);
	kill_state_for(handle);
	sock_result_t res = socket_close(handle);
	log.warn_if(res<0, "%s error %d\n", __func__, res);
}

handle_t server::accept(char (&buffer)[buffer_size], size_t& size) noexcept {
	handle_t result = invalid_handle;
	/* network status check  */
	if( ! ready() ) {
		if( socket != invalid_handle ) stop();
		return invalid_handle;
	}
	/* tcp status check and listen if not yet */
	if( ! socket_handle_valid(socket) && ! listen(port,nif) )
		return invalid_handle;

	if( log.enabled(miculog::level::debug)) {
		dbg_queuelen();
	}

	/* iterate through already accepted sockets in the queue,
	 * take first with data, close not active, expire idle	sockets		*/
	queue.validate([this,&result,&buffer,&size]
				(handle_t& handle, bool expired) noexcept -> bool {
		/* remove already closed items 									*/
		if( ! socket_handle_valid(handle) )
			return false;
		/* close and remove handles of inactive sockets 				*/
		if( ! SOCKET_STATUS_ACTIVE == socket_active_status(handle) ) {
			close(handle);
			return false;
		}
		/* keep handles of pending sockets 								*/
		if( result != invalid_handle )
			return true;
		sock_result_t res = socket_receive(handle, buffer, sizeof(buffer), 0);
		if( res < 0 ) {
			close(handle);
			return false;
		}
		/* expire idle sockets 											*/
		if( res == 0 ) {
			if( expired ) {
				log.debug("expired %lX\n", handle);
				close(handle);
			}
			return not expired;
		}
		/* remove  handle of the selected socket 						*/
		size = res;
		result = handle;
		return false;
	}, inactivity_timeout);

	if( result != invalid_handle ) {
		return result;
	}
	/* accept connections until one with data found, queue connections if idle */
	while(true) {
		result = socket_accept(socket);
		if( result != invalid_handle )
			log.debug("%s %lX\n", __func__, result);
		if( result == invalid_handle ) return result;
		if( (size = socket_receive(result, buffer, sizeof(buffer), 0)) )
			return result;
		if( ! queue.push(result) ) {
			log.debug("close! %lX\n", __func__, result);
			socket_close(result);
			return invalid_handle;
		}
	}
}

const handle_t server::invalid_handle = socket_handle_invalid();

server::session::session(server& _socket, session_state* _state,
		handle_t _handle, char (&data)[buffer_size], size_t length) noexcept
  : socket(_socket), state(_state), handle(_handle), buff(*this, data, length) {
	if( access.enabled() ) {
		socket_peer(handle, &peer, nullptr);
		access.log("session", peer.address, "%s [%d]\n", 
				(state && !state->isempty()? "saved":"new  "), length);
	}
}

inline bool server::session::isgood() const noexcept {
	return
		socket_handle_valid(handle) && socket.ready() &&
		(SOCKET_STATUS_ACTIVE == socket_active_status(handle));
}

size_t server::session::receive(void* buffer, size_t size) const noexcept {
	if( !isgood()  ) return -1;
	sock_result_t res = socket_receive(handle, buffer, size, 0);
	access.log(__func__, peer.address, " [%u]\n", res);
	log.warn_if(res<0, "%s error %d\n", __func__, res);
	return res;
}


size_t server::session::send(const void* data, size_t size) const noexcept {
	if( ! isgood() ) return -1;
	access.log(__func__, peer.address, " [%u]\n", size);
	sock_result_t res = socket_send(handle, data, size);
	log.warn_if(res<0, "%s error %d\n", __func__, res);
    return res;
}

void server::session::close() noexcept {
	buff.flush();
	access.log(__func__, peer.address);
	socket.close(handle);
	handle = invalid_handle;
}

sock_result_t server::session::buffer::sync() noexcept {
	sock_result_t chunk = conn.receive(data, size);
	if( chunk > 0 ) {
		endpos = data+chunk;
		getpos = data;
	} else
	if( chunk < 0 ) {
		error(error_t::ioerror);
	}
	return chunk;
}

bool server::session::buffer::get(char_t& val) noexcept {
	if( getpos >= endpos &&  error_t::noerror == (error() & error_t::ioerror)) {
		sock_result_t chunk = sync();
		if( chunk == 0 ) {
			val = iostate::eos_c;
			error(error_t::eof);
			return false;
		} else 	if( chunk < 0 ) {
			val = iostate::err_c;
			return false;
		}
	}
	val = *getpos++;
	return true;
}

bool server::session::buffer::put(char_t val) noexcept {
	if( putpos >= data+size && ! flush() ) return false;
	*putpos++ = val;
	return true;
}

bool server::session::buffer::flush() noexcept {
	bool res = true;
	if( putpos != data ) {
		res = conn.send(data, putpos-data) == static_cast<size_t>(putpos-data);
		putpos = data;
	}
	return res;
}

void server::dbg_queuelen() noexcept {
	static size_t reported_queuelen = 0;
	static size_t reported_stateslen = 0;
	size_t len = 0;
	for(auto i: states) {
		if( ! i.isempty() ) ++len;
	}
	if(  reported_queuelen  != queue.len()
	  || reported_stateslen != len ) {
		reported_queuelen  = queue.len();
		reported_stateslen = len;
		log.debug("queue conn/sess: %d/%d\n",
			reported_queuelen, reported_stateslen);
	}
}
}}}
void dbg(const char *fmt, ...) noexcept  {
	char buff[128] = {};
	va_list args;
	va_start(args, fmt);
	vsnprintf(buff, sizeof(buff), fmt, args);
	va_end(args);
	Serial1.write(buff);
}

