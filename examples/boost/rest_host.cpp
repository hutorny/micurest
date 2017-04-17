/*
 * Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * rest_host.cpp - boost bases implementation
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

#include <cstdarg>
#include <iostream>
//#include <type_traits>
#include "server.hpp"
#include "micurest.hpp"
#include "enumnames.hpp"

using namespace http::server;
using micurest::char_t;
#include <cstdint>

class istream : public micurest::istream {
public:
	istream(std::istream& io) : in(io) {}
	bool get(char_t& c) noexcept {
		try {
			if( in.get(c).good() ) {
				return true;
			}
			if( in.eof() ) {
				error(cojson::details::error_t::eof);
				c = iostate::eos_c;
			} else {
				error(cojson::details::error_t::ioerror);
				c = iostate::err_c;
			}
		} catch (...) {	}
		return false;
	}
private:
	std::istream& in;
};

class ostream : public micurest::ostream {
public:
	inline ostream(std::ostream& io) noexcept : out(io) {}
	bool put(char_t c) noexcept {
		try {
			if( out.put(c).good() ) return true;
			error(cojson::details::error_t::ioerror);
		} catch(...) {}
	return false;
	}
	bool puts(const char_t* s) noexcept {
		try {
			return (out << s).good();
		} catch(...) {}
		return false;
	}
private:
	std::ostream & out;
};

struct ipaddr {
    uint32_t addr;
	bool read(cojson::details::lexer& in) noexcept;
    bool write(cojson::details::ostream&) const noexcept;
};
struct macaddr {
	static constexpr unsigned len = 6;
	uint8_t addr[len];
	bool read(cojson::details::lexer& in) noexcept;
    bool write(cojson::details::ostream&) const noexcept;
};

//template<typename T, size_t N>
//static inline void assign(T (&a)[N], const T (&v)[N]) noexcept  {
//	for(size_t i=0; i<N; ++i) a[i] = v[i];
//}

bool ipaddr::write(cojson::details::ostream& out) const noexcept {
	using cojson::details::writer;
	return
		out.put('"') &&	writer<uint8_t>::write(0xFF & (addr    ), out) &&
		out.put('.') && writer<uint8_t>::write(0xFF & (addr>> 8), out) &&
		out.put('.') && writer<uint8_t>::write(0xFF & (addr>>16), out) &&
		out.put('.') && writer<uint8_t>::write(0xFF & (addr>>24), out) &&
		out.put('"');
}

bool ipaddr::read(cojson::details::lexer& in) noexcept {
	using namespace cojson;
	using namespace cojson::details;
	char_t buffer[sizeof(ipaddr)*4];
	if(!cojson::details::reader<char_t*>::read(buffer, sizeof(buffer), in))
		return false;
	char_t* curr = buffer;
	unsigned val = 0;
	uint_fast8_t shft = 0;
	uint_fast8_t cnt = 0;

	decltype(addr) adr = 0;
	for(;curr<buffer+sizeof(buffer);++curr) {
		if( +(chartype(*curr) & ctype::digit) ) {
			if( ! tenfold<decltype(val)>(val,*curr-'0') ) return false;
			if( val > 0xFF ) return false;
			++cnt;
		} else {
			if( *curr == 0 || *curr == '.' ) {
				if( cnt == 0 ) return false;
				adr |= static_cast<decltype(addr)>(val) << shft;
				if( *curr == 0 ) {
					addr = adr;
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

static inline bool ashex(uint8_t v, cojson::details::ostream& out) noexcept {
	return out.put(cojson::details::ashex(v >> 4)) &&
		   out.put(cojson::details::ashex(v & 0xF));
}

bool macaddr::write(cojson::details::ostream& out) const noexcept {
	char dlm = '"';
	for(size_t i=0; i<cojson::details::countof(addr); ++i) {
		out.put(dlm);
		ashex(addr[i], out);
		dlm = ':';
	}
	return out.put('"');
}

bool macaddr::read(cojson::details::lexer& in) noexcept {
	using namespace cojson;
	using namespace cojson::details;
	char_t buffer[sizeof(macaddr)*3];
	if(!reader<char_t*>::read(buffer, sizeof(buffer), in))
		return false;
	char_t* curr = buffer;
	unsigned val = 0;
	uint_fast8_t cnt = 0;
	uint_fast8_t pos = 0;
	ctype ctp;

	uint8_t adr[len] = {};
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
				adr[pos++] = val;
				if( *curr == 0 ) {
					assign(addr, adr);
					return true;
				}
				cnt = 0;
				val = 0;
			}
		}
	}
	return false;
}

namespace sample {
/*****************************************************************************/
using namespace micurest;
using cojson::details::countof;
using cojson::cstring;
using cojson::accessor::pointer;

#define NAME(s) static inline constexpr const char* s() noexcept {return #s;}
#define ALIAS(f,s) static inline constexpr const char_t* f() noexcept {return #s;}
namespace name {
	ALIAS(index,index.html)
	NAME(natural)
	NAME(numeric)
	NAME(text)
	NAME(restart)
	NAME(logical)
	NAME(blob)
	NAME(dir)
	NAME(ip)
	NAME(mac)
	NAME(mode)
	NAME(meta)
	NAME(a)
	ALIAS(all,*)
}

enum class mode_t {
	in,
	out,
	pulldown,
	pullup,
	opendrain,
	highdrive,
	__unknown__
};

static inline constexpr int operator+(mode_t v) noexcept {
	return static_cast<int>(v);
}

namespace name {
	NAME(in)
	NAME(out)
	NAME(pulldn)
	NAME(pullup)
	NAME(opendr)
	NAME(highdr)
}

template<mode_t K, enumnames::name V>
struct _ : enumnames::tuple<mode_t, K,V> {};

struct modenames : enumnames::names<mode_t,
	_<mode_t::in,			name::in >,
	_<mode_t::out,          name::out>,
	_<mode_t::pulldown,     name::pulldn>,
	_<mode_t::pullup,       name::pullup>,
	_<mode_t::opendrain,    name::opendr>,
	_<mode_t::highdrive,    name::highdr>,
	_<mode_t::__unknown__,  nullptr  >> {
};

}

namespace cojson {
namespace details {

template<>
struct writer<sample::mode_t> {
	static inline bool write(sample::mode_t mode, ostream& out) noexcept {
		return writer<const char*>::write(sample::modenames::get(mode), out);
	}
};

template<>
struct reader<sample::mode_t> {
	static bool read(sample::mode_t& dst, lexer& in) noexcept {
		char buff[16];
		if( reader<char_t*>::read(buff,countof(buff), in) ) {
			dst = sample::modenames::get(buff);
			return true;
		}
		return false;
	}
};

}}



namespace sample {

// file2c < index.html > index.html.inc
static const char index_html_[] = {
#	include "index.html.inc"
	,0
};

static cstring index_html() noexcept {
	return index_html_;
}

static status_t put_restart(const char* id) noexcept {
	std::cout << __func__ << '/' << id << std::endl;
	return status_t::No_Content;
}

static unsigned natural;
static float numeric;
static bool logical;
char_t text[32]; /* must not be static to be usable in templates */
unsigned char blob[2048]; /* must not be static to be usable in templates */
static unsigned blob_length = 0;
static macaddr mac = {};
static ipaddr ip = {};
unsigned array[32] = {90,80,70,60,50,40,30,20,10}; /* must not be static to be usable in templates */
mode_t mode;

static float get_numeric() noexcept { return numeric; }
static void put_numeric(float v) noexcept { numeric = v; }
using namespace cojson;
using X = cojson::accessor:: array<unsigned, countof(array), array>;

const char* getitem(size_t i) noexcept {
	return "";
}

/* ["in","out",...] */
template<class E>
const cojson::details::value& Enums() noexcept {
	static const cojson::details::strings<E::get> l;
	return l;
}

const directory& root() noexcept {
	return Root<
		E<name::index,
			N<media::text::html, index_html>
		>,
		F<name::natural, pointer<unsigned, &natural>>,
		F<name::numeric, pointer<float, &numeric>>,
		F<name::logical, pointer<bool, &logical>>,
		F<name::mode,   pointer<mode_t, &mode>>,
		F<name::text, countof(text), text>,
		D<name::dir,
			F<name::natural, unsigned, &natural>,
			F<name::numeric, float, get_numeric, put_numeric>,
			F<name::text, countof(text), text>
		>,
		D<name::meta,
			E<name::mode, N<Enums<modenames>>>
		>,
		D<name::a,
			E<name::all, N<cojson::V<unsigned, countof(array), array>>>,
			E<micurest::identity::numeric, N<X, micurest::media::json>>
		>,
		F<name::blob,&blob_length, sizeof(blob), blob>,
		E<name::restart, N<put_restart, 32>>,
		E<name::ip, N<cojson::V<ipaddr, &ip>>>,
		E<name::mac, N<cojson::V<macaddr, &mac>>>
	>();
}

}

int main() {
	class application : public micurest::application, public request_handler {
	public:
		application(const micurest::directory& dir)
		: micurest::application(dir) {}
		bool handle_request(std::iostream& io) {
			istream in(io);
			ostream out(io);
			bool keep = service(in, out) == result_t::keep;
			io.flush();
			return keep;
		 }
	} api(sample::root());

	sample::modenames::get(sample::mode_t::in);

	server rest("0.0.0.0", "8080", api);
	rest.run();
	return 0;
}

namespace enumnames {
	bool match(const char* a, const char* b) noexcept {
		return cojson::details::match(a,b);
	}
}

void dbg(const char *fmt, ...) noexcept {
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
}
