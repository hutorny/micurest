/*
 * esp8266_user.h
 *
 *  Created on: Nov 7, 2016
 *      Author: eugene
 */
#ifndef TOOLS_ESP8266_USER_H_
#define TOOLS_ESP8266_USER_H_
#ifdef __cplusplus
	extern "C" {
#endif
#include "user_interface.h"
	void serial_write(const char*);
	void serial_writec(char);
	int serial_available();
	char serial_read();
	typedef void (*serial_callback)(unsigned len);
	void user_rx_installcb(serial_callback cb);
	typedef void (*heartbeat_callback)(int cnt);
	void user_hb_installcb(heartbeat_callback cb);
	void ets_vprintf(void (*p)(char), const char*, ...);
	void ets_putc(char);
	unsigned  micros(void);
	void console_attach(void);
	void __libc_init_array(void);
	void _init(void);
#ifdef __cplusplus
	}
#endif

#endif /* TEST_TOOLS_ESP8266_USER_H_ */
