/*
 * user_server.cpp - example of implementing a REST server on esp8266
 *
 * This file is part of µcuREST Library. http://hutorny.in.ua/projects/micurest
 *
 * Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
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

#include "esp8266.hpp"

extern "C" {
#	include "user.h"
}

/*****************************************************************************/
using namespace micurest;
using namespace micurest::network_esp;
using cojson::details::countof;
using cojson::cstring;
using micurest::status_t;
using namespace cojson::accessor;

namespace name {
	/* used in simple examples */
	NAME(natural)
	NAME(text)
	NAME(restart)
	NAME(logical)
	NAME(blob)
	NAME(dir)
	NAME(demo)
	NAME(setup)
	NAME(status)
}

using micurest::details::message;
using micurest::resource::node;

/* Resource map parts defined in other modules								*/
extern const cojson::details::value& esp_sysinfo() noexcept;
extern const node& esp_config_current() noexcept;
extern const cojson::details::value& esp_interfaces() noexcept;
extern const cojson::details::value& esp_rtdata() noexcept;
extern const node& esp_config_modenames() noexcept;

/* Bootstrap HTML template 													*/
struct template_html {
	/* Build procedure for index.html.0.inc files
	 * csplit --suppress-matched --prefix=index.html --suffix-format=.%d index.html '/<!--VIEW-->/' '/<!--DATA-->/'
	 * file2c  < index.html.0 > index.html.0.inc
	 * file2c  < index.html.1 > index.html.1.inc
	 * file2c  < index.html.2 > index.html.2.inc
	 */
	static constexpr const char* head() noexcept { return _0; }
	static constexpr const char* midl() noexcept { return _1; }
	static constexpr const char* tail() noexcept { return _2; }
private:
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

constexpr const char template_html::_0[];
constexpr const char template_html::_1[];
constexpr const char template_html::_2[];

/* TLS encryption bundle  													*/
struct tls_cert {
	static constexpr const unsigned char cert[] = {
#		include "cert.inc"
	};
	static constexpr const unsigned char key[] = {
#		include "key.inc"
	};
} cert;

constexpr const unsigned char tls_cert::cert[];
constexpr const unsigned char tls_cert::key[];


/* sysinfo content															*/
static const node& sysinfo() noexcept {
	return N<esp_sysinfo>();
}

/*	status page																*/
struct status : template_html {
	static constexpr const node& (*data)() = &sysinfo;
	static constexpr const char view[] = "status.html";
};

/*	setup page																*/
struct setup : template_html {
	static constexpr const node& (*data)() = &esp_config_current;
	static constexpr const char view[] = "setup.html";
};

/*	demo page																*/
struct demo : template_html {
	static constexpr const node& (*data)() = &esp_config_modenames;
	static constexpr const char view[] = "demo.html";
};

/*	gpio page																*/
struct gpio : template_html {
	static constexpr const node& (*data)() = &esp_gpio_metadata;
	static constexpr const char view[] = "gpio.html";
};

constexpr const char status::view[];
constexpr const char setup::view[];
constexpr const char demo::view[];
constexpr const char gpio::view[];

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress"
/* custom content provider - HTML template with data and the view page name	*/
template<class C>
const node& Embed() noexcept {
	static const struct local : node {
		media::type mediatype() const noexcept {
			return media::html;
		}
		void get(message& msg) const noexcept {
			msg.set_content_type(media::html);
			msg.obody().puts(C::head());
			cojson::details::writer<const char*>::write(C::view,msg.obody());
			msg.obody().puts(C::midl());
			if( C::data ) C::data().get(msg);
			else msg.obody().puts(cojson::details::literal::null_l());
			msg.obody().puts(C::tail());
		}
	} l;
	return l;
}
#pragma GCC diagnostic pop

/* variables used in demo													*/
static unsigned natural;
static bool logical;
char_t text[32];
unsigned char blob[1280];
unsigned blob_length = 0;
static io::mode_t mode;

static os_timer_t restart_timer;
static void restart_later(void *) {
	system_restart();
}

/* method called when a PUT /reset request comes 							*/
static status_t put_restart(const char* id) noexcept {
    os_timer_setfn(&restart_timer, restart_later, nullptr);
    os_timer_arm(&restart_timer, 100, 0);
	return status_t::No_Content;
}

/* Directory for application available via http								*/
const directory& http_root() noexcept {
	return Root<											/// URI
		E<name::status, Embed<status>>,						/// /status
		E<name::setup, Embed<setup>>,						/// /setup
		E<name::restart, N<put_restart, 32>>,				/// /restart
		D<name::sysinfo,									/// /sysinfo
			E<name::dot, N<esp_sysinfo>>,					/// /sysinfo/.
			E<name::interfaces, N<esp_interfaces>>,			/// /sysinfo/interfaces
			E<name::rtdata, N<esp_rtdata>>					/// /sysinfo/rtdata
		>,
		esp_config_entry,									/// /config
		esp_gpio_meta,										/// /meta
		D<name::gpio,										/// /gpio
			E<name::dot,Embed<gpio>>,						/// /gpio
			esp_gpio_entry,									/// /gpio/<N>
			esp_gpio_all									/// /gpio/*
		>,
		esp_pad_entry,										/// /pad
		esp_port_entry,										/// /port
		D<name::demo,										/// /demo
			E<name::index, Embed<demo>>,					/// /demo/index.html
			F<name::natural, pointer<unsigned, &natural>>,	/// /demo/natural
			F<name::logical, pointer<bool, &logical>>,		/// /demo/logical
			F<name::mode,    pointer<io::mode_t, &mode>>,	/// /demo/mode
			F<name::text,  sizeof(text), text>,				/// /demo/text
			F<name::blob,&blob_length, sizeof(blob), blob>	/// /demo/blob
		>
	>();
}

/* Directory for application available via https							*
 * Note: https does not work reliably in the current implementation			*
 * Perhaps, this will be fixed somedays										*
 */
const directory& https_root() noexcept {
	return Root<
		E<name::index, Embed<demo>>,
		D<name::demo,
			F<name::natural, pointer<unsigned, &natural>>,
			F<name::logical, pointer<bool, &logical>>,
			F<name::mode,    pointer<io::mode_t, &mode>>,
			F<name::text,  sizeof(text), text>,
			F<name::blob,&blob_length, sizeof(blob), blob>
		>
	>();
}

/* micurest applications													*/
static application http_app (http_root());
static application https_app(https_root());
typedef network_esp::tls::server<tls_cert> tlsserv;

static tcp::server server(http_app);
static tlsserv secure(https_app);


/* server initialization
 * server.run is not needed because the transport layer is event-driven
 */
extern "C" void user_server() {
	server.listen(80);
	secure.listen(443);
}

void dbg(const char *fmt, ...) noexcept  {
	va_list args;
	va_start(args, fmt);
	ets_vprintf(ets_putc, fmt, args);
	va_end(args);
}

