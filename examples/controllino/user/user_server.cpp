/*
 * Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * user_server.cpp - example of implementing a REST server on Controllino
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

/*
 * This file provides implementation progmem methods and placements for
 * promem literals
 */
#include "network_arduino.hpp"
#include "cojson_helpers.hpp"
#include "enumnames.hpp"
#include "utils.hpp"

using namespace micurest;
using namespace cojson::accessor;
using namespace network;
using namespace network_arduino;
using cojson::details::progmem;
using cojson::details::countof;
using cojson::V;
using cojson::M;

namespace name {
	/* names/identifiers used in this demo
	 * string literals are placed in progmem with macro NAME or ALIAS  		*/
	NAME(natural)
	NAME(text)
	NAME(logical)
	NAME(demo)
	NAME(io)
	NAME(a)
	NAME(r)
	NAME(d)
	NAME(in)
	NAME(ip)
	NAME(mac)
	NAME(blob)
	NAME(mode)
	NAME(modes)
	NAME(rtc)
	ALIAS(X,*)
}

namespace enums {

	/* An enum defining pin modes. In this example it is used solely 		*
	 * for demo purposes  													*/
	enum class mode_t : unsigned char {
		na, /* N/A if a pin is controlled by peripheral */
		in,
		out,
		pullup,
		__unknown__
	};

	/* Enum names (for simplicity are placed in RAM with macro ENUM) 		*/
	ENUM(na)
	ENUM(in)
	ENUM(out)
	ENUM(pullup)

	/* a shorcut for map definition 										*/
	template<mode_t K, enumnames::name V>
	struct _ : enumnames::tuple<mode_t, K,V> {};

	/* map of literals to enum values 										*/
	struct mode_names : enumnames::names<mode_t,
		_<mode_t::na,			na >,
		_<mode_t::in,			in >,
		_<mode_t::out,			out>,
		_<mode_t::pullup,		pullup>,
		_<mode_t::__unknown__,	nullptr  >> {
	};

	/* cojson macro generating array of strings from map E					*/
	template<class E>
	const cojson::details::value& list() noexcept {
		static const cojson::details::strings<E::get> l;
		return l;
	}

}

namespace cojson {
namespace details {
/* implementation of custom cojson writer for enum mode_t					 */
template<>
struct writer<enums::mode_t> {
	static inline bool write(enums::mode_t val, ostream& out) noexcept {
		return writer<const char*>::write(enums::mode_names::get(val), out);
	}
};

/* implementation of custom cojson reader for enum mode_t					*/
template<>
struct reader<enums::mode_t> {
	static bool read(enums::mode_t& dst, lexer& in) noexcept {
		char buff[8];
		if( reader<char_t*>::read(buff,countof(buff), in) ) {
			dst = enums::mode_names::get(buff);
			return true;
		}
		return false;
	}
};
}}

/* data type for reading data of byte type from progmem						*/
typedef progmem<byte> cbyte;

namespace mapper {
/* map of digital outputs													*/
static const byte digital[] __attribute__((progmem)) = {
	CONTROLLINO_D0, CONTROLLINO_D1, CONTROLLINO_D2,
	CONTROLLINO_D3, CONTROLLINO_D4, CONTROLLINO_D5,
	CONTROLLINO_D6, CONTROLLINO_D7, CONTROLLINO_D8,
	CONTROLLINO_D9, CONTROLLINO_D10,CONTROLLINO_D11
};

/* map of relay outputs														*/
static const byte relay[] __attribute__((progmem)) = {
	CONTROLLINO_R0, CONTROLLINO_R1, CONTROLLINO_R2,
	CONTROLLINO_R3, CONTROLLINO_R4, CONTROLLINO_R5,
	CONTROLLINO_R6, CONTROLLINO_R7, CONTROLLINO_R8,
	CONTROLLINO_R9
};

/* map of analog inputs														*/
static const byte analog[] __attribute__((progmem)) = {
	CONTROLLINO_A0, CONTROLLINO_A1, CONTROLLINO_A2,
	CONTROLLINO_A3, CONTROLLINO_A4, CONTROLLINO_A5,
	CONTROLLINO_A6, CONTROLLINO_A7, CONTROLLINO_A8,
	CONTROLLINO_A9
};

/* map of interrupt inputs													*/
static const byte interrupt[] __attribute__((progmem)) = {
	CONTROLLINO_IN0, CONTROLLINO_IN1
};
}

using cojson::details::countof;
/****************************************************************************/
/* functions for validating index of I/O pins, used								*/
/****************************************************************************/
static bool digital_has(size_t index) noexcept {
	return index < countof(mapper::digital);
}

static bool interrupt_has(size_t index) noexcept {
	return index < countof(mapper::interrupt);
}

static bool relay_has(size_t index) noexcept {
	return index < countof(mapper::relay);
}

static bool analog_has(size_t index) noexcept {
	return index < countof(mapper::analog);
}

/****************************************************************************/
/* functions for reading/writing data from/to I/O pins						*/
/****************************************************************************/
static byte digital_read(size_t index) noexcept {
	return digitalRead(cbyte(mapper::digital)[index]);
}

static void digital_write(size_t index, byte val) noexcept {
	digitalWrite(cbyte(mapper::digital)[index], val);
}

static byte relay_read(size_t index) noexcept {
	return digitalRead(cbyte(mapper::relay)[index]);
}

static void relay_write(size_t index, byte val) noexcept {
	digitalWrite(cbyte(mapper::relay)[index], val);
}

static int analog_read(size_t index) noexcept {
	return analogRead(cbyte(mapper::relay)[index]);
}

static byte interrupt_read(size_t index) noexcept {
	return digitalRead(cbyte(mapper::interrupt)[index]);
}

/** definition of a JSON object with all I/O pins 							*/
const cojson::details::value& json_object() noexcept {
	return												/// JSON object:
		V<												/// {
			M<name::d,									///   "d" :
				V<micurest::accessor::vector<byte, 		///			[..],
					digital_read, digital_write, digital_has>>>,
			M<name::r,									///   "r" :
				V<micurest::accessor::vector<byte, 		///     	[..],
					relay_read, relay_write, relay_has>>>,
			M<name::a,									///   "a" :
				V<micurest::accessor::bunch<int,		///     	[..],
					analog_read, analog_has>>>,
			M<name::in,									///   "in":
				V<micurest::accessor::bunch<byte,		///     	[..],
					interrupt_read, interrupt_has>>>
		>();											/// }
}

using micurest::resource::node;

/** definition of a micurest resource associated with json_object			 */
const node& json_node() noexcept {
	return N<json_object>();
}

/** definition of a micurest resource associated with list of mode names	 */
const node& modes_node() noexcept {
	return N<enums::list<enums::mode_names>>();
}
/** a generic bootstrap HTML template split on three parts  				*/
struct template_html {
	static constexpr cstring head() noexcept { return cstring(_0); }
	static constexpr cstring midl() noexcept { return cstring(_1); }
	static constexpr cstring tail() noexcept { return cstring(_2); }
private:
	/* Build procedure for index.html.?.inc files
	 * csplit -s --suppress-matched --prefix=index.html --suffix-format=.%d index.html '/^<!--SPLIT-->/' '{2}'
	  file2c  < index.html.0 > index.html.0.inc
	  file2c  < index.html.1 > index.html.1.inc
	  file2c  < index.html.2 > index.html.2.inc
	 */
	static constexpr const char _0[] = {
#		include "index.html.0.inc"
		,0
	};
	static constexpr const char _1[] = {
#		include "index.html.1.inc"
		,0
	};
	static constexpr const char _2[] = {
#		include "index.html.2.inc"
		,0
	};
};

/* HTML template storage set to progmem  									*/
constexpr const char template_html::_0[] __attribute__((progmem));
constexpr const char template_html::_1[] __attribute__((progmem));
constexpr const char template_html::_2[] __attribute__((progmem));

/* demo HTML page 															*/
struct demo : template_html {
	/* data field points to a node that provides content to be served 		*/
	/* within the bootstrap page - list of mode_t values					*/
	static constexpr const node& (*data)() = &modes_node;
	/* view points to an html page that will be loaded from the cloud 		*/
	static constexpr const char view[] = "demo.html";
};

/* io HTML page 															*/
struct io : template_html {
	/* data field points to a node that provides content to be served 		*/
	/* within the bootstrap page - all pins as one object					*/
	static constexpr const node& (*data)() = &json_node;
	static constexpr const char view[] = "io.html";
};

constexpr const char demo::view[] __attribute__((progmem));
constexpr const char io::view[] __attribute__((progmem));

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress"

/* custom micurest node for generating HTML content from class C			*/
template<class C>
const node& Embed() noexcept {
	static const struct local : node {
		/* media type is HTML, so micurest may serve is a a default
		 * content for a directory request coming from browser, such as /	*/
		media::type mediatype() const noexcept {
			return media::html;
		}
		/* get handles GET request 											*/
		void get(message& msg) const noexcept {
			msg.set_content_type(mediatype());	/* setting content type		*/
			msg.obody().puts(C::head());		/* writing template head	*/
			/* writing template's view as a JSON value						*/
			details::writer<cstring>::write(cstring(C::view),msg.obody());
			msg.obody().puts(C::midl());		/* writing template middle	*/
			if( C::data ) C::data().get(msg);	/* writing embedded data 	*/
			else msg.obody().puts(cojson::details::literal::null_l());
			msg.obody().puts(C::tail());		/* writing template tail 	*/
		}
		/* put is not implemented by design									*/
	} l;
	return l;
}
#pragma GCC diagnostic pop

/* variables used on the demo page */
static unsigned natural;
static bool logical;
char_t text[32];
unsigned char blob[1400];
unsigned blob_length = 0;
enums::mode_t mode;
extern ip_addr ip;
extern mac_addr mac;

/* map of URIs to application resources
 * D defines a directory,
 * E defines an entry with a name and content node
 * N defines a content node
 * F combines E and N in one for simple content, such as variables
 */
const directory& root() noexcept {
	return Root<		/// this notation documents URI of the resource
		/*		cost of adding another page: 190/28							*/
		/* first entry that matches accepted content is served by default	*/
		E<name::io,   Embed<io>>, 						/// /io
		/* directory d	 													*/
		D<name::d,										/// /d
			E<name::X,									/// /d/*
				N<V<micurest::accessor::vector<byte,
					digital_read, digital_write, digital_has>>>>,
			E<identity::numeric,						/// /d/0 .. /d/11
				N<micurest::accessor::vector<byte,
					digital_read, digital_write, digital_has>>>>,
		/* directory r	 													*/
		D<name::r,										/// /r
			E<name::X,									/// /r/*
				N<V<micurest::accessor::vector<byte,
					relay_read, relay_write, relay_has>>>>,
			E<identity::numeric,						/// /r/0 .. /r/9
				N<micurest::accessor::vector<byte,
					relay_read, relay_write, relay_has>>>>,
		/* directory a	 													*/
		D<name::a,										/// /a
			E<name::X,									/// /a/*
				N<V<micurest::accessor::bunch<int,
					analog_read, analog_has>>>>,
			E<identity::numeric,						/// /a/0 .. /a/9
				N<micurest::accessor::bunch<int,
					analog_read, analog_has>>>>,
		/* directory in	 													*/
		D<name::in,										/// /in
			E<name::X,									/// /in/*
				N<V<micurest::accessor::bunch<byte,
					interrupt_read, interrupt_has>>>>,
			E<identity::numeric,						/// /in/0 .. /in/1
				N<micurest::accessor::bunch<byte,
					interrupt_read, interrupt_has>>>>,
		E<name::X, json_node>,							/// /*
		F<name::blob,&blob_length, sizeof(blob), blob>,	/// /blob
		F<name::mode, pointer<enums::mode_t, &mode>>,	/// /mode
		E<name::modes, 									/// /modes
			N<enums::list<enums::mode_names>>>,
		E<name::ip, N<V<ip_addr, &ip>>>,				/// /ip
		E<name::mac, N<V<mac_addr, &mac>>>,				/// /mac
		E<name::rtc, Object<datetime>>,					/// /rtc
		/* if all above dropped size is: 22174/1715 						*/
		E<name::demo, Embed<demo>>,						/// /demo
		F<name::natural, pointer<unsigned, &natural>>,	/// /natural
		F<name::logical, pointer<bool, &logical>>,		/// /logical
		F<name::text, countof(text), text>				/// /text
	>();
}
application rest(root());
using network_arduino::proto::tcp;
/* instance of a tcp server													*/
tcp server(80);

void server_listen() noexcept {
	::server.listen();
}

void server_run() noexcept {
	if( ::server.accept() ) {
		tcp::connection client(::server);
		network::proto::http::service(client, rest);
	}
}
/* Minimal example of 4 entries including 1 html page:
Program:   21644 bytes (8.3% Full)
Data:       1571 bytes (19.2% Full)
*/

/* setting pin modes for this example										*/
void io_setup() noexcept {
	for(uint8_t i =0; i < countof(mapper::digital); ++i) {
		pinMode(cbyte(mapper::digital)[i], OUTPUT);
	}
	for(uint8_t i =0; i < countof(mapper::relay); ++i) {
		pinMode(cbyte(mapper::relay)[i], OUTPUT);
	}
	for(uint8_t i =0; i < countof(mapper::analog); ++i) {
		pinMode(cbyte(mapper::analog)[i], INPUT);
	}
	for(uint8_t i =0; i < countof(mapper::interrupt); ++i) {
		pinMode(cbyte(mapper::interrupt)[i], INPUT);
	}
}

namespace cojson {
namespace details {
/* implementing progmem reader for the pin mappers							*/
template<>
byte progmem<byte>::read(const byte * ptr) noexcept {
	return pgm_read_byte(ptr);
}

}}
namespace enumnames {
/* implementing name matcher enum types										*/
	bool match(const char* a, const char* b) noexcept {
		return cojson::details::match(a,b);
	}
}
