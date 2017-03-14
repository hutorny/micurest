//
// connection.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <utility>
#include <vector>
#	include <iostream>
#include "connection.hpp"
#include "connection_manager.hpp"
#include "request_handler.hpp"

namespace http {
namespace server {

connection::connection(boost::asio::ip::tcp::socket socket,
    connection_manager& manager, request_handler& handler)
  : socket_(std::move(socket)),
    connection_manager_(manager),
    request_handler_(handler)
{
}

void connection::start()
{
  do_read();
}

void connection::stop()
{
  socket_.close();
}

void connection::do_read()
{
	auto self(shared_from_this());
	socket_.non_blocking(true);
	socket_.async_read_some(boost::asio::null_buffers(),
		[this, self](boost::system::error_code ec, std::size_t bytes_transferred)
		{
			if (!ec)
				{
					boost::asio::ip::tcp::iostream io;
					io.rdbuf()->assign( boost::asio::ip::tcp::v4(), socket_.native() );
					if( ! request_handler_.handle_request(io) ) {
//						socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_receive);
					}
					connection_manager_.stop(shared_from_this());
				}
			else if (ec != boost::asio::error::operation_aborted)
			{
			  connection_manager_.stop(shared_from_this());
			}
		});
}

} // namespace server
} // namespace http
