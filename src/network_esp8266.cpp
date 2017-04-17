/*
 * Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * network_esp8266.cpp - session layer implementation for ESP8266
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
 * See the GNU General Public License v2 for more details.
 *
 * You should have received a copy of the GNU General Public License v2
 * along with the µcuREST Library; if not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>.
 */
extern "C" {
#	include "user.h"
}

#include "access_log.hpp"
#include "network_esp8266.hpp"

static constexpr const char* proto_name(espconn_type p) noexcept {
	return p == ESPCONN_TCP ? "TCP" : p == ESPCONN_UDP ? "UDP" : "ERR";
}

namespace micurest {
namespace network {
template<>
inline void access_log::_log<esp_tcp>(const char* lbl, const esp_tcp& addr,
														char dlm) noexcept {
	info("%s " IPSTR "%c", lbl, IP2STR(addr.remote_ip), dlm);
}
}
namespace network_esp {

class ibuffer : public istream {
public:
	template<size_t N>
	inline ibuffer(char (&buf)[N]) noexcept
	  : begin(buf), curr(buf), end(buf+N) { }
	inline ibuffer(const char* data, size_t len) noexcept
	  : begin(data), curr(data), end(data+len) { }
	bool get(char_t& val) noexcept;
private:
	const char * begin;
	const char * curr;
	const char * const end;
};

class obuffer : public ostream {
public:
	template<size_t N>
	inline obuffer(char (&buf)[N]) noexcept
	  : pos(0), size(N), buff(buf) { }
	bool put(char_t val) noexcept;
	size_t 	pos;
	const size_t size;
	char* buff;
};

bool ibuffer::get(char_t& val) noexcept {
	if( curr >= end  ) {
		val = iostate::eos_c;
		error(error_t::eof);
		return false;
	}
	val = *(curr++);
	return true;
}

bool obuffer::put(char_t val) noexcept {
	if( pos >= size ) {
		error(error_t::eof);
		return false;
	}
	buff[pos++] = val;
	return true;
}

static inline constexpr tcp::server* cast(espconn* conn) noexcept {
	return static_cast<tcp::server*>(conn->reverse);
}

static inline constexpr espconn* cast(void *arg) noexcept {
	return static_cast<espconn*>(arg);
}

/* generic purpose log to console */
static const miculog::Log<tcp::server> log;
static const network::access_log access;
/***************************************************************************/

static constexpr size_t maxconn = 5; //TODO make maxconn configurable
static constexpr size_t closequ_len = 8; //TODO make closequ_len configurable
using micurest::details::httpmessage;


struct endpoint {
	port_t port;
	ip_addr ip;
	bool inline constexpr operator==(const endpoint& that) const noexcept {
		return that.ip.addr == ip.addr && that.port == port;
	}
};


template<typename T>
static inline constexpr ip_addr ipaddr(T (&adr)[4]) noexcept {
	typedef decltype(ip_addr::addr) ip_addr_t;
	return {
		(static_cast<ip_addr_t>(adr[0]))
	  |	(static_cast<ip_addr_t>(adr[1])>>8)
	  | (static_cast<ip_addr_t>(adr[2])>>16)
	  | (static_cast<ip_addr_t>(adr[3])>>24) };
}


/* HTTP state for 5 connections at most */
details::map<endpoint, httpmessage::state_pdo, maxconn> httpstate = {};


/******************************************************************************/
namespace tcp {

void server::keep() noexcept {
	/* by default connection option is close and callback - keep */
	sint8 res = espconn_set_opt(&con, ESPCONN_KEEPALIVE);
	log.warn_if(res,"espconn_set_opt error %d\n", res);
	res = espconn_regist_sentcb(&con, &on::sending);
	log.warn_if(res,"espconn_regist_sentcb error %d\n", res);
}

void server::nokeep() noexcept {
	sint8 res = espconn_clear_opt(&con, ESPCONN_KEEPALIVE);
	log.warn_if(res,"espconn_clear_opt error %d\n", res);
	res = espconn_regist_sentcb(&con, &on::sent);
	log.warn_if(res,"espconn_regist_sentcb error %d\n", res);
}

bool server::listen(port_t port) noexcept {
	con.type = ESPCONN_TCP;
	con.proto.tcp = &tpc;
	con.state = ESPCONN_NONE;
	tpc.local_port = port;
	con.reverse = this;
	sint8 res =	espconn_regist_connectcb(&con, &on::connect);
	log.error_if(res, "espconn_regist_* error %d\n", res);
	return accept();
}

void server::receive(const void* data, size_t len) noexcept {
	//FIXME make configurable static/automatic
	static char buff[mtu_size]; /*
		* output buffer is shared											*
		* among all connections												*
		* in esp8266 non os sdk it is safe because							*
		* no concurrent callbacks  possible 								*/
	typedef application::result_t result_t;
	access.log(__func__, *con.proto.tcp, " [%d]\n", len);
	ibuffer in((const char*)data, len);
	obuffer out(buff);
	httpmessage::state_pdo* pdo = httpstate.find(
		endpoint{(port_t)con.proto.tcp->remote_port,
				  ipaddr(con.proto.tcp->remote_ip)}
	);
	result_t res = app.service(in, out, pdo);
	if( res == result_t::keep ||  res == result_t::fragment)
		keep();
	else
		nokeep();
	//TODO 503 if response is empty and not a fragment
	if( !(pdo && res == result_t::fragment) ) {
		send(out.buff, out.pos);
		if( pdo ) pdo->clear();
	}
}

void server::send(const void* data, size_t size) noexcept {
	access.log(__func__, *con.proto.tcp, " [%u]\n", size);
//	os_delay_us(5000);
	sint8 res = espconn_send(&con,(uint8 *)data, size);
	log.warn_if(res, "espconn_send error %d", res);
}

void server::close() noexcept {
	sint8 res = -1;
	access.log("disconnecting",*con.proto.tcp," state=%d\n",con.state);
	res = espconn_disconnect(&con);
	log.warn_if(res, "espconn_disconnect error %d\n", res);
}

void server::on::connect(void *arg) noexcept {
	access.log(__func__, *cast(arg)->proto.tcp);
	espconn_regist_recvcb(cast(arg), &received);
	espconn_regist_sentcb(cast(arg), &sent);
	espconn_regist_disconcb(cast(arg), &disconnect);
	espconn_regist_reconcb(cast(arg), &error);
	espconn_regist_time(cast(arg), timeout, 1);
}

void server::on::received(void *arg, char *data, unsigned short len) noexcept {
	access.log(__func__, *cast(arg)->proto.tcp, " [%d]\n", len);
	server* soc = cast(cast(arg));
	if( soc ) {
		soc->receive(const_cast<const char*>(data), len);
	}
}

void server::on::error(void *arg, sint8 err) noexcept {
	access.log(__func__, *cast(arg)->proto.tcp);
	log.error("connection error %d\n", err);
	if( cast(arg)->state == ESPCONN_CLOSE ) {
		espconn_regist_time(cast(arg), 1, 1);
		return;
	}
	server* soc = cast(cast(arg));
	if( soc ) soc->close();
	else espconn_secure_disconnect(cast(arg)); /* fallback */
}

void server::on::disconnect(void *arg) noexcept {
	access.log(__func__, *cast(arg)->proto.tcp);
	httpmessage::state_pdo* pdo = httpstate.find(
		endpoint{(port_t)cast(arg)->proto.tcp->remote_port,
				  ipaddr(cast(arg)->proto.tcp->remote_ip)});
	if( pdo ) pdo->clear();
}

void server::on::sent(void *arg) noexcept {
	/* 1. esp8266 non-os sdk reference states:
	 *    Do not call espconn_disconnect in any espconn callback
	 *    if needed, please use system_os_task
	 * 2. Connection pointers in callbacks cannot be used outside of
	 *    a callback context
	 * 3. However, closing connection with a connection identity:
	 *   {local_port, local_ip, remote_port, remote_ip} or
	 *   {remote_port, remote_ip} does not work
	 */

	access.log(__func__, *cast(arg)->proto.tcp);
	server* soc = cast(cast(arg));
	if( soc ) soc->close();
	else {
		log.warn("fallback to espconn_secure_disconnect\n");
		espconn_secure_disconnect(cast(arg)); /* fallback */
	}
}

void server::on::sending(void *arg) noexcept {
	access.trace(__func__, *cast(arg)->proto.tcp,"\n");
}

bool server::accept() noexcept {
	sint8 res = espconn_accept(&con);
	log.error_if(res, "espconn_accept error %d\n", res);
	if( res ) return false;
	return true;
}
}
namespace tls {
void proto::send(const char* data, size_t size) noexcept {
	access.log(__func__, *con.proto.tcp, " [%u]\n", size);
	sint8 res = espconn_secure_send(&con,(uint8 *)data, size);
	log.warn_if(res, "espconn_secure_send error %d\n", res);
}

void proto::close() noexcept {
	access.trace("disconnecting secure",*con.proto.tcp," state=%d\n",con.state);
	sint8 res = espconn_secure_disconnect(&con);
	log.warn_if(res, "espconn_secure_disconnect %d\n", res);
}

bool proto::accept() noexcept {
	if( ! espconn_secure_set_default_certificate(cert, cert_len) )
		log.error("espconn_secure_set_default_certificate failed\n");
	if( ! espconn_secure_set_default_private_key(key, key_len) )
		log.error("espconn_secure_set_default_private_key failed\n");
	return ! espconn_secure_accept(&con);
}

}

}}

