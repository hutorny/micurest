/*
 * Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * esp_sysinfo.cpp - Example of binding ESP SDK functions to COJSON
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

#include "esp8266.hpp"
extern "C" {
#	include "user.h"
}

//TODO return rtcock in us by multiplying on result of system_rtc_clock_cali_proc

using namespace cojson;
using details::value;

template<typename T, size_t N, typename V>
static inline void assign(T (&a)[N], V v) noexcept  {
	for(size_t i=0; i<N; ++i) a[i] = v;
}


struct if_info {
	template<uint8 IF>
	static const value& info() noexcept {
		return V<accessor::function<ip_info,get<IF>>,
				O<ip_info,
					P<ip_info, name::addr,   ip_addr, &ip_info::ip>,
					P<ip_info, name::mask, ip_addr, &ip_info::netmask>,
					P<ip_info, name::gw, ip_addr, &ip_info::gw>,
					X<ip_info, name::mac, mac_addr, if_info::getmac<IF>>
				>>();
	}
	template<uint8 IF>
	static ip_info* get() noexcept {
		buff = {0};
		return wifi_get_ip_info(IF, &buff) ? &buff : nullptr;
	}
	template<uint8 IF>
	static mac_addr* getmac() noexcept {
//		if(n == 0) {
		assign(mac.addr,0);
		return( wifi_get_macaddr(IF,mac.addr) ) ? &mac : nullptr;
//		}
//		return n < details::countof(mac) ? mac+n : nullptr;
	}
private:
	static mac_addr mac;
	static ip_info buff;
};

mac_addr if_info::mac = {0};
ip_info if_info::buff = {0};

const value& esp_interfaces() noexcept {
	return V<
		M<name::sta, if_info::info<STATION_IF>>,
		M<name::ap,  if_info::info<SOFTAP_IF>>>();
}

static uint32 get_current_timestamp() noexcept {
	return sntp_getservername(0) ? sntp_get_current_timestamp() : 0;
}

static inline void dummy(uint32) noexcept {}
static inline void dummy(uint16) noexcept {}
static inline void dummy(uint8) noexcept {}
static inline const char* get_host_name() noexcept {
	return wifi_station_get_hostname();
}

const value& esp_rtdata() noexcept {
	return V<
		M<name::free_heap_size, V<uint32, system_get_free_heap_size, dummy>>,
		M<name::time, V<uint32, system_get_time, dummy>>,
		M<name::rtclock, V<uint32, system_get_rtc_time, dummy>>,
		M<name::sntptime, V<uint32, get_current_timestamp, dummy>>>();
}


const value& esp_sysinfo() noexcept {
	return V<
		M<name::sdk_version, system_get_sdk_version>,
		M<name::chip_id,V<uint32, system_get_chip_id, dummy>>,
		M<name::vdd33, V<uint16, system_get_vdd33, dummy>>,
		M<name::boot_version, V<uint8, system_get_boot_version, dummy>>,
		M<name::boot_mode, V<uint8, system_get_boot_mode, dummy>>,
		M<name::cpu_freq, V<uint8, system_get_cpu_freq, dummy>>,
		M<name::wifi_mode, V<uint8, wifi_get_opmode, dummy>>,
		M<name::hostname, get_host_name>,
		M<name::interfaces, esp_interfaces>,
		M<name::rtdata, esp_rtdata>
	>();
}




//os_install_putc1
