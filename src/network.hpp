/*
 * Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * network.hpp - common network definitions
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
#ifndef NETWORK_HPP_
#define NETWORK_HPP_
#ifndef MICUREST_HPP_
#	include "micurest.hpp"
#endif

namespace micurest {
struct application;

namespace network {
using cojson::size_t;
using cojson::char_t;
using cojson::details::istream;
using cojson::details::ostream;

class noncopyable {
private:
	noncopyable(const noncopyable&);
	noncopyable& operator=(const noncopyable&);
public:
	noncopyable() { }
};

typedef uint16_t port_t;
typedef uint32_t ipaddr_t;

struct endpoint {
	port_t port;
	ipaddr_t ip;
	bool inline constexpr operator==(const endpoint& that) const noexcept {
		return that.ip == ip && that.port == port;
	}
};

template<typename T>
static inline constexpr ipaddr_t ipaddr(T (&adr)[4]) noexcept {
	return
		(static_cast<ipaddr_t>(adr[0]))
	  |	(static_cast<ipaddr_t>(adr[1])>>8)
	  | (static_cast<ipaddr_t>(adr[2])>>16)
	  | (static_cast<ipaddr_t>(adr[3])>>24);
}

typedef uint_fast8_t queuetoken;

/** bind
 * binds port, application, transport protocol and application protocol
 * allocates per port connection data,
 * associates application with the port
 * @param Transport - transport protocol/socket class
 * @param Port - port
 * @param Proto - protocol instance (defaulted to T::I)
 * @param App - micurest application instance
 */
template<class Transport, port_t Port, class Proto, const application* App>
struct bind : noncopyable {
	//typedef provision<P, typename T::connection> conn;
	/* protocol instance have no storage for connections,
	 * therefore bind allocates static socket instance					*/
	static Transport socket;
	static inline bool listen() noexcept {
		return socket.listen(Port);
	}
	static inline bool run() noexcept {
		typename Transport::connection connection(socket);
		if( ! connection.accept() )
			return false;
		Proto::service(connection, App);
		return true;
	}
};


template<class Transport, port_t Port, class Proto, const application* App>
Transport bind<Transport,Port,Proto,App>::socket(App); /*
	App needed here for event-drivet TCP stack, such as on esp8266 	*/

/** bindings
 * a list list of port/application/protocol triplets
 * variadic iteration is selected because list L is expected to be short
 */
template<typename ... L>
struct bindings;

/* bindings: iterator finish */
template<>
struct bindings<> {
	static inline constexpr unsigned listen() noexcept { return 0;	}
	static constexpr queuetoken token = 0;
	static inline constexpr queuetoken run(queuetoken) noexcept { return token;	}
};

/* bindings: last iterable element  */
template<typename F>
struct bindings<F> {
	static inline unsigned listen() noexcept {
		return F::listen() ? 1 : 0;
	}
	static constexpr queuetoken token = 1;
	static inline queuetoken run(queuetoken next) noexcept {
		return F::run() ? --next : 0;
	}

};

/* bindings: ongoing iterations  */
template<typename F, typename ... L>
struct bindings<F, L...> {
	static constexpr queuetoken token = bindings<L...>::token+1;
	static_assert(token != 0, "count of bindings exceeded queuetoken width");
	static inline unsigned listen() noexcept {
		return bindings<F>::listen() + bindings<L...>::listen();
	}
	static inline queuetoken run(queuetoken next) noexcept {
		return next == token
		  ? (F::run() ? --next : bindings<L...>::run(--next))
		  : bindings<L...>::run(next);
	}
};

/** server
 * @patramlist L - bind templates
 */
template<typename ... L>
class server : public noncopyable {
private:
	typedef bindings<L...> all;
public:
	/* starts listening on all bound ports,
	 * returns number of successfully started listeners */
	unsigned listen() noexcept {
		return all::listen();
	}
	void run() noexcept { /* implements round-robin queuing */
		static queuetoken next = 0;
		next = all::run(next == 0 ? all::token : next);
	}
};

struct connection {
	inline const endpoint& remote() const noexcept { return _remote; }
protected:
	endpoint _remote;
};
namespace proto {

struct http {
	template<class connection>
	static void service(connection&, const application&) noexcept;
};

}

}}
#endif
