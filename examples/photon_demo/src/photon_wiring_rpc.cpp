#include "micurest/micurpc.hpp"
#include "utils.hpp"
#include <application.h>
#include <Arduino.h>

using namespace micurest;
using namespace micurpc;

namespace name {
  /* defining names/identifiers used in this demo
   * string literals are placed in progmem with macro NAME or ALIAS     */
	NAME(rpc)
	NAME(rpcc)
	NAME(digitalWrite)
	NAME(digitalRead)
	NAME(analogRead)
	NAME(analogWrite)
	NAME(pinMode)
	NAME(getPinMode)
	NAME(pin)
	NAME(val)
	NAME(mode)
	NAME(meta)
	NAME(tone)
	NAME(noTone)
	NAME(frequency)
	NAME(duration)
	ALIAS(_const,const)
}

/* HTML page facilitating a web console for JSON RPC   */
cstring rpcc_content() {
  return
	"<!DOCTYPE html><html><head>"
	"<meta charset='utf-8'><title>Photon RPC Example</title>"
	"<link rel='stylesheet' href='//r.iot-ware.com/p/css/responsive.css'>"
	"<link rel='stylesheet' href='//r.iot-ware.com/p/css/rpc.css'>"
	"</head>"
	"<body>"
		  "<div id='left'>&nbsp;</div>"
		  "<div id='main'>"
		  "<input type='text' placeholder='Type a function call and hit enter'>"
		  "<div id='log'></div></div>"
		  "<div id='right'>&nbsp;</div>"
	"</body>"
	"<script type='text/javascript' src='//r.iot-ware.com/p/js/rpc.js'></script>"
	"</html>";
}

cstring constants_json() {
	return
		"{\"INPUT\":0,\"OUTPUT\":1,\"INPUT_PULLUP\":2,\"INPUT_PULLDOWN\":3,"
		"\"AF_OUTPUT_PUSHPULL\":4,\"AF_OUTPUT_DRAIN\":5,"
		"\"AN_INPUT\":6,\"AN_OUTPUT\":7}";
}


namespace rpc {
using cojson::details::lexer;

namespace messages {
	ALIAS(no_such_digital_pin,Pin ID exceeds count of digital pins)
	ALIAS(no_such_analog_pin, Pin ID exceeds count of analog pins)
	ALIAS(invalid_parameter,  Invalid parameter)
}

/* a base class with common functions */
class PinFunction : public Procedure {
protected:
	static bool digitalPinIsOK(uint8_t pin, response::error_t& error) noexcept {
		if( pin >= NUM_DIGITAL_PINS ) {
			error.code     = error_code(1);
			error.message  = messages::no_such_digital_pin();
			return false;
		}
		return true;
	}
	static bool analogPinIsOK(uint8_t pin, response::error_t& error) noexcept {
		if( pin >= TOTAL_ANALOG_PINS ) {
			error.code     = error_code(2);
			error.message  = messages::no_such_analog_pin();
			return false;
		}
		return true;
	}
	static void invalidParameter(response::error_t& error) noexcept {
		error.code = error_code(3);
		error.message = messages::invalid_parameter();
	}
};

/* class, implementing RPC call digitalWrite								*/
class DigitalWrite : public PinFunction {
public:
	typedef DigitalWrite self;
	/* micurpc calls this method to read procedure parameters				*/
	bool read_params(lexer& in) noexcept {
		return
			Params<self, uint8_t, uint8_t>::	/* defines parameter types	*/
			Names<name::pin, name::val>::		/* defines parameter names	*/
			FieldPointers<&self::pin,&self::val>/* binds parameters to members*/
			::json().read(*this, in);			/* reads JSON				*/
	}
	/* micurpc calls this method to write result							*/
	bool write_result(ostream& out) const noexcept {
		/* result must be a JSON value (in its wide sense)					*/
		return write(val, out);
	}
	/* micurpc calls this method to run the procedure						*/
	void run(response::error_t& error) noexcept {
		if(! digitalPinIsOK(pin, error)) return;
		if( val != 0 && val != 1 ) {
			invalidParameter(error);
			return;
		}
		digitalWrite(pin, val);
		val = digitalRead(pin);
	}
	/* defines a literal name for this procedure							*/
	static cstring method() noexcept { return name::digitalWrite(); }
private:
	/* ! pin cannot be moved to parent because of member address type 		*/
	uint8_t pin	= -1; /* invalid initial values indicated missing parameter */
	uint8_t val = -1;
};

/* class, implementing RPC call digitalRead									*/
class DigitalRead : public PinFunction {
public:
	typedef DigitalRead self;

	bool read_params(lexer& in) noexcept {
		return
			Params<self, uint8_t>::
			Names<name::pin>::
			FieldPointers<&self::pin>::json().read(*this, in);
	}
	bool write_result(ostream& out) const noexcept {
		return write(val, out);
	}
	void run(response::error_t& error) noexcept {
		if(! digitalPinIsOK(pin, error)) return;
		val = digitalRead(pin);
	}
	static cstring method() noexcept { return name::digitalRead(); }
private:
	uint8_t pin = -1;
	uint8_t val;
};


class PinMode : public PinFunction {
public:
	typedef PinMode self;

	bool read_params(lexer& in) noexcept {
		return
			Params<self, uint8_t, uint8_t>::
			Names<name::pin, name::mode>::
			FieldPointers<&self::pin, &self::mode>::json().read(*this, in);
	}
	void run(response::error_t& error) noexcept {
		if(! digitalPinIsOK(pin, error)) return;
		if(mode >= INPUT && mode <= AN_OUTPUT) {
			pinMode(pin,(::PinMode)mode);
			return;
		}
		invalidParameter(error);
	}
	static cstring method() noexcept { return name::pinMode(); }
private:
	uint8_t pin  = -1;
	uint8_t mode = -1; /* when fewer parameters come, initial value remains	*/
};

class GetPinMode : public PinFunction {
public:
	typedef GetPinMode self;

	bool read_params(lexer& in) noexcept {
		return
			Params<self, uint8_t>::
			Names<name::pin>::
			FieldPointers<&self::pin>::json().read(*this, in);
	}
	void run(response::error_t& error) noexcept {
		if(! digitalPinIsOK(pin, error)) return;
		mode = getPinMode(pin);
	}
	bool write_result(ostream& out) const noexcept {
		return write(mode, out);
	}
	static cstring method() noexcept { return name::getPinMode(); }
private:
	uint8_t pin  = -1;
	uint8_t mode;
};


class AnalogRead : public PinFunction {
public:
	typedef AnalogRead self;

	bool read_params(lexer& in) noexcept {
		return
			Params<self, uint8_t>::
			Names<name::pin>::
			FieldPointers<&self::pin>::json().read(*this, in);
	}
	bool write_result(ostream& out) const noexcept {
		return write(val, out);
	}
	void run(response::error_t& error) noexcept {
		if(! analogPinIsOK(pin, error) ) return;
		val = analogRead(pin);
	}
	static cstring method() noexcept { return name::analogRead(); }
private:
	uint8_t pin = -1;
	uint16_t val;
};

class AnalogWrite : public PinFunction {
public:
	typedef AnalogWrite self;

	bool read_params(lexer& in) noexcept {
		return
			Params<self, uint8_t, uint8_t>::
			Names<name::pin, name::val>::
			FieldPointers<&self::pin,&self::val>::json().read(*this, in);
	}
	void run(response::error_t& error) noexcept {
		if(! digitalPinIsOK(pin, error) ) return;
		analogWrite(pin, val);
	}
	static cstring method() noexcept { return name::analogWrite(); }
private:
	uint8_t pin;
	uint8_t val;
};

class Tone : public PinFunction {
public:
	typedef Tone self;

	bool read_params(lexer& in) noexcept {
		return
			Params<self, uint8_t, uint16_t, uint32_t>::
			Names<name::pin, name::frequency, name::duration>::
			FieldPointers<&self::pin,&self::freq, &self::duration>::json().read(*this, in);
	}
	void run(response::error_t& error) noexcept {
		if(! digitalPinIsOK(pin, error) ) return;
		if( freq < 31 ) {
			invalidParameter(error);
			return;
		}
		tone(pin, freq, duration);
	}
	static cstring method() noexcept { return name::tone(); }
private:
	uint8_t  pin = -1;
	uint16_t freq;
	uint32_t duration;
};

class NoTone : public PinFunction {
public:
	typedef NoTone self;

	bool read_params(lexer& in) noexcept {
		return
			Params<self, uint8_t>::
			Names<name::pin>::
			FieldPointers<&self::pin>::json().read(*this, in);
	}
	void run(response::error_t& error) noexcept {
		if(! digitalPinIsOK(pin, error) ) return;
		noTone(pin);
	}
	static cstring method() noexcept { return name::noTone(); }
private:
	uint8_t  pin = -1;
};
}

/****************************************************************************/
/* API is defined as an RPC service with a list of procedures				*/
typedef micurpc::Service<
	rpc::DigitalWrite,
	rpc::DigitalRead,
	rpc::PinMode,
	rpc::GetPinMode,
	rpc::AnalogWrite,
	rpc::AnalogRead,
	rpc::Tone,
	rpc::NoTone
> Api;

/***************************************************************************/


/* This is used to get a list of procedure names from API					*/
template<cojson::details::cstring (*get)(unsigned)>
const cojson::details::value& Enums() noexcept {
	static const cojson::details::cstrings<get> l;
	return l;
}

using micurest::details::entry;

/* Defines RPC-related resources 											*/

const entry& rpcc_entry() noexcept {
	return resource::FileConstString<name::rpcc, rpcc_content, media::html>();
}

const entry& rpc_entry() noexcept {
	return E<name::rpc, Api::rpc>();
}

const entry& meta_entry() noexcept {
	return D<name::meta,
		E<name::rpc,N<Enums<Api::nameof>>>,	/* list of API procedures	*/
		F<name::_const, constants_json, media::json> /* static json text*/
	>();
}



