/*
 * Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * user_server.cpp - example of implementing a REST server on Photon
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
#include "micurest/network_spark_socket.hpp"
#include "micurest/enumnames.hpp"
#include "utils.hpp"
#include <application.h>

using namespace micurest;
using namespace cojson::accessor;
using namespace network_spark_socket;
using cojson::details::countof;
using cojson::V;
using cojson::M;

using micurest::details::entry;

/* RPC-related resources from photon_wiring_rpc.cpp							*/

extern const entry& rpcc_entry() noexcept;
extern const entry& rpc_entry() noexcept;
extern const entry& meta_entry() noexcept;


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
	using mode_t = PinMode;

	/* Enum names  															*/
	ENUM(INPUT)
	ENUM(OUTPUT)
	ENUM(INPUT_PULLUP)
	ENUM(INPUT_PULLDOWN)
	ENUM(AF_OUTPUT_PUSHPULL)
	ENUM(AF_OUTPUT_DRAIN)
	ENUM(AN_INPUT)
	ENUM(AN_OUTPUT)

	/* a shortcut for map definition 										*/
	template<mode_t K, enumnames::name V>
	struct _ : enumnames::tuple<mode_t, K,V> {};

	/* map of literals to enum values 										*/
	struct mode_names : enumnames::names<mode_t,
		_<mode_t::INPUT,				INPUT>,
		_<mode_t::OUTPUT,				OUTPUT>,
		_<mode_t::INPUT_PULLUP,     	INPUT_PULLUP>,
		_<mode_t::INPUT_PULLDOWN,   	INPUT_PULLDOWN>,
		_<mode_t::AF_OUTPUT_PUSHPULL,	AF_OUTPUT_PUSHPULL>,
		_<mode_t::AF_OUTPUT_DRAIN,		AF_OUTPUT_DRAIN>,
		_<mode_t::AN_INPUT,				AN_INPUT>,
		_<mode_t::AN_OUTPUT,			AN_OUTPUT>,
		_<mode_t::PIN_MODE_NONE,		nullptr>> {
	};

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
		char buff[20];
		if( reader<char_t*>::read(buff,countof(buff), in) ) {
			dst = enums::mode_names::get(buff);
			return true;
		}
		return false;
	}
};
}}

/****************************************************************************/
/* functions for validating index of I/O pins								*/
/****************************************************************************/
static bool digital_has(size_t index) noexcept {
	return index <= D7;
}

static bool analog_has(size_t index) noexcept {
	return index < TOTAL_ANALOG_PINS;
}

/****************************************************************************/
/* functions for reading/writing data from/to I/O pins						*/
/****************************************************************************/
static byte digital_read(size_t index) noexcept {
	return HAL_GPIO_Read(index);
}

static void digital_write(size_t index, byte val) noexcept {
	HAL_GPIO_Write(index, val);
}

static int analog_read(size_t index) noexcept {
	return HAL_ADC_Read(FIRST_ANALOG_PIN+index);
}

/** definition of a JSON object with all I/O pins 							*/
const cojson::details::value& json_object() noexcept {
	return												/// JSON object:
		V<												/// {
			M<name::d,									///   "d" :
				V<micurest::accessor::vector<byte, 		///			[..],
					digital_read, digital_write, digital_has>>>,
			M<name::a,									///   "a" :
				V<micurest::accessor::bunch<int,		///     	[..],
					analog_read, analog_has>>>
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
constexpr const char template_html::_0[];
constexpr const char template_html::_1[];
constexpr const char template_html::_2[];

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

constexpr const char demo::view[];
constexpr const char io::view[];

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
			cojson::Write(cstring(C::view),msg.obody());
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
static unsigned blob_length = 0;
static enums::mode_t mode;
static ip_addr ip;
static mac_addr mac;

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
		rpc_entry,
		rpcc_entry,
		meta_entry,
		/* directory d	 													*/
		D<name::d,										/// /d
			E<name::X,									/// /d/*
				N<V<micurest::accessor::vector<byte,
					digital_read, digital_write, digital_has>>>>,
			E<identity::numeric,						/// /d/0 .. /d/11
				N<micurest::accessor::vector<byte,
					digital_read, digital_write, digital_has>,media::json>>>,
		/* directory a	 													*/
		D<name::a,										/// /a
			E<name::X,									/// /a/*
				N<V<micurest::accessor::bunch<int,
					analog_read, analog_has>>>>,
			E<identity::numeric,						/// /a/0 .. /a/9
				N<micurest::accessor::bunch<int,
					analog_read, analog_has>, media::json>>>,
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

static application rest(root());
/* instance of a tcp server													*/
tcp::server server(rest);

void report_ip() noexcept {
	WLanConfig config;
	wlan_fetch_ipconfig(&config);
	ip = config.nw.aucIP.ipv4;
	cojson::details::assign(mac.addr,config.BSSID);
	Serial1.print("my IP: "); /* welcome to Ardiuno world     */
	Serial1.print((uint32_t)(config.nw.aucIP.ipv4 >> 24) & 0xFF);
	Serial1.print('.');
	Serial1.print((uint32_t)(config.nw.aucIP.ipv4 >> 16) & 0xFF);
	Serial1.print('.');
	Serial1.print((uint32_t)(config.nw.aucIP.ipv4 >>  8) & 0xFF);
	Serial1.print('.');
	Serial1.println((uint32_t)(config.nw.aucIP.ipv4)     & 0xFF);

}

static bool done;

void server_run() noexcept {
	if( done ) {
		server.run();
	}
	else {
		/* it takes some time for wifi getting up and running,
		 * so here we try to start server and report ip when done */
		done = server.listen(80);
		if( done ) report_ip();
		else delay(100);
	}
}
/* Minimal example of 4 entries including 1 html page:
Program:   21644 bytes (8.3% Full)
Data:       1571 bytes (19.2% Full)
*/

/* setting pin modes for this example										*/
void io_setup() noexcept {
	for(uint8_t i =0; i <= D7; ++i) {
		HAL_Pin_Mode(i, PinMode::OUTPUT);
	}
	for(uint8_t i=FIRST_ANALOG_PIN; i<FIRST_ANALOG_PIN+TOTAL_ANALOG_PINS; ++i){
		HAL_Pin_Mode(i, PinMode::AN_INPUT);
	}
}

namespace enumnames {
/* implementing name matcher enum types										*/
	bool match(const char* a, const char* b) noexcept {
		return cojson::details::match(a,b);
	}
}
