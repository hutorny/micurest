/*
 * Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * network_lwip.hpp - session layer definitions for lwIP transport layer
 *
 * This file is part of µcuREST Library. http://hutorny.in.ua/projects/micurest
 *
 * modify it under the terms of the GNU General Public License v2
 * as published by the Free Software Foundation;
 *
 * The µcuREST Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License v2
 * along with the COJSON Library; if not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>.
 */
#pragma once
#include <configuration.h>
#include "network_lwip.ccs"
#include "micurest.hpp"
struct tcp_pcb;

namespace micurest {
namespace network_lwip {
using micurest::application;

/*****************************************************************************/
namespace tcp {
using micurest::port_t;

/**
 * This class uses event-driven lwIP API
 */
class server {
public:
	typedef int8_t err_t;
	typedef uint16_t size_t;
	static constexpr uint16_t timeout = 60;
	static constexpr size_t mtu_size = 1460;
	inline server(const application& a) noexcept : app(a), pcb(nullptr) {}
	bool listen(port_t p) noexcept;
	inline const application& getapp() const noexcept { return app; }
private:
	const application& app;
	tcp_pcb* pcb;
};

}}}
