#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <micurest_arduino.h>

using namespace micurest;

namespace name {
  /* names/identifiers used in this demo
   * string literals are placed in progmem with macro NAME or ALIAS     */
  NAME(hello)
  NAME(led)
  ALIAS(hello_html,hello.html)
}

/* simple HTML page with two input controls */
cstring hello_html_content() {
  static const char content[] __attribute__((progmem)) = 
  "<!DOCTYPE html><html><head><meta charset='utf-8'><title>Arduino Hello World Example</title></head>"
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
application rest(resourceMap());

/* instance of a tcp server, running the rest application            */
network_arduino::tcp::server server(rest);
static uint8_t mac[]={0xC2,0xB5,0x52,0x45,0x53,0x54};
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  // start serial at 115200
  Serial.begin(115200);    
  Serial.println("Micurest Hello World is starting");  
  // start Ethernet
  Ethernet.begin(mac);
  Serial.print("server IP is ");
  Serial.println(Ethernet.localIP());  
  // start listening on port 80 
  server.listen(80); 
}

void loop() {
	server.run();
}
