#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <micurest_arduino.h>
#include "micurpc.hpp"

using namespace micurest;
using namespace micurpc;

namespace name {
  /* defining names/identifiers used in this demo
   * string literals are placed in progmem with macro NAME or ALIAS     */
	NAME(rpc)
	NAME(demo)
	NAME(digitalWrite)
	NAME(digitalRead)
	NAME(analogRead)
	NAME(analogWrite)
	NAME(pinMode)
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

//HTTP with one page (ROM/RAM):  26874/1449
//RPC  with one proc (ROM/RAM):  35292/1815
//RPC  with two proc (ROM/RAM):  36458/1865
//RPC  with five proc (ROM/RAM): 40376/2071

/* HTML page facilitating a web console for JSON RPC   */
cstring demo_content() {
  static const char content[] __attribute__((progmem)) = 
	"<!DOCTYPE html><html><head>"
	"<meta charset='utf-8'><title>Arduino RPC Example</title>"
	"<link rel='stylesheet' href='//r.iot-ware.com/r/css/responsive.css'>"
	"<link rel='stylesheet' href='//r.iot-ware.com/r/css/rpc.css'>"
	"</head>"
	"<body>"
		  "<div id='left'>&nbsp;</div>"
		  "<div id='main'>"
		  "<input type='text' placeholder='Type a function call and hit enter'>"
		  "<div id='log'></div></div>"
		  "<div id='right'>&nbsp;</div>"
	"</body>"
	"<script type='text/javascript' src='//r.iot-ware.com/r/js/rpc.js'></script>"
	"</html>";
	return cstring(content);
}

cstring constants_json() {
	static const char content[] __attribute__((progmem)) =
		"{\"INPUT\":0,\"OUTPUT\":1,\"INPUT_PULLUP\":2}";
	return cstring(content);
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
		if( pin >= NUM_DIGITAL_PINS ) {
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

	//uint8_t pin; /* this does not work */

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
		switch(mode) {
		case INPUT:
		case OUTPUT:
		case INPUT_PULLUP:
			pinMode(pin,(uint8_t)mode);
			return;
		}
		invalidParameter(error);
	}
	static cstring method() noexcept { return name::pinMode(); }
private:
	uint8_t pin  = -1;
	uint8_t mode = -1; /* when fewer parameters come, initial value remains	*/
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


/* Defines REST map of URIs resources 										*/
const directory& resourceMap() noexcept {
	return Root<
		/* first entry returns an html page									*/
		resource::FileConstString<name::demo, demo_content, media::html>,
		/* this entry associates URI /rpc with the RPC api					*/
		E<name::rpc, Api::rpc>,
		/* this is a directory with mete resources							*/
		D<name::meta,
			E<name::rpc,N<Enums<Api::nameof>>>,	/* list of API procedures	*/
			F<name::_const, constants_json, media::json> /* static json text*/
		>
	>();
}

/* define an application instance associated with the map */
application rest(resourceMap());

/* instance of a tcp server, bbound to port 80            */
network_arduino::tcp::server server(rest);
static uint8_t mac[]={0xC2,0xB5,0x52,0x45,0x53,0x54};
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  // start serial at 115200
  Serial.begin(115200);    
  Serial.println("MicuRPC Demo is starting");
  // start Ethernet
  Ethernet.begin(mac);
  Serial.print("server IP is ");
  Serial.println(Ethernet.localIP());  
  // start listening
  server.listen(80);

}

void loop() {
	server.run();
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

