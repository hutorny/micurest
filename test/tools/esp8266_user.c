#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "driver/uart.h"
#include "esp8266_user.h"

#define USER_RX_QUEUE_LEN    		1
os_event_t    user_rx_queue[USER_RX_QUEUE_LEN];
#define BUF_SPAN (8)
#define BUF_SIZE (1<<BUF_SPAN)
#define POS_MASK (BUF_SIZE - 1)

static volatile unsigned putpos = 0;
static volatile unsigned getpos = 0;
static char serial_buffer[BUF_SIZE];

static inline volatile unsigned serial_datalen() {
	return putpos-getpos;
}
static inline volatile int has_space() {
	return serial_datalen() < sizeof(serial_buffer);
}

void ICACHE_FLASH_ATTR serial_write(const char* s) {
	uart0_sendStr(s);
}

void ICACHE_FLASH_ATTR serial_writec(char c) {
	uart_tx_one_char(UART0, c);
}

int ICACHE_FLASH_ATTR serial_available() {
	return serial_datalen();
}

char ICACHE_FLASH_ATTR serial_read() {
	return serial_datalen() ? serial_buffer[getpos++ & POS_MASK] : 0;
}

static serial_callback user_rx_callback;

void ICACHE_FLASH_ATTR user_rx_installcb(serial_callback cb) {
	user_rx_callback = cb;
}

static void ICACHE_FLASH_ATTR user_rx_task(os_event_t *events) {
    if(events->sig == 0){
        uint8 fifo_len = (READ_PERI_REG(UART_STATUS(UART0))>>UART_RXFIFO_CNT_S)&UART_RXFIFO_CNT;
        uint8 d_tmp = 0;
        uint8 idx=0;
        for(idx=0;idx<fifo_len;idx++) {
            d_tmp = READ_PERI_REG(UART_FIFO(UART0)) & 0xFF;
            uart_tx_one_char(UART0, d_tmp);
            if( has_space() ) {
            	serial_buffer[putpos++ & POS_MASK] = d_tmp;
            }
        }
        WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_FULL_INT_CLR|UART_RXFIFO_TOUT_INT_CLR);
        uart_rx_intr_enable(UART0);
        if( serial_datalen() && user_rx_callback ) {
        	user_rx_callback(serial_datalen());
        }
    }
}

static volatile os_timer_t hb_timer;
static heartbeat_callback user_hb_callback;

static int cnt = 0;
static void ICACHE_FLASH_ATTR heartbeat_task(void *arg) {
	if( ++cnt & 1 ) {
        gpio_output_set(BIT2, 0, BIT2, 0);
    } else {
        gpio_output_set(0, BIT2, BIT2, 0);
    }

	if( user_hb_callback ) {
		user_hb_callback(cnt);
	}
}

void ICACHE_FLASH_ATTR user_hb_installcb(heartbeat_callback cb) {
	user_hb_callback = cb;
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

//Init function 
void ICACHE_FLASH_ATTR user_init() {
    uart_init(BIT_RATE_115200,BIT_RATE_115200);
    UART_SetPrintPort(UART0);
    __libc_init_array(); /* fix missig global ctors */
    // Initialize the GPIO subsystem.
    gpio_init();
    //Set GPIO2 to output mode
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
    //Set GPIO2 low
    gpio_output_set(0, BIT2, BIT2, 0);
    os_timer_disarm(&hb_timer);
    os_timer_setfn(&hb_timer, (os_timer_func_t *)heartbeat_task, NULL);
    os_timer_arm(&hb_timer, 1000, 1);
    //Start os task at priority 0
    system_os_task(user_rx_task, 0, user_rx_queue, USER_RX_QUEUE_LEN);
    console_attach();
}
