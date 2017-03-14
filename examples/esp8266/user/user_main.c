#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "sntp.h"
#include "os_type.h"
#include "user_config.h"
#include "driver/uart.h"
#include "user_interface.h"
#include "user.h"

#define user_procTaskPrio        1
#define user_procTaskQueueLen    1
os_event_t    user_procTaskQueue[user_procTaskQueueLen];

static os_timer_t some_timer;

void some_timerfunc(void *arg) {
	static int cnt = 0;
	if( ++cnt & 1 )
        gpio_output_set(0, BIT2, BIT2, 0);
    else
        gpio_output_set(BIT2, 0, BIT2, 0);
}

static inline unsigned get_ccount(void) {
	unsigned r;
	asm volatile ("rsr %0, ccount" : "=r"(r));
	return r;
}

unsigned ICACHE_FLASH_ATTR micros(void) {
	return get_ccount() / system_get_cpu_freq();
}

void ICACHE_FLASH_ATTR _init(void) {}

void ICACHE_FLASH_ATTR user_init() {
    uart_init(BIT_RATE_115200,BIT_RATE_115200);
    UART_SetPrintPort(UART0);
    __libc_init_array(); /* fix missing global ctors */

    gpio_init();

    //Set GPIO2 to output mode
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);

    //Set GPIO2 low
    gpio_output_set(0, BIT2, BIT2, 0);
    user_load_persistent();
    user_sntp_init();
    os_timer_disarm(&some_timer);
    os_timer_setfn(&some_timer, (os_timer_func_t *)some_timerfunc, NULL);
    os_timer_arm(&some_timer, 1000, 1);
//    espconn_secure_set_size(0x02, 4096);
    user_server();
}
