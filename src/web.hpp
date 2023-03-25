#ifndef _HEADER_WEB_
#define _HEADER_WEB_

#include <boost/asio.hpp>

std::string_view get_path(std::string_view message);
void css_route(std::ostringstream &response);
void js_route(std::ostringstream &response);
void favicon_route(std::ostringstream &response);
void stats_route(std::ostringstream &response);
void home_route(std::ostringstream &response);
void invalid_route(std::ostringstream &response);
void handle_request(boost::asio::ip::tcp::socket &socket);

#endif
