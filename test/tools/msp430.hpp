#include <stdint.h>

#include "msp430_hw.hpp"

namespace msp430 {

template<typename T>
inline constexpr volatile T & hwreg(uint16_t addr) noexcept {
	return * reinterpret_cast<volatile T *>(addr);
}

inline constexpr volatile uint16_t & hwreg16(uint16_t base, uint16_t  off) noexcept {
	return * reinterpret_cast<volatile uint16_t *>(base + off);
}

inline constexpr volatile uint8_t & hwreg8(uint16_t base, uint16_t  off) noexcept {
	return * reinterpret_cast<volatile uint8_t *>(base + off);
}

template<bool, typename A, typename B>
struct conditional;
template<typename A, typename B>
struct conditional<true, A, B> { typedef A type; };
template<typename A, typename B>
struct conditional<false, A, B> { typedef B type; };


class pmm {
public:
	static inline void unlockLPM5() noexcept {
	    hwreg8(PMM_BASE, OFS_PM5CTL0) &= ~LOCKLPM5;
	}
	//TODO other methods
private:
	pmm();
};

class clocks {
public:
	static inline void unlock() noexcept {
		hwreg16(CS_BASE, OFS_CSCTL0) = CSKEY;
	}
	static inline void lock() noexcept {
		hwreg8(CS_BASE, OFS_CSCTL0_H) = 0x00;
	}

	enum class divider_t {
		_1  =  0,
		_2  =  1,
		_4  =  2,
		_8  =  3,
		_16 =  4,
		_32 =  5,
	};
	/************************************************************************
	 * 							Clock Sources								*
	 ************************************************************************/
	/**
	 * LFXTCLK: Low-frequency oscillator (32768 Hz)
	 */
	class lfxt {
	public:
		static constexpr uint16_t ctl2 = 0;
		enum class frequency_t : long {
			_32768Hz  =  32768L,
		};
		static constexpr frequency_t frequency = frequency_t::_32768Hz;
	};
	/**
	 * VLOCLK: Internal very-low-power low-frequency oscillator (10-kHz)
	 */
	class vlo {
	public:
		static constexpr uint16_t ctl2 = 1;
		enum class frequency_t : long {
			_10kHz  =  10000L,
		};
		static constexpr frequency_t frequency = frequency_t::_10kHz;
	};
	/**
	 * DCOCLK: Internal digitally controlled oscillator (1-24MHz)
	 */
	class dco {
	public:
		static constexpr uint16_t ctl2 = 3;
		enum class frequency_t : long {
			_1MHz  =  1000000L,
			_2M6Hz =  2600000L,
			_3M3Hz =  3300000L,
			_4MHz  =  4000000L,
			_5M3Hz =  5300000L,
			_6M7Hz =  6700000L,
			_8MHz  =  8000000L,
			_16MHz = 16000000L,
			_21MHz = 21000000L,
			_24MHz = 24000000L,
		};
		struct frequency {
			const uint16_t ctl1;
			inline constexpr frequency(frequency_t frq)
			  : ctl1(
				frq == frequency_t::_1MHz  ? 0x00 :
				frq == frequency_t::_2M6Hz ? 0x02 :
				frq == frequency_t::_3M3Hz ? 0x04 :
				frq == frequency_t::_4MHz  ? 0x06 :
				frq == frequency_t::_5M3Hz ? 0x08 :
				frq == frequency_t::_6M7Hz ? 0x0A :
				frq == frequency_t::_8MHz  ? 0x0C :
				frq == frequency_t::_16MHz ? 0x48 :
				frq == frequency_t::_21MHz ? 0x4A :
				frq == frequency_t::_24MHz ? 0x4C : 0x00
			  ) {}
		};
		static inline void setup(const frequency & frq) {
			clocks::unlock();
			hwreg16(CS_BASE, OFS_CSCTL1) = frq.ctl1;
			clocks::lock();
		}
		static inline void setup(frequency_t frq) {
			setup(frequency(frq));
		}
	private:
		dco();
	};
	/**
	 * MODCLK: Internal low-power oscillator (5-MHz)
	 */
	class mod {
	public:
		static constexpr uint16_t ctl2 = 4;
		enum class frequency_t : long {
			_5MHz  =  5000000L,
		};
		static constexpr frequency_t frequency = frequency_t::_5MHz;
		//TODO
	};
	class lfmod {
	public:
		static constexpr uint16_t ctl2 = 2;
		enum class frequency_t : long {
			_39kHz  =  39000L,
		};
		static constexpr frequency_t frequency = frequency_t::_39kHz;
		//TODO
	};
	/**
	 * HFXTCLK: High-frequency oscillator (4-24-MHz)
	 */
	class hfxt {
	public:
		static constexpr uint16_t ctl2 = 5;
		//TODO
	};
	/************************************************************************
	 * 							Clock Signals (outputs)						*
	 ************************************************************************/
	class aclk {
	public:
		template<typename src>
		static inline void init(divider_t divider) {
			static_assert(
				src::ctl2 == lfmod::ctl2 ||
				src::ctl2 == lfxt::ctl2  ||
				src::ctl2 == vlo::ctl2,
				"Invalid ACLK source");
			clocks::unlock();
			hwreg16(CS_BASE, OFS_CSCTL2) &= ~(SELA_7);
			hwreg16(CS_BASE, OFS_CSCTL2) |= src::ctl2 << 8;
			hwreg16(CS_BASE, OFS_CSCTL3) &= ~(DIVA0 | DIVA1 | DIVA2);
			hwreg16(CS_BASE, OFS_CSCTL3) |= static_cast<uint16_t>(divider) << 8;
			clocks::lock();
		}
	private:
		aclk();
	};
	class mclk {
	public:
		template<typename src>
		static inline void init(divider_t divider) {
			static_assert(
				src::ctl2 >= lfxt::ctl2 && src::ctl2 <= hfxt::ctl2,
				"Invalid MCLK source");
			clocks::unlock();
			hwreg16(CS_BASE, OFS_CSCTL2) &= ~(SELM_7);
			hwreg16(CS_BASE, OFS_CSCTL2) |= src::ctl2;
			hwreg16(CS_BASE, OFS_CSCTL3) &= ~(DIVM0 | DIVM1 | DIVM2);
			hwreg16(CS_BASE, OFS_CSCTL3) |= static_cast<uint16_t>(divider);
			clocks::lock();
		}
	private:
		mclk();
	};
	class smclk {
	public:
		template<typename src>
		static inline void init(divider_t divider) {
			static_assert(
				src::ctl2 >= lfxt::ctl2 && src::ctl2 <= hfxt::ctl2,
				"Invalid SMCLK source");
			clocks::unlock();
	        hwreg16(CS_BASE, OFS_CSCTL2) &= ~(SELS_7);
			hwreg16(CS_BASE, OFS_CSCTL2) |= src::ctl2 << 4;
			hwreg16(CS_BASE, OFS_CSCTL3) &= ~(DIVS0 | DIVS1 | DIVS2);
			hwreg16(CS_BASE, OFS_CSCTL3) |= static_cast<uint16_t>(divider) << 4;
			clocks::lock();
		}
	private:
		smclk();
	};

	enum class signal_t {
		aclk,	/** ACLK: Auxiliary clock									*/
		mclk,	/** MCLK: Master clock 										*/
		smclk,	/** SMCLK: Sub-system master clock.							*/
		modclk,	/** MODCLK: Module clock.									*/
		vloclk	/** VLOCLK: VLO clock										*/
	};
private:
	clocks();
};


class gpio {
public:
	enum class port_t {
		p1  =  1,
		p2  =  2,
		p3  =  3,
		p4  =  4,
		p5  =  5,
		p6  =  6,
		p7  =  7,
		p8  =  8,
		p9  =  9,
		p10 = 10,
		p11 = 11,
		/* 16-bit ports are not implemented
		pa  =  1,
		pb  =  3,
		pc  =  5,
		pd  =  7,
		pe  =  9,
		pf  = 11, 							*/
		pj  = 13,
	};

	enum class direction_t { in, out };
	enum class function_t { io, primary, secondary, ternary };
	enum class pull_t { none, up, down };

	static constexpr uint8_t all = 0xFF;

	template<port_t P>
	class port {
	public:
		static constexpr uint8_t num = static_cast<uint8_t>(P);
		static constexpr uint16_t address = gpio_port_addr(num);
		static_assert(address!=0xFFFF,"Unsupported port");
		template<uint8_t M>
		class pins {
		public:
			static_assert(M!=0,"Invalid pin mask");
			static constexpr uint16_t shift = (num&1 ?0:8);
			static constexpr uint16_t mask = M << shift;
			/** select pin function as GPIO */
			static inline void sel() noexcept {
				hwreg16(address,OFS_PASEL0) &= ~ mask;
				hwreg16(address,OFS_PASEL1) &= ~ mask;
			}
			static inline void sel(function_t mode) noexcept {
				switch( mode ) {
				case function_t::io:
					sel();
				case function_t::primary:
					hwreg16(address,OFS_PASEL0) |=   mask;
					hwreg16(address,OFS_PASEL1) &= ~ mask;
					break;
				case function_t::secondary:
					hwreg16(address,OFS_PASEL0) &= ~ mask;
					hwreg16(address,OFS_PASEL1) |=   mask;
					break;
				case function_t::ternary:
					hwreg16(address,OFS_PASELC) |=   mask;
				}
			}
			/** set pin direction */
			static inline void dir(direction_t d) noexcept {
				if(d == direction_t::in)
					hwreg16(address,OFS_PADIR) &= ~ mask;
				else
					hwreg16(address,OFS_PADIR) |=   mask;
			}
			/** select pin and direction input */
			static inline void in(pull_t pull) noexcept {
				sel();
				dir(direction_t::in);
				switch(pull) {
				case pull_t::none:
					hwreg16(address,OFS_PAREN) &= ~mask;
					break;
				case pull_t::up:
					set();
					hwreg16(address,OFS_PAREN) |= mask;
				case pull_t::down:
					clr();
					hwreg16(address,OFS_PAREN) |= mask;
				}
			}
			/** select pin and direction input */
			static inline void in() noexcept {
				sel();
				hwreg16(address,OFS_PADIR) &= ~ mask;
			}
			/** select pin and direction output */
			static inline void out() noexcept {
				sel();
				hwreg16(address,OFS_PADIR) |=   mask;
			}
			static inline void set(bool val) noexcept {
				if( val )
					set();
				else
					clr();
			}
			static inline void set() noexcept {
				hwreg16(address,OFS_PAOUT) |= mask;
			}
			static inline void clr() noexcept {
				hwreg16(address,OFS_PAOUT) &= ~mask;
			}
			static inline void tgl() noexcept {
				hwreg16(address,OFS_PAOUT) ^= mask;
			}
			static inline uint16_t read() noexcept {
				return (hwreg16(address,OFS_PAIN) & mask) >> shift;
			}
			static inline bool get() noexcept {
				return hwreg16(address,OFS_PAIN) & mask;
			}
			//TODO enable/disable interrupt,
		private:
			pins();
		};
		template<uint8_t I>
		class pin : public pins<(1<<I)>{
			static_assert(I<=7,"Pin number out of range");
		};
		template<uint8_t M = all>
		static inline void init() noexcept {
			port<P>::template pins<M>::clr();
			port<P>::template pins<M>::out();
		}
	private:
		port();
	};
private:
	gpio();
};


class uart {
public:
	static constexpr unsigned baudrate_count = 2;
	static constexpr unsigned max_channel_count = 2;
	enum class baudrate_t {
	    _57600,
	    _115200
	};
	enum class blocking_t {	blocking, non_blocking	};
	enum class channel_t : uint_fast8_t { esci_a0, esci_a1 };
	enum class clk_src_t { uclk, aclk, smclk };
	enum class bits_t { _8, _7 };
	enum class parity_t { no, odd, even	};
	enum class stopbits_t {	one, two };

	struct params {
		uint16_t ctlw0;
		inline constexpr params(
			clk_src_t clk_src = clk_src_t::smclk,
			bits_t bits = bits_t::_8,
			parity_t parity = parity_t::no,
			stopbits_t stopbits = stopbits_t::one
		) :
			ctlw0(
				(parity != parity_t::no   		? 0x8000 : 0 ) |
				(parity == parity_t::even		? 0x4000 : 0 ) |
				/* always LSB first								*/
				(bits == bits_t::_7 	 		? 0x1000 : 0 ) |
				(stopbits == stopbits_t::one	? 0x0800 : 0 ) |
				/* always UART mode 							*/
				/* always Asynchronous mode 					*/
				(clk_src == clk_src_t::smclk	? 0x0080 : 0 ) |
				(clk_src == clk_src_t::aclk		? 0x0040 : 0 )
			) {	}
	};


	bool put(char, blocking_t = blocking_t::blocking) const noexcept;
	bool get(char&, blocking_t = blocking_t::blocking) const  noexcept;
	inline constexpr uart(channel_t ch) noexcept : channel(ch) {}
	inline uart(channel_t ch, baudrate_t b) noexcept : channel(ch) { begin(b); }
	void begin(baudrate_t b, const params & p = params()) const noexcept;
	void end() const noexcept;
	void put(const char* msg) const noexcept {
	    while(*msg) put(*msg++);
	}
public:
	const channel_t channel;
private:
	inline void init(uart::baudrate_t,const uart::params&) const noexcept;
	uart();
};
}
