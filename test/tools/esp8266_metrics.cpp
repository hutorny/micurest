extern "C" {
#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "esp8266_user.h"
}

#define USER_QUEUE_LEN    		1
os_event_t    user_queue[USER_QUEUE_LEN];

extern void loop();
extern void setup();

static void ICACHE_FLASH_ATTR user_task(os_event_t *events) {
	loop();
}

void ICACHE_FLASH_ATTR _init(void) {}
extern "C" void user_init();

//Init function 
void ICACHE_FLASH_ATTR user_init() {
    __libc_init_array(); /* fix missig global ctors */
    system_os_task(user_task, 0, user_queue, USER_QUEUE_LEN);
}
