/*
 * Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * network_lwip.cpp - session layer implementation for lwIP
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
#include "network_lwip.hpp"
#include "access_log.hpp"
#include <list>
extern "C" {
#	include <lwip/api.h>
#	include <lwip/tcp.h>
}

namespace micurest {
namespace network {
template<>
void access_log::_log<ip_addr>(const char* lbl, const ip_addr& adr,
		char dlm) noexcept {
	switch( adr.type ) {
	case IPADDR_TYPE_V4:
		info("%s %s%c", lbl, ip4addr_ntoa(&adr.u_addr.ip4), dlm);
		break;
	case IPADDR_TYPE_V6:
		info("%s %s%c", lbl, ip6addr_ntoa(&adr.u_addr.ip6), dlm);
		break;
	default:
		info("%s family %d%c", lbl, adr.type, dlm);
	}
}
}

namespace network_lwip {
using namespace network;

using cojson::char_t;
using cojson::details::noncopyable;
using cojson::details::istream;
using cojson::details::ostream;
using cojson::details::error_t;

class ibuffer : public istream {
public:
	inline ibuffer(pbuf* p) noexcept : buf(p), pos(0) { }
	bool get(char_t& val) noexcept;
private:
	pbuf *buf;
	size_t 	pos;
};

class obuffer : public ostream {
public:
	inline obuffer(tcp_pcb* pcb) noexcept
	  : pos(0), socket(pcb) { }
	bool put(char_t val) noexcept;
	size_t 	pos;
	static constexpr const size_t size = 64;
	char buff[size];
	/*
	 * static output buffer will be shared								*
	 * among all connections, anyway lwip is restricted to one thread	*
	 * and therefore no concurrent callbacks  to here possible			*/
private:
	tcp_pcb* socket;
};

bool ibuffer::get(char_t& val) noexcept {
	int chr = pbuf_try_get_at(buf, pos++);
	if( chr < 0  ) {
		val = iostate::eos_c;
		error(error_t::eof);
		return false;
	}
	val = chr;
	return true;
}

bool obuffer::put(char_t val) noexcept {
	if( pos >= size ) {
		err_t err = tcp_write(socket, buff, size, TCP_WRITE_FLAG_COPY|TCP_WRITE_FLAG_MORE);
		if( ERR_OK != err ) {
			error(error_t::ioerror);
			return false;
		}
		pos = 0;
	}
	buff[pos++] = val;
	return true;
}

static const network::access_log access;

/******************************************************************************/
namespace tcp {
using micurest::details::httpmessage;
using config = configuration::Configuration<server>;

class session : details::noncopyable {
public:
	static constexpr uint8_t poll_interval = config::poll_interval; // 4; //TODO make configurable
	template<bool alloction_free = false>
	struct factory;
	void receive(pbuf *p) noexcept;
	void send(const void* data, size_t size, bool keep) noexcept;
	void close() noexcept;
	/** lwIP callbacks for active connection */
	struct on {
		static err_t poll(void *arg, tcp_pcb*) noexcept;
		static err_t received(void *arg, tcp_pcb*, pbuf *data, err_t) noexcept;
		static void error(void *arg, err_t) noexcept;
		static err_t sent(void *arg, tcp_pcb*, uint16_t len) noexcept;
		static err_t sending(void *arg, tcp_pcb*, uint16_t len) noexcept;
		static inline session* cast(void* arg) noexcept {
			return static_cast<session*>(arg);
		}
	};
	inline session(const application& a, tcp_pcb* pcb) noexcept
	  : sock(pcb), app(a)  { attach(pcb); }
	inline ~session() {}
	static void detach(tcp_pcb* pcb) noexcept;
private:
	inline session() = delete;
	void attach(tcp_pcb* pcb) noexcept;
	tcp_pcb* sock;
	httpmessage::state_pdo pdo = {};
	const application& app;
	static const miculog::Log<session> log;
};

template<>
struct session::factory<true> {
	//TODO make configurable allocation free factory
};

template<>
struct session::factory<false> {
	static session* construct(const application& app, tcp_pcb* sock) noexcept {
		return new session(app, sock);
	}
	static void destruct(session* s) noexcept {
		delete s;
	}
};

err_t session::on::received(void *arg, tcp_pcb* pcb, pbuf *p, err_t err) noexcept {
	if( err != ERR_OK || p == nullptr || arg == nullptr ) {
		log.warn("BAD %s(%p,%p,%p,%d)\n",__func__, arg, pcb, p, err);
		tcp_recved(pcb, p ? p->tot_len : 0);
		if( p ) pbuf_free(p);
		//FIXME send 503 Out of memory instead
		if( arg ) cast(arg)->close();
		return ERR_OK;
	}
	cast(arg)->receive(p);
	return ERR_OK;
}

err_t session::on::poll(void *arg, tcp_pcb* pcb) noexcept {
	if( arg == nullptr ) {
		if( ERR_MEM == tcp_close(pcb) ) {
			log.warn("aborting stale connection\n");
			tcp_abort(pcb);
			return ERR_ABRT;		}
	} else cast(arg)->close(); /* stale sessions are closed  */
	return ERR_OK;
}


void session::detach(tcp_pcb* pcb) noexcept {
	tcp_arg(pcb, nullptr);
	tcp_recv(pcb, nullptr);
	tcp_err(pcb, nullptr);
	tcp_poll(pcb, nullptr, 0);
	tcp_sent(pcb, nullptr);
}

void session::attach(tcp_pcb* pcb) noexcept {
	tcp_arg(pcb, this);
	tcp_recv(pcb, on::received);
	tcp_err(pcb, on::error);
	tcp_poll(pcb, on::poll, poll_interval);
	tcp_sent(pcb, on::sent);
}


void session::receive(pbuf *p) noexcept {
	typedef application::result_t result_t;
	ibuffer in(p);
	obuffer out(sock);
	access.log(__func__, sock->remote_ip, "%d bytes\n", p->tot_len);
	result_t res = app.service(in, out, &pdo);
	tcp_recved(sock, p->tot_len); /* assuming at this point everything is received */
	pbuf_free(p);
	if( res != result_t::fragment ) {
		send(out.buff, out.pos, res==result_t::keep||res==result_t::fragment);
		if( res == result_t::keep ) pdo.clear();
		//TODO 503 if response is empty and not a fragment
	}
}

void session::send(const void* data, size_t size, bool keep) noexcept {
	tcp_sent(sock, keep ? on::sending : on::sent);
	err_t res = ERR_OK;
	if( size )
		res = tcp_write(sock, data, size, TCP_WRITE_FLAG_COPY | (keep ? TCP_WRITE_FLAG_MORE : 0));
	log.warn_if(res, "tcp_write error %d", res);
	if( ERR_OK == res ) {
		res = tcp_output(sock);
		log.warn_if(res, "tcp_write error %d", res);
	}
}

void session::close() noexcept {
	detach(sock);
	err_t res = tcp_close(sock);
	if( ERR_OK == res )
		factory<>::destruct(this);
	else {
		log.warn("tcp_close error %d\n", res);
		tcp_poll(sock, on::poll, poll_interval);
	}
}

err_t session::on::sent(void * arg, tcp_pcb* pcb, uint16_t len) noexcept {
	access.log(__func__, pcb->remote_ip, "%d bytes, %s\n", len, "close");
	cast(arg)->close();
	return ERR_OK;
}

err_t session::on::sending(void *, tcp_pcb* pcb, uint16_t len) noexcept {
	access.log("sent", pcb->remote_ip, "%d bytes, %s\n", len, "keep");
	return ERR_OK;
}

void session::on::error(void *arg, err_t err) noexcept {
	if( arg ) access.log(__func__, cast(arg)->sock->remote_ip);
	log.error("session error %d\n", err);
	cast(arg)->close();
}

/******************************************************************************/

static miculog::Log<server> log;

/** lwIP callbacks for listening socket */
struct on {
	/* assumption - lwip does not report errors for a listening socket		*/
	static err_t accept(void *arg, tcp_pcb* pcb, err_t err) noexcept;
	static inline constexpr const tcp::server* cast(const void *arg) noexcept {
		return static_cast<const tcp::server*>(arg);
	}
};


static constexpr uint8_t tcp_prio = config::tcp_prio; //TODO make prio configurable

bool server::listen(port_t port) noexcept {
	err_t err;
	tcp_pcb* lpcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
	if( ! lpcb ) return false;
	tcp_setprio(lpcb, tcp_prio);
	err = tcp_bind(lpcb, IP_ANY_TYPE, port); //TODO make IP configurable
	if( ERR_OK == err && (pcb = tcp_listen(lpcb)) ) {
		access.log(__func__, pcb->local_ip);
		tcp_arg(pcb, this);
		tcp_accept(pcb, on::accept);
		return true;
	}
	tcp_close(lpcb);
	return false;
}

err_t on::accept(void *arg, tcp_pcb* pcb, err_t err) noexcept {
	if ((err != ERR_OK) || (pcb == nullptr) || (arg == nullptr)) {
		log.debug("%s(%p,%p,%d) - invalid arguments\n", arg, pcb, err);
		return ERR_VAL;
	}
	access.log(__func__, pcb->remote_ip);
	tcp_setprio(pcb, tcp_prio);
	session* client = session::factory<>::construct(cast(arg)->getapp(), pcb);
	if( ! client ) {
		log.error("null session, closing connection\n");
		// TODO perhaps we can send 500 Server too busy
		tcp_close(pcb);
		return ERR_VAL;
	}
	return ERR_OK;
}

}
}
}

