//
// request_handler.hpp
// ~~~~~~~~~~~~~~~~~~~
// boost example adjusted for the needs of micurest demo
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_REQUEST_HANDLER_HPP
#define HTTP_REQUEST_HANDLER_HPP

#include <string>

namespace http {
namespace server {

struct reply;
struct request;

/// The common handler for all incoming requests.
class request_handler
{
public:
  request_handler(const request_handler&) = delete;
  request_handler& operator=(const request_handler&) = delete;

  /// Construct with a directory containing files to be served.
  explicit request_handler() {}

  /// Handle a request and produce a reply,
  /// returns true if connection should be kept
  virtual bool handle_request(std::iostream& io) = 0;
};

} // namespace server
} // namespace http

#endif // HTTP_REQUEST_HANDLER_HPP
