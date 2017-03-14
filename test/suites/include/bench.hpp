/*
 * Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * bench.hpp - cojson tests, header for benchmarking tests
 *
 * This file is part of COJSON Library. http://hutorny.in.ua/projects/cojson
 *
 * The COJSON Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License v2
 * as published by the Free Software Foundation;
 *
 * The COJSON Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the COJSON Library; if not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>.
 */

#ifndef BENCH_HPP_
#define BENCH_HPP_
#include "test.hpp"
namespace cojson {
namespace test {
typedef unsigned char ip_t[4];

struct ip4_t {
	unsigned char byte[4];
	inline bool write(ostream& out, size_t i) const noexcept {
		writer<unsigned char>::write(byte[i], out);
		return i < sizeof(byte) - 1;
	}
	inline bool read(lexer& in, size_t i) const noexcept {
		reader<unsigned char>::read(const_cast<ip4_t*>(this)->byte[i], in);
		return i < sizeof(byte) - 1;
	}
	inline void clear() noexcept {
		byte[0] = 0;
		byte[1] = 0;
		byte[2] = 0;
		byte[3] = 0;
	}
};

struct Config {
	struct Name {
		NAME(wan)
		NAME(membuffers)
		NAME(conncount)
		NAME(memcached)
		NAME(connmax)
		NAME(swapcached)
		NAME(swaptotal)
		NAME(memfree)
		NAME(uptime)
		NAME(wifinets)
		NAME(memtotal)
		NAME(localtime)
		NAME(swapfree)
		NAME(loadavg)
	};
	struct Wan {
		struct Name {
			NAME(proto)
			NAME(ipaddr)
			NAME(netmask)
			NAME(gwaddr)
			NAME(expires)
			NAME(uptime)
			NAME(ifname)
			NAME(dns)
		};
		char proto[8];
		ip4_t ipaddr;
		ip4_t netmask;
	    ip4_t gwaddr;
	    long expires;
	    long uptime;
	    char ifname[16];
	    ip4_t dns[4];
	    static const clas<Wan>& structure() {
	    	return O<Wan,
	    		P<Wan, Name::proto, sizeof(proto), &Wan::proto>,
	    		P<Wan, Name::ipaddr, decltype(Wan::ipaddr), &Wan::ipaddr>,
	    		P<Wan, Name::netmask, decltype(Wan::netmask), &Wan::netmask>,
	    		P<Wan, Name::gwaddr, decltype(Wan::gwaddr), &Wan::gwaddr>,
	    		P<Wan, Name::expires, decltype(Wan::expires), &Wan::expires>,
    			P<Wan, Name::uptime, decltype(Wan::uptime), &Wan::uptime>,
	    		P<Wan, Name::ifname, sizeof(Wan::ifname), &Wan::ifname>,
	    		P<Wan, Name::dns, ip4_t, countof(&Wan::dns), &Wan::dns>
    		>();
	    }
	} wan;
	short membuffers;
	short conncount;
	short memcached;
	short connmax;
	short swapcached;
	short swaptotal;
	short memfree;
	long uptime;
	struct WiFiNet {
		struct Name {
			NAME(device)
			NAME(networks)
			NAME(name)
			NAME(up)
		};
	    char device[16];
	    struct Network {
			struct Name {
				NAME(ifname)
				NAME(encryption)
				NAME(bssid)
				NAME(mode)
				NAME(quality)
				NAME(noise)
				NAME(ssid)
			};
		    char ifname[16];
		    char encryption[16];
		    char ssid[16];
		    char mode[8];
		    char bssid[18];
		    signed char quality;
		    signed char noise;
		    static const clas<Network>& structure() {
		    	return O<Network,
		    		P<Network, Name::ifname, sizeof(Network::ifname), &Network::ifname>,
					P<Network, Name::encryption, sizeof(Network::encryption), &Network::encryption>,
	    			P<Network, Name::ssid, sizeof(Network::ssid), &Network::ssid>,
	    			P<Network, Name::mode, sizeof(Network::mode), &Network::mode>,
    				P<Network, Name::bssid, sizeof(Network::bssid), &Network::bssid>,
					P<Network, Name::quality, signed char, &Network::quality>,
					P<Network, Name::noise, signed char, &Network::noise>
		    >();
		    }
	    } networks[4];
	  	char name[64];
	    bool up;
	    static const clas<WiFiNet>& structure() {
	    	return O<WiFiNet,
	    		P<WiFiNet, Name::device, sizeof(device), &WiFiNet::device>,
	    		P<WiFiNet, Name::networks, Network, countof(&WiFiNet::networks),
					&WiFiNet::networks, Network::structure>,
	    		P<WiFiNet, Name::name, sizeof(name), &WiFiNet::name>,
    			P<WiFiNet, Name::up, decltype(up), &WiFiNet::up>
	    	>();
	    }
	} wifinets[4];
	long memtotal;
	char localtime[32];
	short swapfree;
	double loadavg[3];
	inline void set_uptime(long v) noexcept { uptime = v; }
	inline long get_uptime() const noexcept { return uptime; }
	inline void set_swapfree(short v) noexcept { swapfree = v; }
	inline short get_swapfree() const noexcept { return swapfree; }
	static const clas<Config>& structure() noexcept {
		return
			O<Config,
				P<Config, Name::wan, Config::Wan, &Config::wan, Config::Wan::structure>,
				P<Config, Name::localtime, sizeof(Config::localtime), &Config::localtime>,
	//			P<Config, Name::uptime,   long, &Config::uptime>,
				P<Config, Name::uptime,
					accessor::methods<Config, long, &Config::get_uptime, &Config::set_uptime>>,
				P<Config, Name::conncount, short, &Config::conncount>,
				P<Config, Name::connmax, short, &Config::connmax>,
				P<Config, Name::memcached, short, &Config::memcached>,
				P<Config, Name::membuffers, short, &Config::membuffers>,
				P<Config, Name::swapcached, short, &Config::swapcached>,
				P<Config, Name::swaptotal, short, &Config::swaptotal>,
				P<Config, Name::memfree, short, &Config::memfree>,
				P<Config, Name::wifinets, WiFiNet, countof(&Config::wifinets),
					&Config::wifinets, WiFiNet::structure>,
				P<Config, Name::memtotal, long, &Config::memtotal>,
				P<Config, Name::localtime,sizeof(Config::localtime), &Config::localtime>,
				P<Config, Name::swapfree,
					accessor::methods<Config, short, &Config::get_swapfree, &Config::set_swapfree>>,
				P<Config, Name::loadavg, double, countof(&Config::loadavg), &Config::loadavg>
			>();
	}

};
struct runner {
	typedef void (*func)(lexer& in, ostream& out);
	inline runner(func f) noexcept { add(f); }
private:
	void add(func) noexcept;
};

}
namespace details {
template<>
struct writer<ip4_t> {
	static inline bool write(const ip4_t& val, ostream& out) noexcept {
		return array::write(val, out);
	}
};

template<>
struct reader<ip4_t> {
	static reader<ip4_t> unit;
	static inline bool read(ip4_t& val, lexer& in) noexcept {
		return collection<>::read(unit, val, in);
	}
	static inline bool null(ip4_t& a) noexcept {
		a.clear();
		return true;
	}
	static inline bool read(ip4_t& dst, lexer& in, size_t i) noexcept {
		return dst.read(in,i);
	}
};
}

}
#endif /* BENCH_HPP_ */
