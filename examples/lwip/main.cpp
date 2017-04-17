/*
 * main.c
 *
 *  Created on: Mar 20, 2017
 *      Author: Eugene
 */

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <conio.h>
#include <ctime>

extern "C" {
#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/timeouts.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/init.h"
#include "lwip/tcpip.h"
#include "lwip/netif.h"
#include "lwip/api.h"
#include "lwip/etharp.h"
#include "lwip/dhcp.h"
#include "netif/ethernet.h"
#include "pcapif.h"
#include "lwip/apps/httpd.h"
#include "lwip/apps/mdns.h"
#include "apps/tcpecho/tcpecho.h"
#include "apps/udpecho/udpecho.h"
}

extern "C" unsigned short LWIP_RAND() {
	return rand();
}

#include "micurpc.hpp"
#include "network_lwip.hpp"
#include "enumnames.hpp"
#include "miculog.hpp"

using micurest::char_t;
using cojson::cstring;

#include <cstdint>

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
	NAME(demo)
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
	NAME(rpc)
	NAME(rpcc)
	ALIAS(_const,const)
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

extern cstring rpc_list(cojson::size_t) noexcept;
extern cstring rpc_constants() noexcept;
extern const micurest::resource::node& rpc_node() noexcept;

namespace sample {
using namespace micurest;

// file2c < index.html > index.html.inc
static const char demo_html_[] = {
#	include "index.html.inc"
	,0
};

static const char rpc_html_[] = {
#	include "rpc.html.inc"
	,0
};

static cstring demo_html() noexcept {
	return demo_html_;
}

static cstring rpc_html() noexcept {
	return rpc_html_;
}

static status_t put_restart(const char* id) noexcept {
	//TODO printf
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

template<cojson::details::cstring (*get)(size_t)>
const cojson::details::value& Enums() noexcept {
	static const cojson::details::strings<get> l;
	return l;
}

const directory& resourceMap() noexcept {
	return Root<
		F<name::demo, demo_html,media::text::html>,
		F<name::rpcc, rpc_html, media::text::html>,
		E<name::rpc, rpc_node>,
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
			E<name::mode, N<Enums<modenames::get>>>,
			E<name::rpc,  N<Enums<rpc_list>>>,
			F<name::_const, rpc_constants, media::json> /* static json text*/
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

micurest::application app(sample::resourceMap());
micurest::network_lwip::tcp::server server(app);

extern "C" void server_init(void) {
	setvbuf(stderr, NULL, _IONBF, 0);
	server.listen(80);
}

namespace enumnames {
	bool match(const char* a, const char* b) noexcept {
		return cojson::details::match(a,b);
	}
}

inline static constexpr ip4_addr_t ip4(uint8_t a,uint8_t b,uint8_t c, uint8_t d) noexcept {
	return ip4_addr_t{(uint32_t(d)<<24)|(uint32_t(c)<<16)|(uint32_t(b)<<8)|(uint32_t(a))};
}


static void status_callback(struct netif *state_netif) {
  if (netif_is_up(state_netif)) {
    printf("status_callback==UP, local interface IP is %s\n", ip4addr_ntoa(netif_ip4_addr(state_netif)));
    mdns_resp_netif_settings_changed(state_netif);
  } else {
    printf("status_callback==DOWN\n");
  }
}

static void link_callback(struct netif *state_netif) {
  if (netif_is_link_up(state_netif)) {
    printf("link_callback==UP\n");
  } else {
    printf("link_callback==DOWN\n");
  }
}

static void srv_txt(struct mdns_service *service, void *) {
   mdns_resp_add_service_txtitem(service, "path=/", 6);
}

static struct netif netif;

static void ifc_init(void) {
  ip4_addr_t ipaddr{ip4(192,168,28,251)}, netmask{ip4(255,255,255,0)}, gw{ip4(192,168,28,1)};
  printf("Starting lwIP, local interface IP is %s\n", ip4addr_ntoa(&ipaddr));

  netif_set_default(netif_add(&netif, &ipaddr, &netmask, &gw, nullptr, pcapif_init, tcpip_input));
  netif_create_ip6_linklocal_address(&netif, 1);
  printf("ip6 linklocal address: ");
  ip6_addr_debug_print(0xFFFFFFFF & ~LWIP_DBG_HALT, netif_ip6_addr(&netif, 0));
  printf("\n");
  netif_set_status_callback(&netif, status_callback);
  netif_set_link_callback(&netif, link_callback);
  netif_set_up(&netif);
//  auto err = dhcp_start(&netif);
//  if( err )
//	  printf("dhcp start error %d\n", err);
  //err = autoip_start(&netif);
}


static void srvr_init(void *init_sem) {
  srand((unsigned int)time(0));
  ifc_init();

//  httpd_init();
  server_init();
  mdns_resp_init();
  mdns_resp_add_netif(netif_default, "lwip", 3600);
  mdns_resp_add_service(netif_default, "lwipweb", "_http", DNSSD_PROTO_TCP, HTTPD_SERVER_PORT, 3600, srv_txt, NULL);

  udpecho_init();

  sys_sem_signal((sys_sem_t*)init_sem);
}


int main(void) {
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	sys_sem_t init_sem;
	sys_sem_new(&init_sem, 0);
	tcpip_init(srvr_init, &init_sem);
	sys_sem_wait(&init_sem);
	sys_sem_free(&init_sem);

	while (!_kbhit()) {
		sys_msleep(50);
	}
	pcapif_shutdown(&netif);
	return 0;
}

void dbg(const char *fmt, ...) noexcept {
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
}
