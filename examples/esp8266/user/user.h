/*
 * user.h
 */

#ifndef USER_USER_H_
#define USER_USER_H_
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wparentheses"
#include "osapi.h"
#include "user_interface.h"
#include "ets_sys.h"
#include "gpio.h"
#include "eagle_soc.h"
#include "sntp.h"
#include "ip_addr.h"
#include "espconn.h"
#pragma GCC diagnostic pop

void user_server(void);
void user_sntp_init(void);
bool user_load_persistent(void);
bool user_save_persistent(void);

extern void __libc_init_array (void);
extern void ets_printf(const char*, ...);
extern void os_printf_plus(const char*,...);
extern void ets_vprintf(void (*p)(char), const char*, ...);
extern void ets_putc(char);
extern void ets_delay_us(uint16);
extern void ets_timer_setfn(os_timer_t *ptimer,os_timer_func_t *pfunction,
		void *parg);
extern void ets_timer_arm_new(os_timer_t *ptimer, uint32_t milliseconds,
		bool repeat_flag, bool millis);


#endif /* USER_USER_H_ */
