// To verify product creator macros work correctly.
// These values get sent to the cloud on connection and help dashboard.particle.io do the right thing
//
#include "application.h"
#include "Serial2/Serial2.h"
#include "system_network.h"
#include "wlan_hal.h"
#include "micurest/network_spark_socket.hpp"
#include "micurest/access_log.hpp"

using namespace micurest;
using namespace micurest::network_spark_socket;

#define NAME(s) static inline constexpr const char* s() noexcept {return #s;}
#define ALIAS(f,s) static inline constexpr const char_t* f() noexcept {return #s;}
namespace name {
  /* names/identifiers used in this demo									*/
  NAME(hello)
  NAME(led)
  ALIAS(hello_html,hello.html)
}

/* simple HTML page with two input controls */
cstring hello_html_content() {
  static const char content[] =
  "<!DOCTYPE html><html><head><meta charset='utf-8'><title>Photon Hello World Example</title></head>"
  "<body><label for='text'>Message:</label><input class='bind' id='hello' maxlength='32' disabled><br>"
  "<label for='led'>LED:</label><input type='checkbox' class='bind' id='led' disabled></body>"
  "<script type='text/javascript' src='//r.iot-ware.com/a/main.js'></script></html>";
  return cstring(content);
}

/* string variable, accessible via HTTP */
char hello_text[32] = "Hello world";

/* functions for controlling built-in LED */
bool getLed() {
  return digitalRead(LED_BUILTIN);
}

/* functions for controlling built-in LED */
void setLed(bool val) {
  return digitalWrite(LED_BUILTIN,val);
}

/* map of URIs to application resources */
const directory& resourceMap() noexcept {
  return Root<
    resource::FileConstString<name::hello_html, hello_html_content, media::html>,
    resource::FileText<name::hello, sizeof(hello_text), hello_text>,
    resource::FileFunctions<name::led, bool, getLed, setLed>
  >();
}

/* define an application instance associated with the map */
application rest_app(resourceMap());

/* instance of a tcp server, running the rest application            */
tcp::server server(rest_app);


extern const micurest::network::access_log access;
extern const miculog::Log<micurest::network_spark_socket::tcp::server> _log;

void report_ip() {
	WLanConfig config;
	wlan_fetch_ipconfig(&config);
	Serial1.print("my IP: "); /* welcome to Ardiuno world     */
	Serial1.print((uint32_t)(config.nw.aucIP.ipv4 >> 24) & 0xFF);
	Serial1.print('.');
	Serial1.print((uint32_t)(config.nw.aucIP.ipv4 >> 16) & 0xFF);
	Serial1.print('.');
	Serial1.print((uint32_t)(config.nw.aucIP.ipv4 >>  8) & 0xFF);
	Serial1.print('.');
	Serial1.println((uint32_t)(config.nw.aucIP.ipv4)     & 0xFF);
}

void setup() {
	Serial1.begin(115200);
	pinMode(LED_BUILTIN, OUTPUT);
	Serial1.println("Photon demo starting");
	/* wifi is not ready here, so there is no reason to try listen */
}
bool done = false;

void loop() {
	if( done )
		server.run();
	else {
		/* it takes some time for wifi getting up and running,
		 * so here we try to start server and report ip when done */
		done = server.listen(80);
		if( done ) report_ip();
		else delay(100);
	}
}
