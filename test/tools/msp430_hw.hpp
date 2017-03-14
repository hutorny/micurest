/*
 * msp430_hw.hpp
 *
 *  Created on: Nov 14, 2015
 *      Author: eugene
 */

#ifndef MSP430_HW_HPP_
#define MSP430_HW_HPP_
#include "hw_memmap.h"

namespace msp430 {

static inline constexpr uint16_t gpio_port_addr(uint16_t port) noexcept {
	return
#if defined(__MSP430_HAS_PORT1_R__)
	port == 1 ?
    __MSP430_BASEADDRESS_PORT1_R__ :
#elif defined(__MSP430_HAS_PORT1__)
	port == 1 ?
    __MSP430_BASEADDRESS_PORT1__ :
#endif
#if defined(__MSP430_HAS_PORT2_R__)
	port == 2 ?
    __MSP430_BASEADDRESS_PORT2_R__ :
#elif defined(__MSP430_HAS_PORT2__)
	port == 2 ?
    __MSP430_BASEADDRESS_PORT2__ :
#endif
#if defined(__MSP430_HAS_PORT3_R__)
	port == 3 ?
    __MSP430_BASEADDRESS_PORT3_R__ :
#elif defined(__MSP430_HAS_PORT3__)
	port == 3 ?
    __MSP430_BASEADDRESS_PORT3__ :
#endif
#if defined(__MSP430_HAS_PORT4_R__)
	port == 4 ?
    __MSP430_BASEADDRESS_PORT4_R__ :
#elif defined(__MSP430_HAS_PORT4__)
	port == 4 ?
    __MSP430_BASEADDRESS_PORT4__ :
#endif
#if defined(__MSP430_HAS_PORT5_R__)
	port == 5 ?
    __MSP430_BASEADDRESS_PORT5_R__ :
#elif defined(__MSP430_HAS_PORT5__)
	port == 5 ?
    __MSP430_BASEADDRESS_PORT5__ :
#endif
#if defined(__MSP430_HAS_PORT6_R__)
	port == 6 ?
    __MSP430_BASEADDRESS_PORT6_R__:
#elif defined(__MSP430_HAS_PORT6__)
	port == 6 ?
    __MSP430_BASEADDRESS_PORT6__:
#endif
#if defined(__MSP430_HAS_PORT7_R__)
	port == 7 ?
    __MSP430_BASEADDRESS_PORT7_R__:
#elif defined(__MSP430_HAS_PORT7__)
	port == 7 ?
    __MSP430_BASEADDRESS_PORT7__:
#endif
#if defined(__MSP430_HAS_PORT8_R__)
	port == 8 ?
    __MSP430_BASEADDRESS_PORT8_R__:
#elif defined(__MSP430_HAS_PORT8__)
	port == 8 ?
    __MSP430_BASEADDRESS_PORT8__:
#endif
#if defined(__MSP430_HAS_PORT9_R__)
	port == 9 ?
    __MSP430_BASEADDRESS_PORT9_R__:
#elif defined(__MSP430_HAS_PORT9__)
	port == 9 ?
    __MSP430_BASEADDRESS_PORT9__:
#endif
#if defined(__MSP430_HAS_PORT10_R__)
	port == 10 ?
    __MSP430_BASEADDRESS_PORT10_R__:
#elif defined(__MSP430_HAS_PORT10__)
	port == 10 ?
    __MSP430_BASEADDRESS_PORT10__ :
#endif
#if defined(__MSP430_HAS_PORT11_R__)
	port == 11 ?
    __MSP430_BASEADDRESS_PORT11_R__:
#elif defined(__MSP430_HAS_PORT11__)
	port == 11 ?
    __MSP430_BASEADDRESS_PORT11__:
#endif
#if defined(__MSP430_HAS_PORTJ_R__)
	port == 13 ?
    __MSP430_BASEADDRESS_PORTJ_R__:
#elif defined(__MSP430_HAS_PORTJ__)
	port == 13 ?
    __MSP430_BASEADDRESS_PORTJ__:
#endif
    0xFFFF;
}
}

#endif /* MSP430_HW_HPP_ */
