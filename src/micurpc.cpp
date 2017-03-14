/*
 * Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * micurpc.cpp - JSON RPC implementation
 *
 * This file is part of µcuREST Library. http://hutorny.in.ua/projects/micurest
 *
 * The µcuREST Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License v2
 * as published by the Free Software Foundation;
 *
 * The µcuREST Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the µcuREST Library; if not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>.
 */
#include <string.h>
#include "micurpc.hpp"
namespace micurpc {
namespace details {

constexpr typename property<api::request>::node
	api::request::json::props[api::request::json::count];
constexpr typename property<api::response>::node
	api::response::json::success[api::response::json::count];
constexpr typename property<api::response>::node
	api::response::json::fail[api::response::json::count];
}
}
