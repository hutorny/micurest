
#include <msp430.h>
#include <msp430fr6989.h>
#include "msp430.hpp"
#include "hw_memmap.h"

namespace msp430 {

static inline constexpr volatile uint16_t& uart_reg(
	uart::channel_t chnl, uint16_t off) {
	return chnl == uart::channel_t::esci_a0 ?
		hwreg16(EUSCI_A0_BASE,off) : hwreg16(EUSCI_A1_BASE,off);
}

struct brg {
	const uint16_t brw;
	const uint16_t mctlw;
	inline constexpr brg(
		uint16_t brw_,
		uint8_t brf,
		uint8_t brs,
		bool os) noexcept
	  : brw(brw_),
		mctlw(brs << 8 | brf << 4 | (os ? UCOS16 : 0)) {}
};

static constexpr const brg presets[uart::baudrate_count] = {
/*	 brw brf brs   oversample */
	{138, 0, 0xF7, false},
	{  4, 5, 0x55, true}
};

inline void uart::init(
		uart::baudrate_t baudrate,
		const uart::params& param) const noexcept {
	uart_reg(channel, OFS_UCAxCTLW0) = UCSWRST;
	uart_reg(channel, OFS_UCAxBRW)   =
			presets[static_cast<unsigned>(baudrate)].brw;
	uart_reg(channel, OFS_UCAxMCTLW) =
			presets[static_cast<unsigned>(baudrate)].mctlw;
	uart_reg(channel, OFS_UCAxCTLW0) = param.ctlw0;
}


bool uart::put(char data, uart::blocking_t blocking) const noexcept {
    if( blocking == uart::blocking_t::blocking )
        //Poll for transmit interrupt flag
        while(!(uart_reg(channel, OFS_UCAxIFG) & UCTXIFG));
    else 
        if(!(uart_reg(channel,OFS_UCAxIFG) & UCTXIFG)) return false;
    uart_reg(channel, OFS_UCAxTXBUF) = data;
    return true;
}

bool uart::get(char& data, uart::blocking_t blocking) const noexcept {
    if( blocking == uart::blocking_t::blocking )
        while(!(uart_reg(channel, OFS_UCAxIFG) & UCRXIFG));
    else
        if(!(uart_reg(channel,OFS_UCAxIFG) & UCRXIFG)) return false;
    data = uart_reg(channel,OFS_UCAxRXBUF);
    return true;
}

void uart::begin(uart::baudrate_t baudrate, const uart::params & p) const noexcept {
	/* msp430fr6989.pdf 6.11.24.1 											*/
    switch(channel) {
    case channel_t::esci_a0:
    	/* USCI_A0 UART operation P4.3/P4.2									*/
        P4SEL0 |= BIT2 | BIT3;				/* UCA0TXD						*/
        P4SEL1 &= ~(BIT2 | BIT3);			/* UCA0RXD						*/
        break;
    case channel_t::esci_a1:
        P3SEL0 |= BIT4 | BIT5;              /* UCA1TXD						*/
        P3SEL1 &= ~(BIT4 | BIT5);			/* UCA1RXD						*/
    }
	init(baudrate, p);
}
}
