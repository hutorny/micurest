#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <Controllino.h>
#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <utility/socket.h>
#pragma GCC diagnostic pop
#include "utils.hpp"

/*
 * This file contains usual Arduino staff, all micurest specific is in
 * user_server.cpp and utils.hpp analogRead
 */

extern void server_run() noexcept;
extern void server_listen() noexcept;
extern void io_setup() noexcept;

ip_addr ip;
mac_addr mac = {{0xDA, 0xD1, 0xBE, 0xEF, 0xF0, 0x00}};

void setup() {
  Serial.begin(115200);
  io_setup();						/*	setup pin modes for this example	*/
  /**************************************************************************/
  /* part of Controllino example											*/
  /*																		*/
  #ifdef CONTROLLINO_MEGA
  DDRD |= B01110000;
  DDRJ |= B00010000;
  #endif
  #if defined(CONTROLLINO_MAXI) || defined(CONTROLLINO_MEGA)
  DDRJ |= B01100000;
  DDRE &= B01111111;
  #endif
  Controllino_RTC_init(1);
  Serial.println("CONTROLLINO Micurest demo starting");
  Ethernet.begin(mac.addr);
  Serial.print("server IP is ");
  ip = Ethernet.localIP();
  Serial.println(Ethernet.localIP());
  /**************************************************************************/

  /* initialize micurest server 											*/
  server_listen();
}

unsigned long last = 0;
uint8_t toggle = 0;

void loop() {
	auto elapsed = millis() - last;
	if( elapsed > 1000 ) {
	/* blink every second to indicate we are alive */
	  digitalWrite(CONTROLLINO_D0, ++toggle & 1);
	  last = millis();
	}
	/* run the server */
	server_run();
}

#ifdef TOOLS_DBG_H_
static char buff[128];

void dbg(const char *fmt, ...) noexcept  {
	va_list args;
	va_start(args, fmt);
	vsprintf(buff, fmt, args);
	va_end(args);
	Serial.write(buff);
}

#endif
