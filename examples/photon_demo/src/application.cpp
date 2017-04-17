// To verify product creator macros work correctly.
// These values get sent to the cloud on connection and help dashboard.particle.io do the right thing
//
#include "application.h"
#include "Serial2/Serial2.h"
#include "system_network.h"
#include "wlan_hal.h"
#include "micurest/network_spark_socket.hpp"
#include "micurest/access_log.hpp"

using namespace micurest;
using namespace micurest::network_spark_socket;

extern void server_run() noexcept;
extern void io_setup() noexcept;

void setup() {
	Serial1.begin(115200);
	io_setup();		/*	setup pin modes for this example	*/
	Serial1.println("Photon demo starting");
	/* wifi is not ready here, so there is no reason to try listen */
}
bool done = false;

void loop() {
	server_run();
}
