/*
 * Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * network_arduino.cpp - session layer implementation for Arduino
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
#include "network_arduino.hpp"

namespace micurest {
namespace network {
	typedef byte pbyte[4];
	template<>
	inline void access_log::_log<pbyte>(const char* lbl, const pbyte& addr,
														char dlm) noexcept {
	info("%s %d.%d.%d.%d %c", lbl, addr[0], addr[1], addr[2], addr[3], dlm);
}
}
template<>
void application::service<network_arduino::tcp::server::session>(
		network_arduino::tcp::server::session& con) const noexcept {
	typedef application::result_t result_t;
	istream& in(con.input());
	ostream& out(con.output());
	result_t res = service(in, out);
	if( res == result_t::keep )
		con.keep();
	else
		con.close();
}

namespace network_arduino {
/******************************************************************************/

template<typename K, typename V, uint8_t N>
class array {
public:
	/* finds a slot corresponding to key or returns an empty slot */
	template<typename T>
	V* find(const T& key, uint8_t pos) noexcept {
		return pos <= N && tuples[pos].key == key ? &tuples[pos].key : nullptr;
	}
	inline void clear(uint8_t pos) noexcept {
		tuples[pos].key = {};
	}
private:
	struct tuple {
		K key;
		V val;
	} tuples[N];
};

//static array<endpoint, httpmessage::state_pdo, MAX_SOCK_NUM> httpstate = {};

static const network::access_log access;
static const miculog::Log<tcp::server> log;

/******************************************************************************/
namespace tcp {

inline bool server::listen(handle_t handle, port_t port) noexcept {
	if ( ::socket(handle, SnMR::TCP, port, 0) ) {
		if( ::listen(handle) ) return true;
		::close(handle);
	}
	log.error("listen port %d error\n", port);
	return false;
}


bool server::listen(port_t p) noexcept {
	port = p;
	uint_fast8_t res = 0;
	for (uint8_t handle = 0; handle < MAX_SOCK_NUM; ++handle) {
		if( ::socketStatus(handle) == SnSR::CLOSED ) {
				if( listen(handle, port) ) ++res;
		}
	}
	return res != 0;
}

void server::run() noexcept {
	if( accept() ) {
		session client(*this);
		app.service(client);
	}
}


void server::stop() noexcept {
	for (uint_fast8_t handle = 0; handle < MAX_SOCK_NUM; ++handle) {
		if( ::socketStatus(handle) != SnSR::CLOSED ) {
			::close(handle);
		}
	}
}

bool server::accept() noexcept {
	accepted = badhandle;
	uint_fast8_t i;
	handle_t handle = 0;
	for(i = 0; i < MAX_SOCK_NUM && accepted == badhandle; ++i ) {
		handle = next++ % MAX_SOCK_NUM; /* round robin */
		uint8_t status = ::socketStatus(handle);
		if( status == SnSR::ESTABLISHED || status == SnSR::CLOSE_WAIT ) {
			uint16_t len;
			if( (len = recvAvailable(handle)) ) {
				accepted = handle;
//				httpstate.clear(handle);
			} else {
				if( status == SnSR::CLOSE_WAIT ) ::disconnect(handle);
			}
		} else {
			if( status == SnSR::CLOSED ) listen(handle, port);
		}
	}
	/* reopening stale sockets and closing disconnected by PEER */
	while( i++ < MAX_SOCK_NUM ) {
		++handle;
		handle %= MAX_SOCK_NUM;
		uint8_t status = ::socketStatus(handle);
		if( status == SnSR::CLOSED ) {
			listen(handle, port);
		}
		else if( status == SnSR::CLOSE_WAIT && ! recvAvailable(handle) ) {
			::disconnect(handle);
		}
	}
	return accepted != badhandle;
}

server::session::session(const server& socket) noexcept
  : handle(socket.accepted), buff(*this) {
	if( access.enabled() ) {
		W5100.readSnDIPR(handle,remote_ip);
		access.log("accepted", remote_ip);
	}
}

size_t server::session::available() const noexcept {
	return ::recvAvailable(handle);
}

size_t server::session::receive(void* data, size_t size) const noexcept {
	access.log(__func__, remote_ip, " [%u]\n", size);
	int16_t res = ::recv(handle, (uint8_t*) data, size);
	log.warn_if(res<0, "%s error\n", __func__);
	return res;
}


size_t server::session::send(const void* data, size_t size) const noexcept {
	access.log(__func__, remote_ip, " [%u]\n", size);
	size_t res = ::send(handle, (uint8_t*) data, size);
	log.warn_if(!res, "%s error\n", __func__);
    return res;
}

void server::session::close() noexcept {
	access.log(__func__, remote_ip);
	buff.flush();
	size_t remain;
	size_t limit =  1500 / sizeof(buff.data);
	/* try to discard up to MTU bytes 										*/
	while( (remain = recvAvailable(handle)) && limit-- ) {
		receive(buff.data, remain>sizeof(buff.data)?sizeof(buff.data):remain);
	}
	::disconnect(handle);
	//TODO associate time stamp with current handle to close forcibly if not closed willingly
}

bool server::session::buffer::get(char_t& val) noexcept {
	if( getpos >= endpos ) {
		size_t chunk = conn.available();
		if( chunk )
			chunk = conn.receive(data, chunk>size?size:chunk);
		else {
			val = iostate::eos_c;
			error(error_t::eof);
			return false;
		}
		if( ! chunk ) {
			val = iostate::err_c;
			error(error_t::ioerror);
			return false;
		}
		endpos = data+chunk;
		getpos = data;
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

}}}
