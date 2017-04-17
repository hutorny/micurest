/*
 * Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * esp_setup.cpp - Example of  µcuREST API for ESP SDK
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
 * Service config
 * resource [current, default]:
	{
		"sta": {
			"enable": true,
			"autoconnect": true,
			"dhcp": true,
			"config": {
				"ssid": "",
				"password": "",
				"bssid": ""
			},
			"mac": "5C:00:00:00:00:00",
			"ip": {
				"addr": "10.0.0.2",
				"mask": "255.0.0.0",
				"gw": "10.0.0.1"
			},
			"hostname": "ESP_862F0C"
		},
		"ap": {
			"enable": false,
			"config": {
				"ssid": "ESP_862F0C",
				"password": "",
				"channel": 1,
				"maxcon": 4,
				"hidden": false,
				"beacon_interval": 100
			},
			"ip": {
				"addr": "0.0.0.0",
				"mask": "0.0.0.0",
				"gw": "0.0.0.0"
			},
			"mac": "5E:00:00:00:00:00",
			"dhcps": {
				"enable": true,
				"leasetime": 120,
				"range": {
					"enable": true,
					"start": "192.168.0.2",
					"end": "192.168.0.254"
				}
			}
		},
		"sntp": {
			"enable": true,
			"server": ["0.pool.ntp.org","1.pool.ntp.org"],
		}
	}
 */

#include "esp8266.hpp"
extern "C" {
#	include "user.h"
}


using namespace cojson;
using details::countof;
using details::clas;

struct esp_config {
	enum class type : bool {
		current, persisted
	};
	inline esp_config() noexcept { }
	inline void init(type t) noexcept {
		sta.init(t);
		ap.init(t);
		sntp.init(t);
	}
	inline int apply(type t) noexcept {
		return sta.apply(t) | ap.apply(t) | sntp.apply(t);
	}
	struct sta {
		station_config config; //ssid, password, bssid
		ip_info ip;
		mac_addr mac;
		char hostname[32];
		bool dhcp;
		bool enable;
		bool autoconnect;
		struct has_bssid {
			static bool has(const station_config& cfg) noexcept { return cfg.bssid_set; }
			static bool has(station_config& cfg, bool has) noexcept { return (cfg.bssid_set = has); }
		};
		static bool has(const sta& s) noexcept { return s.mac.addr[0]; }
		static bool has(sta& cfg, bool) noexcept { return true; }
		void init(type);
		int apply(type);
	public:
		static const clas<sta>& structure() noexcept {
			return
				O<sta,
					P<sta, name::enable, decltype(sta::enable), &sta::enable>,
					P<sta, name::autoconnect, decltype(sta::autoconnect), &sta::autoconnect>,
					P<sta, name::dhcp, decltype(sta::dhcp), &sta::dhcp>,
					P<sta, name::config, decltype(sta::config), &sta::config,
						O<station_config,
							S<station_config, name::ssid, uint8, countof(&station_config::ssid), &station_config::ssid>,
							S<station_config, name::password, uint8, countof(&station_config::password), &station_config::password>,
							X<station_config, name::bssid, mac_addr, decltype(station_config::bssid), &station_config::bssid, has_bssid>
						>
					>,
					X<sta, name::mac, mac_addr, decltype(sta::mac), &sta::mac, sta>,
					P<sta, name::ip, decltype(sta::ip), &sta::ip, ip_structure>,
					S<sta, name::hostname, countof(&sta::hostname), &sta::hostname, wifi_station_get_hostname>
				>();
		}
	} sta;
	struct ap {
		softap_config  config;  //ssid, password, channel, auth mode, hidden, ...
		ip_info ip;
		mac_addr mac;
		struct dhcps {
			dhcps_lease range;
			uint32 leasetime;
			bool enable;
			void init(type);
			int apply(type);
			static const clas<dhcps>& structure() noexcept {
				return
					O<struct ap::dhcps,
						P<struct ap::dhcps, name::enable, decltype(dhcps::enable), &dhcps::enable>,
						P<struct ap::dhcps, name::leasetime, decltype(dhcps::leasetime), &dhcps::leasetime>,
						P<struct ap::dhcps, name::range, decltype(ap::dhcps::range), & ap::dhcps::range,
							O<decltype(ap::dhcps::range),
								P<dhcps_lease, name::enable, decltype(dhcps_lease::enable), &dhcps_lease::enable>,
								P<dhcps_lease, name::start, decltype(dhcps_lease::start_ip), &dhcps_lease::start_ip>,
								P<dhcps_lease, name::end, decltype(dhcps_lease::end_ip), &dhcps_lease::end_ip>
							>
						>
					>();
			}

		} dhcps;
		bool enable;
		void init(type);
		int apply(type);

		static bool has(const ap& s) noexcept { return s.mac.addr[0]; }
		static bool has(ap& cfg, bool) noexcept { return true; }
	public:
		static const clas<ap>& structure() noexcept {
			return
				O<ap,
					P<ap, name::enable, decltype(ap::enable), &ap::enable>,
					P<ap, name::config, decltype(ap::config), &ap::config,
						O<softap_config,
							S<softap_config, name::ssid, uint8, countof(&softap_config::ssid), &softap_config::ssid>,
							S<softap_config, name::password, uint8, countof(&softap_config::password), &softap_config::password>,
							P<softap_config, name::channel, decltype(softap_config::channel), &softap_config::channel>,
							P<softap_config, name::maxcon, decltype(softap_config::max_connection), &softap_config::max_connection>,
							X<softap_config, name::hidden, bool, decltype(softap_config::ssid_hidden), &softap_config::ssid_hidden>,
							P<softap_config, name::beacon_interval, decltype(softap_config::beacon_interval), &softap_config::beacon_interval>
					>>,
					P<ap, name::ip, decltype(ap::ip), &ap::ip, ip_structure>,
					X<ap, name::mac, mac_addr, decltype(ap::mac), &ap::mac, ap>,
					P<ap, name::dhcps, decltype(ap::dhcps), &ap::dhcps, ap::dhcps::structure>
				>();
		}
	} ap;
	static struct sntp {
		static constexpr size_t server_name_len = 24;
		static constexpr size_t server_count    = 3;
		char server[server_count][server_name_len];
		bool enable;
		void init(type t) noexcept;
		int apply(type t) noexcept;
		static const clas<sntp>& structure() noexcept {
			return
				O<sntp,
					P<sntp, name::enable, bool, &sntp::enable>,
					P<sntp, name::server, server_count, server_name_len, &sntp::server>
				>();
		}
	} sntp;
	static const clas<esp_config>& structure() noexcept {
		return
			O<esp_config,
				P<esp_config, name::sta, decltype(sta), &esp_config::sta, esp_config::sta::structure>,
				P<esp_config, name::ap, decltype(ap), &esp_config::ap, esp_config::ap::structure>,
				P<esp_config, name::sntp, V<accessor::pointer<decltype(sntp), &esp_config::sntp>, esp_config::sntp::structure>>
			>();
	}
};

decltype(esp_config::sntp) esp_config::sntp __attribute__((section(".persistent")));

using namespace micurest;
using micurest::details::message;

template<class C, typename T>
struct relay_node : micurest::resource::node {
	media::type mediatype() const noexcept {
		return media::json;
	}
	virtual T getswitch() const noexcept = 0;
	void get(message& msg) const noexcept {
		msg.status(status_t::OK);
		C obj = {};
		obj.init(getswitch());
		C::structure().write(obj, msg.obody());
	}
	void put(message& msg) const noexcept {
		if( msg.req().content_type == mediatype() ) {
			C obj = {};
			cojson::lexer in(msg.ibody());
			if( C::structure().read(obj, in) ) {
				int res = obj.apply(getswitch());
				if( res == 0 )
					msg.status(status_t::No_Content); //FIXME or status_t::Accepted
				else {
					msg.status(status_t::Bad_Request);
					json_error{"Apply failed", res}.write(msg.obody());
				}
				return;
			}
		}
		msg.bad_content();
	}
};

template<class C, esp_config::type Y = esp_config::type::current>
const node& esp_config_node() noexcept {
	static const struct local : relay_node<C,esp_config::type> {
		esp_config::type getswitch() const noexcept { return Y; }
	} l;
	return l;
}

static esp_config current;

const node& esp_config_current() noexcept {
	return esp_config_node<esp_config, esp_config::type::current>();
}

const entry& esp_config_entry() noexcept {
	return
		D<name::config,
			//TODO add html redirect to /setup
			E<name::current, esp_config_node<esp_config, esp_config::type::current>>,
			E<name::_default, esp_config_node<esp_config, esp_config::type::persisted>>,
			D<name::ap,
				E<name::current, esp_config_node<struct esp_config::ap, esp_config::type::current>>,
				E<name::_default, esp_config_node<struct esp_config::ap, esp_config::type::persisted>>,
				E<name::dhcps, esp_config_node<struct esp_config::ap::dhcps, esp_config::type::current>>
			>,
			D<name::sta,
				E<name::current, esp_config_node<struct esp_config::sta, esp_config::type::current>>,
				E<name::_default, esp_config_node<struct esp_config::sta, esp_config::type::persisted>>
			>,
			D<name::sntp,
				E<name::current, esp_config_node<struct esp_config::sntp, esp_config::type::current>>
			>,
			esp_config_gpio,
			esp_config_port,
			esp_config_pad
		>();
}

template<typename T>
static inline void setbit(T& i, T m, bool v) noexcept {
	i = (i & ~m) | (v ? m : 0);
}

static inline uint8 wifi_get_opmode(esp_config::type type) noexcept {
	return type == esp_config::type::current
		? wifi_get_opmode() : wifi_get_opmode_default();
}

static inline bool wifi_set_opmode(esp_config::type type, uint8 mode, bool set) noexcept {
	uint8 m = wifi_get_opmode(type);
	setbit(m, mode, set);
	return type == esp_config::type::current
		? wifi_set_opmode_current(m) : wifi_set_opmode(m);
}

void esp_config::ap::init(esp_config::type type) noexcept {
	enable = SOFTAP_MODE & wifi_get_opmode(type);
	if( type == esp_config::type::current )
		wifi_softap_get_config(&config);
	else
		wifi_softap_get_config_default(&config);
	//FIXME wifi_get_channel
	dhcps.init(type);
	wifi_get_ip_info(SOFTAP_IF, &ip);
	wifi_get_macaddr(SOFTAP_IF, mac.addr);
}


void esp_config::sta::init(esp_config::type type) noexcept {
	enable = STATION_MODE & wifi_get_opmode(type);
	if( type == esp_config::type::current )
		wifi_station_get_config(&config);
	else
		wifi_station_get_config_default(&config);
	dhcp = wifi_station_dhcpc_status();
	autoconnect = wifi_station_get_auto_connect();
	wifi_get_ip_info(STATION_IF, &ip);
	wifi_get_macaddr(STATION_IF, mac.addr);
}


void esp_config::ap::dhcps::init(esp_config::type) noexcept {
	enable = wifi_softap_dhcps_status();
	wifi_softap_get_dhcps_lease(&range);
	leasetime = wifi_softap_get_dhcps_lease_time();
}

int esp_config::ap::apply(esp_config::type type) noexcept {
	int res = 0;
	setbit(res, 1,
		! wifi_set_opmode(type, SOFTAP_MODE, enable));
	if( enable )
		setbit(res, 2, type == esp_config::type::current
			? ! wifi_softap_set_config_current(&config)
			: ! wifi_softap_set_config(&config));
	setbit(res, 4,
		! wifi_set_ip_info(SOFTAP_IF, &ip));
	if ( mac.addr[0] )
		setbit(res, 4,
			! wifi_set_macaddr(SOFTAP_IF, mac.addr));
	return res | dhcps.apply(type);
}

int esp_config::ap::dhcps::apply(esp_config::type) noexcept {
	int res = 0;
	wifi_softap_dhcps_stop();
	setbit(res, 0x20,
		! wifi_softap_set_dhcps_lease(&range));
	if( enable ) {
		setbit(res, 0x10, ! wifi_softap_dhcps_start());
		if( leasetime > 0 &&  leasetime <= 2880 )
			wifi_softap_set_dhcps_lease_time(leasetime);
	}
	return res;
}


int esp_config::sta::apply(esp_config::type type) noexcept {
	int res = 0;
	setbit(res, 0x100,
		!wifi_set_opmode(type, STATION_MODE, enable));
	setbit(res, 0x101,
		type == esp_config::type::current
			? ! wifi_station_set_config_current(&config)
				: ! wifi_station_set_config(&config));
	setbit(res, 0x102,
			dhcp ? wifi_station_dhcpc_start() : wifi_station_dhcpc_stop());
	setbit(res, 0x104,
			! wifi_station_set_auto_connect(autoconnect));
	setbit(res, 0x108,
		! wifi_set_ip_info(STATION_IF, &ip));
	setbit(res, 0x110,
		! wifi_set_macaddr(STATION_IF, mac.addr));
	return res;
}

void esp_config::sntp::init(esp_config::type t) noexcept {
	if( this == & esp_config::sntp ) return;
	*this = esp_config::sntp;
}

int esp_config::sntp::apply(esp_config::type t) noexcept {
	if( ! enable )
		sntp_stop();
	else {
		sntp_init();
		if( server[0][0] ) sntp_setservername(0, server[0]);
		if( server[1][0] ) sntp_setservername(1, server[1]);
		if( server[2][0] ) sntp_setservername(2, server[2]);
	}
	if( t == esp_config::type::persisted ) {
		user_save_persistent();
	}
	return 0;
}

void user_sntp_init(void) {
	esp_config::sntp.apply(esp_config::type::current);
}
