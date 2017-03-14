/*
 * Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * esp8266.hpp - header file for esp8266 example
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

#ifndef USER_ESP8266_HPP_
#define USER_ESP8266_HPP_
#include "network_esp8266.hpp"
#include "io.hpp"
extern "C" {
#include "user.h"
}

struct mac_addr {
	static constexpr unsigned len = 6;
	uint8 addr[len];
};

#define NAME(s) constexpr const char* s() noexcept {return #s;}
#define ALIAS(f,s) constexpr const char* f() noexcept {return #s;}
namespace name {
	ALIAS(_default,default)
	ALIAS(index,index.html)
	static inline constexpr const char* dot() noexcept {return ".";}
	static inline constexpr const char* asterisk() noexcept {return "*";}

	NAME(addr)
	NAME(ap)
	NAME(autoconnect)
	NAME(beacon_interval)
	NAME(boot_mode)
	NAME(boot_version)
	NAME(bssid)
	NAME(channel)
	NAME(chip_id)
	NAME(config)
	NAME(cpu_freq)
	NAME(current)
	NAME(dhcp)
	NAME(dhcps)
	NAME(enable)
	NAME(end)
	NAME(free_heap_size)
	NAME(gw)
	NAME(hidden)
	NAME(hostname)
	NAME(interfaces)
	NAME(ip)
	NAME(leasetime)
	NAME(mac)
	NAME(mask)
	NAME(maxcon)
	NAME(meta)
//	NAME(mode)
	NAME(password)
	NAME(range)
	NAME(rtclock)
	NAME(rtdata)
	NAME(server)
	NAME(sdk_version)
	NAME(ssid)
	NAME(sta)
	NAME(start)
	NAME(sysinfo)
	NAME(sntptime)
	NAME(sntp)
	NAME(time)
	NAME(username)
	NAME(vdd33)
	NAME(wifi_mode)
	NAME(port)
	NAME(gpio)
	NAME(pad)
	NAME(mode)
	NAME(func)
}

namespace espconfig {
//
template<io::mode_t K, enumnames::name V>
struct _ : enumnames::tuple<io::mode_t, K,V> {};

template<io::func_t K, enumnames::name V>
struct __ : enumnames::tuple<io::func_t, K,V> {};

struct mode_names : enumnames::names<io::mode_t,
	_<io::mode_t::na,			&io::name::na >,
	_<io::mode_t::in,			&io::name::in >,
	_<io::mode_t::out,			&io::name::out>,
	_<io::mode_t::pullup,		&io::name::pullup>,
	_<io::mode_t::__unknown__,	nullptr  >> {
};

}

namespace cojson {
namespace details {

template<>
struct writer<ip_addr> {
	static inline bool write(const ip_addr addr, ostream& out) noexcept {
		return
			out.put('"') &&	writer<uint8_t>::write(0xFF & (addr.addr    ), out) &&
			out.put('.') && writer<uint8_t>::write(0xFF & (addr.addr>> 8), out) &&
			out.put('.') && writer<uint8_t>::write(0xFF & (addr.addr>>16), out) &&
			out.put('.') && writer<uint8_t>::write(0xFF & (addr.addr>>24), out) &&
			out.put('"');
	}
};

template<>
struct reader<ip_addr> {
	static bool read(ip_addr& dst, lexer& in) noexcept {
		using namespace cojson;
		using namespace cojson::details;
		char_t buffer[sizeof(ip_addr)*4];
		if(!cojson::details::reader<char_t*>::read(buffer, sizeof(buffer), in))
			return false;
		char_t* curr = buffer;
		unsigned val = 0;
		uint_fast8_t shft = 0;
		uint_fast8_t cnt = 0;

		decltype(dst.addr) addr = 0;
		for(;curr<buffer+sizeof(buffer);++curr) {
			if( +(chartype(*curr) & ctype::digit) ) {
				if( ! tenfold<decltype(val)>(val,*curr-'0') ) return false;
				if( val > 0xFF ) return false;
				++cnt;
			} else {
				if( *curr == 0 || *curr == '.' ) {
					if( cnt == 0 ) return false;
					addr |= static_cast<decltype(addr)>(val) << shft;
					if( *curr == 0 ) {
						dst.addr = addr;
						return true;
					}
					shft += 8;
					cnt = 0;
					val = 0;
				}
			}
		}
		return false;
	}
};

static inline bool ashex(uint8_t v, ostream& out) noexcept {
	return out.put(details::ashex(v >> 4)) && out.put(details::ashex(v & 0xF));
}

template<>
struct writer<mac_addr> {
	static inline bool write(const mac_addr& addr, ostream& out) noexcept {
		char dlm = '"';
		for(size_t i=0; i<countof(addr.addr); ++i) {
			out.put(dlm);
			ashex(addr.addr[i], out);
			dlm = ':';
		}
		return out.put('"');
	}
};
template<>
struct reader<mac_addr> {
	static bool read(mac_addr& dst, lexer& in) noexcept {
		using namespace cojson;
		using namespace cojson::details;
		char_t buffer[sizeof(mac_addr)*3];
		if(!cojson::details::reader<char_t*>::read(buffer, sizeof(buffer), in))
			return false;
		char_t* curr = buffer;
		unsigned val = 0;
		uint_fast8_t cnt = 0;
		uint_fast8_t pos = 0;
		ctype ctp;

		uint8_t addr[mac_addr::len] = {};
		for(;curr<buffer+sizeof(buffer);++curr) {
			if(+(ctp = chartype(*curr) & (ctype::digit|ctype::hex|ctype::heX))){
				val <<= 4;
				val |= *curr - (ctp == ctype::digit
						? '0' : ctp == ctype::hex
						? 'a'-0xA : 'A'-0xA);
				if( val > 0xFF ) return false;
				++cnt;
			} else {
				if( *curr == 0 || *curr == ':' ) {
					if( cnt != 2 ) return false;
					addr[pos++] = val;
					if( *curr == 0 ) {
						assign(dst.addr, addr);
						return true;
					}
					cnt = 0;
					val = 0;
				}
			}
		}
		return false;
	}
};

/**
 * string class property with storage type T (could be different from char)
 */
template<>
struct writer<io::mode_t> {
	static inline bool write(io::mode_t val, ostream& out) noexcept {
		return writer<const char*>::write(espconfig::mode_names::get(val), out);
	}
};

template<>
struct reader<io::mode_t> {
	static bool read(io::mode_t& dst, lexer& in) noexcept {
		char buff[16];
		if( reader<char_t*>::read(buff,countof(buff), in) ) {
			dst = espconfig::mode_names::get(buff);
			return true;
		}
		return false;
	}
};
}
template<class C, details::name id, typename T, size_t N, T (C::*M)[N]>
const details::property<C> & S() noexcept {
	static const struct local : details::property<C> {
		cstring name() const noexcept { return id(); }
		bool read(C& obj, details::lexer& in) const noexcept {
			return details::reader<char_t*>::read(reinterpret_cast<char_t*>(obj.*M), N, in);
		}
		bool write(const C& obj, details::ostream& out) const noexcept {
			return details::writer<const char_t*>::write(reinterpret_cast<const char_t*>(obj.*M), out);
		}
	} l;
	return l;
}

/**
 * string class property with asymmetric read/write
 */
template<class C, details::name id, size_t N, char_t (C::*M)[N], char* (*F)() noexcept>
const details::property<C> & S() noexcept {
	static const struct local : details::property<C> {
		cstring name() const noexcept { return id(); }
		bool read(C& obj, details::lexer& in) const noexcept {
			return details::reader<char_t*>::read(obj.*M, N, in);
		}
		bool write(const C& obj, details::ostream& out) const noexcept {
			return details::writer<const char_t*>::write(F(), out);
		}
	} l;
	return l;
}



/**
 * object property of type T with storage class S
 */
template<class C, details::name id, typename T, typename S, S (C::*M), typename H>
const details::property<C> & X() {
	static const struct local : details::property<C> {
		cstring name() const noexcept { return id(); }
		bool read(C& obj, details::lexer& in) const noexcept {
			return H::has(obj, details::reader<T>::read(*reinterpret_cast<T*>(&(obj.*M)), in));
		}
		bool write(const C& obj, details::ostream& out) const noexcept {
			return H::has(obj)
			  ? details::writer<T>::write(*reinterpret_cast<const T*>(&(obj.*M)), out)
			  : details::value::null(out);
		}
	} l;
	return l;
}

/**
 * property of type T accessible via F
 */
template<class C, details::name id, typename T, T* (*F)() noexcept>
const details::property<C> & X() {
	static const struct local : details::property<C> {
		cstring name() const noexcept { return id(); }
		bool read(C& obj, details::lexer& in) const noexcept {
			return details::reader<T>::read(*F(), in);
		}
		bool write(const C& obj, details::ostream& out) const noexcept {
			T* v;
			return (v = F()) ? details::writer<T>::write(*v, out) : details::value::null(out);
		}
	} l;
	return l;
}

namespace accessor {
/**
 * Field accessor via member pointer
 */

template<class C, typename T, typename S, S C::*V>
struct field_cast {
	typedef C clas;
	typedef T type;
	static constexpr bool canget = true;
	static constexpr bool canset = true;
	static constexpr bool canlref   = false; //true;
	static constexpr bool canrref   = false; //true;
	static constexpr bool is_vector = false;
	static inline constexpr bool has() noexcept { return true; }
	static inline void init(T&) noexcept { }
	static inline T get(const C& o) noexcept { return static_cast<T>(o.*V); }
	static T& lref(C& o) noexcept;
	static const T& rref(const C& o) noexcept;
	static inline void set(C& o, T v) noexcept { o.*V = static_cast<S>(v); }
	static inline constexpr bool null(C&) noexcept {
		return not config::null_is_error;
	}

private:
	field_cast();
};
}

/**
 * scalar class property with type case
 */
template<class C, details::name id, typename T, typename S, S C::*V>
const details::property<C> & X() noexcept {
	static const struct local : details::propertyx<accessor::field_cast<C,T,S,V>> {
		cstring name() const noexcept { return id(); }
	} l;
	return l;
}

/**
 * not parseable string bound to a zero-teminated string via function F
 */
template<class C, details::name id, const char_t* (*F)() noexcept>
const details::property<C>& X() noexcept {
	static const struct local : details::property<C>, details::string {
		inline local() noexcept : details::string(F()) {}
		cstring name() const noexcept { return id(); }
		bool read(C&, details::lexer& in) const noexcept {
			return details::string::read(in);
		}
		bool write(const C&, details::ostream& out) const noexcept {
			return details::string::write(out);
		}
	} l;
	return l;
}

}

using cojson::details::clas;
static inline const clas<ip_info>& ip_structure() noexcept {
	using namespace cojson;
	return
		O<ip_info,
			P<ip_info, name::addr,   ip_addr, &ip_info::ip>,
			P<ip_info, name::mask, ip_addr, &ip_info::netmask>,
			P<ip_info, name::gw, ip_addr, &ip_info::gw>
		>();
}

template<class E>
const cojson::details::value& Enums() noexcept {
	static const cojson::details::strings<E::get> l;
	return l;
}

using micurest::resource::entry;
using micurest::resource::node;

const entry& esp_config_entry() noexcept;
const entry& esp_config_gpio()  noexcept;
const entry& esp_config_port()  noexcept;
const entry& esp_config_pad()  noexcept;
const entry& esp_gpio_meta() noexcept;
const node& esp_gpio_metadata() noexcept;
const entry& esp_gpio_entry() noexcept;
const entry& esp_gpio_all() noexcept;
const entry& esp_pad_entry() noexcept;
const entry& esp_port_entry() noexcept;

using cojson::O;
using cojson::P;
using cojson::X;

struct json_error {
	struct name {
		static NAME(response_type)
		static NAME(msg)
		static NAME(error)
	};
	const char* msg;
	int32 error;
	static inline const clas<json_error>& structure() {
		return O<json_error,
			X<json_error, name::response_type, &json_error::name::error>,
			P<json_error, name::error, int32, &json_error::error>,
			P<json_error, name::msg, &json_error::msg>
		>();
	}
	inline bool write(cojson::details::ostream& out) noexcept {
		return structure().write(*this, out);
	}
};

#endif /* USER_ESP8266_HPP_ */
